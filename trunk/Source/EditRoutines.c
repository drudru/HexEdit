/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 * 
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 * 
 * The Original Code is Copyright 1993 Jim Bumgardner.
 * 
 * The Initial Developer of the Original Code is Jim Bumgardner
 * Portions created by Lane Roathe are
 * Copyright (C) Copyright © 1996-2001.
 * All Rights Reserved.
 * 
 * Contributor(s):
 *		Nick Shanks
 *		Greg Branche
 */

// 05/10/01 - GAB: MPW environment support
#ifdef __MPW__
#include "MPWIncludes.h"
#endif

#include <ctype.h>

#include "EditScrollbar.h"
#include "Menus.h"
#include "Utility.h"

#include "EditRoutines.h"

// Global Vars

UndoRecord gUndo, gRedo;

EditChunk	**gScrapChunk;


/*** LOAD FILE ***/
void LoadFile( EditWindowPtr dWin )
{
// Assumes theWin has just been opened, file is open, fileSize field is correct
	EditChunk	**nc;
	long		count, chunkSize, pos;

	count = dWin->fileSize;
	pos = 0L;

	// LR: - if empty fork, just create a chunk so we can insert data!

	if( !count )
		count = 1;

	while( count )
	{
		if( count <= ( kMaxFileRAM - kSlushRAM ) )
			chunkSize = count;
		else
			chunkSize = ( kMaxFileRAM - kSlushRAM );
		count -= chunkSize;
		nc = NewChunk( chunkSize, pos, pos, CT_Original );
		dWin->firstChunk = AppendChunk( dWin->firstChunk, nc );
		pos += chunkSize;
	}
	dWin->curChunk = dWin->firstChunk;
}

/*** UNLOAD FILE ***/
void UnloadFile( EditWindowPtr dWin )
{
	EditChunk	**cc, **bc;
	cc = dWin->firstChunk;
	while( cc )
	{
		bc = ( *cc )->next;
		DisposeChunk( dWin, cc );
		cc = bc;
	}
	dWin->firstChunk = dWin->curChunk = NULL;
}

/*** NEW CHUNK ***/
EditChunk** NewChunk( long size, long addr, long filePos, short type )
{
	EditChunk **nc;
	nc = ( EditChunk ** ) NewHandleClear( sizeof( EditChunk ) );
	if( !nc )
	{
		ErrorAlert( ES_Caution, errMemory );
		return NULL;
	}
	( *nc )->type = type;
	( *nc )->size = size;
	( *nc )->addr = addr;
	( *nc )->filePos = filePos;
	( *nc )->lastCtr = -1;
	if( type == CT_Unwritten )
	{
		( *nc )->loaded = true;
		( *nc )->allocSize = size;
		( *nc )->data = NewHandleClear( ( *nc )->allocSize );
		if( !( *nc )->data )
		{
			ErrorAlert( ES_Caution, errMemory );
			DisposeHandle( (Handle) nc );
			return NULL;
		}
	}
	else
	{
		( *nc )->loaded = false;
		( *nc )->data = NULL;
		( *nc )->allocSize = 0L;
	}
	return nc;
}

/*** DISPOSE CHUNK ***/
void DisposeChunk( EditWindowPtr dWin, EditChunk **cc )
{
	if( ( *cc )->loaded )
	{
		if( ( *cc )->loaded )
			UnloadChunk( dWin, cc, false );
	}
	DisposeHandle( (Handle) cc );
}

/*** APPEND CHUNK ***/
EditChunk** AppendChunk( EditChunk **list, EditChunk **chunk )
{
	if( list )
	{
		register EditChunk	**curChunk;
		curChunk = list;
		while( ( *curChunk )->next )
			curChunk		= ( *curChunk )->next;
		( *curChunk )->next	= chunk;
		( *chunk )->prev	= curChunk;
		( *chunk )->next	= NULL;
	}
	else
	{
		list = chunk;
		( *chunk )->next	= ( *chunk )->prev = NULL;
	}
	return list;
}

/*** SET CURRENT CHUNK ***/
void SetCurrentChunk( EditWindowPtr dWin, long addr )
{
	register EditChunk **cc;
	cc = GetChunkByAddr( dWin, addr );
	dWin->curChunk = cc;
}

/*** GET CHUNK BY ADDRESS ***/
EditChunk** GetChunkByAddr( EditWindowPtr dWin, long addr )
{
	register EditChunk **cc;
	if( dWin->curChunk && addr >= ( *dWin->curChunk )->addr )
		cc = dWin->curChunk;
	else	// Otherwise, start from beginning of chain
		cc = dWin->firstChunk;
	while( cc )
	{
		if( addr < ( *cc )->addr+( *cc )->size )	break;
		else
		{
			if( ( *cc )->next )	cc = ( *cc )->next;
			else return cc;
		}
	}
	return cc;
}

/*** GET BYTE ***/
Byte GetByte( EditWindowPtr dWin, long addr )
{
	register EditChunk **cc;
	if( ( cc = GetChunkByAddr( dWin, addr ) ) != NULL )
	{
		// Correct Chunk
		if( !( *cc )->loaded ) LoadChunk( dWin, cc );
		if( ( *cc )->lastCtr != dWin->useCtr )
		{
			// Update the Counter
			++dWin->useCtr;
			( *cc )->lastCtr = dWin->useCtr;
		}
		return (Byte) ( *( *cc )->data )[addr - ( *cc )->addr];
	}
	return (Byte)-1;
}

/*** LOAD CHUNK ***/
void LoadChunk( EditWindowPtr dWin, EditChunk **cc )
{
	long	count;
	OSErr	error;
	short	refNum;
	if( ( *cc )->loaded )
		return;
	// Check if we can fit within MaxFileRam, if not, deallocate old chunks
	// until we're ok
	while( dWin->totLoaded+( *cc )->size > kMaxFileRAM )
		UnloadLeastUsedChunk( dWin );
	( *cc )->data = NewHandleClear( ( *cc )->size );
	if( !( *cc )->data )
	{
		ErrorAlert( ES_Caution, errMemory );
		( *cc )->allocSize = 0L;
		( *cc )->loaded = false;
	}
	else
	{
		if( ( *cc )->type == CT_Work ) refNum = dWin->workRefNum;
		else refNum = dWin->refNum;
		( *cc )->allocSize = ( *cc )->size;
		( *cc )->loaded = true;
		if( ( error = SetFPos( refNum, fsFromStart, ( *cc )->filePos ) ) != noErr )
			ErrorAlert( ES_Caution, errSeek, error );
		count = ( *cc )->size;
		dWin->totLoaded += ( *cc )->size;
		if( ( error = FSRead( refNum, &count, *( *cc )->data ) ) != noErr )
			ErrorAlert( ES_Caution, errRead, error );
	}
}

/*** UNLOAD LEAST USED CHUNK ***/
void UnloadLeastUsedChunk( EditWindowPtr dWin )
{
	EditChunk	**cc, **oc=NULL;
	oc = cc = dWin->firstChunk;
	while( cc )
	{
		if( ( *cc )->loaded && ( !( *oc )->loaded || ( *cc )->lastCtr < ( *oc )->lastCtr ) )
			oc = cc;
		cc = ( *cc )->next;
	}
	if( oc ) UnloadChunk( dWin, oc, true );
}

/*** UNLOAD CHUNK ***/
void UnloadChunk( EditWindowPtr dWin, EditChunk	**cc, Boolean writeFlag )
{
	long	count;
	OSErr	error;

	if( cc && ( *cc )->loaded && ( *cc )->data )
	{
		if( writeFlag && ( *cc )->type == CT_Unwritten )
		{
			// Record New Chunks in Work File
			error = SetFPos( dWin->workRefNum, fsFromStart, dWin->workBytesWritten );
			if( error )
				ErrorAlert( ES_Caution, errSetFPos, error );
			count = ( *cc )->size;
			error = FSWrite( dWin->workRefNum, &count, *( *cc )->data );
			if( error )
				ErrorAlert( ES_Caution, errWrite, error );
			( *cc )->type = CT_Work;
			( *cc )->filePos = dWin->workBytesWritten;
			dWin->workBytesWritten += count;
		}

		dWin->totLoaded -= ( *cc )->size;
		( *cc )->loaded = false;
		DisposeHandle( ( *cc )->data );
		( *cc )->data = NULL;
		( *cc )->allocSize = 0L;
	}
}

/*** REWRITE ADDRESS CHAIN ***/
void RewriteAddressChain( EditChunk **fc )
{
	EditChunk	**nc;
	// Rewrite Addresses of chunks starting from fc
	nc = ( *fc )->next;
	while( nc )
	{
		( *nc )->addr = ( *( *nc )->prev )->addr + ( *( *nc )->prev )->size;
		nc = ( *nc )->next;
	}
}

/*** DELETE SELECTION ***/
void DeleteSelection( EditWindowPtr dWin )
{
	EditChunk **fc, **ec, **nc, **tc;

	if( dWin->endSel == dWin->startSel ) return;

	// Identify Starting Chunk
	fc = GetChunkByAddr( dWin, dWin->startSel );
	dWin->curChunk = fc;		// Optimize chunk searches

	// Identify Ending Chunk
	ec = GetChunkByAddr( dWin, dWin->endSel );

	// If Chunks are the same
	if( fc == ec )
	{
		// If chunk is unwritten
		if( ( *fc )->type == CT_Unwritten )
		{
			BlockMove( *( *fc )->data + ( dWin->endSel - ( *fc )->addr ), 
						*( *fc )->data + ( dWin->startSel - ( *fc )->addr ), 
						( *fc )->size - ( dWin->endSel - ( *fc )->addr ) );
			( *fc )->size -= dWin->endSel - dWin->startSel;
		}
		else
		{
			UnloadChunk( dWin, fc, true );
			// Split shorto two chunks
			nc = NewChunk( ( *fc )->size - ( dWin->endSel - ( *fc )->addr ), 
							0, 
							( *fc )->filePos + ( dWin->endSel - ( *fc )->addr ), 
							( *fc )->type );
			( *nc )->prev = fc;
			( *nc )->next = ( *fc )->next;
			if( ( *nc )->next )
				( *( *nc )->next )->prev = nc;
			( *fc )->next = nc;
			( *fc )->size = dWin->startSel - ( *fc )->addr;
		}
	}
	else
	{
		// Truncate end of first Chunk
		( *fc )->size = dWin->startSel - ( *fc )->addr;
		// Unlink & Dispose Middle Chunks, If Any
		nc = ( *fc )->next;
		while( nc != ec )
		{
			tc = ( *nc )->next;
			DisposeChunk( dWin, nc );
			nc = tc;
		}
		( *ec )->prev = fc;
		( *fc )->next = ec;
		// Truncate beg of end chunk
		if( ( *ec )->type == CT_Unwritten )
		{
			long	offset;
			offset = dWin->endSel - ( *ec )->addr;
// LR: -- fix according to feedback from Jonathan Wright
// 			BlockMove( *( *ec )->data, *( *ec )->data+offset, ( *ec )->size - offset );
			BlockMove( *( *ec )->data+offset, *( *ec )->data, ( *ec )->size - offset );
			( *ec )->size -= offset;
		}
		else
		{
			long	offset;
			offset = dWin->endSel - ( *ec )->addr;
			UnloadChunk( dWin, ec, true );
			( *ec )->filePos += offset;
			( *ec )->size -= offset;
		}
	}

	dWin->fileSize -= ( dWin->endSel - dWin->startSel );

	RewriteAddressChain( fc );

	// Modify Current Selection such that	endSel = firstSel
	dWin->endSel = dWin->startSel;
	dWin->dirtyFlag = true;
}

// Assumes selection Point is already 0 chars wide...

/*** INSERT CHARACTER ***/
void InsertCharacter( EditWindowPtr dWin, short charCode )
{
	EditChunk **fc, **ec, **nc;	// LR: remove tc to fix warnings

	// !! Remember Current State for Undo


	// Insert Character shorto List
	// 	Identify current chunk - optimize so that if char is between
	// 		chunks, pick the unwritten one of the two...

	// Identify Starting Chunk
	fc = GetChunkByAddr( dWin, dWin->startSel );

	// 	Identify current chunk - optimize so that if char is between
	// 	chunks, pick the unwritten one of the two... - this way, if I keep typing
	// 	characters, I won't generate a bunch of 1 byte chunks.
	if( dWin->startSel - ( *fc )->addr == 0 &&
		( *fc )->prev && ( *fc )->type != CT_Unwritten &&
	( *( *fc )->prev )->type == CT_Unwritten )
		fc = ( *fc )->prev;
	dWin->curChunk = fc;		// Optimize chunk searches

	// 	If current chunk is not unwritten
	if( ( *fc )->type != CT_Unwritten )
	{
		// Unload it
		UnloadChunk( dWin, fc, true );

		if( dWin->startSel > ( *fc )->addr )
		{

			// Split shorto two chunks
			if( dWin->startSel < ( *fc )->addr + ( *fc )->size )
			{
				ec = NewChunk( ( *fc )->size - ( dWin->startSel - ( *fc )->addr ), 0, 
								( *fc )->filePos + ( dWin->startSel - ( *fc )->addr ), 
								( *fc )->type );
				( *ec )->prev = fc;
				( *ec )->next = ( *fc )->next;
				if( ( *ec )->next )
					( *( *ec )->next )->prev = ec;
				( *fc )->next = ec;
			}
			else ec = ( *fc )->next;

			( *fc )->size = dWin->startSel - ( *fc )->addr;
		}
		else
		{
			ec = fc;
			fc = ( *fc )->prev;
		}

		// Add New unwritten chunk in middle with 0 size
		nc = NewChunk( 0, 0, 0, CT_Unwritten );
		if( fc )
		{
			( *fc )->next = nc;
			( *nc )->addr = ( *fc )->addr + ( *fc )->size;
		}
		else
			dWin->firstChunk = nc;
		if( ec )
			( *ec )->prev = nc;
		( *nc )->prev = fc;
		( *nc )->next = ec;
		// current chunk = new chunk
		dWin->curChunk = nc;
		fc = nc;
	}

	// 	Expand Ptr if Necessary
	if( ( *fc )->allocSize <= ( *fc )->size )
	{
		( *fc )->allocSize += kAllocIncrement;		// !! consider expanding as size goes up
		SetHandleSize( ( *fc )->data, ( *fc )->allocSize );
	}

	// Make Room for Character if necessary
	if( dWin->startSel < ( *fc )->addr + ( *fc )->size )
		BlockMove( *( *fc )->data + ( dWin->startSel - ( *fc )->addr ), 
					*( *fc )->data + ( 1+( dWin->startSel - ( *fc )->addr ) ), 
					( *fc )->addr + ( *fc )->size - dWin->startSel );

	// 	Insert Char shorto buffer
	( *( *fc )->data )[dWin->startSel - ( *fc )->addr] = charCode;

	// 	Update Fields in this chunk
	( *fc )->size++;
	dWin->fileSize++;

	// Set Dirty Flag
	dWin->dirtyFlag = true;

	// 	Update addr fields of following chunks
	RewriteAddressChain( fc );

	// Increment current Selection
	dWin->startSel++;
	dWin->endSel++;


	// Update Display
	ScrollToSelection( dWin, dWin->startSel, true, false );
}

/*** RELEASE EDIT SCRAP ***/
void ReleaseEditScrap( EditWindowPtr dWin, EditChunk ***scrap )
{
	EditChunk	**cc, **bc;
	cc = *scrap;
	while( cc )
	{
		bc = ( *cc )->next;
		DisposeChunk( dWin, cc );
		cc = bc;
	}
	*scrap = NULL;
}

// High Level Copy

/*** COPY SELECTION ***/
void CopySelection( EditWindowPtr dWin )
{
#if !TARGET_API_MAC_CARBON
// LR: v1.6.5	PScrapStuff ScrapInfo;	// LR: see below
#endif

	CopyOperation( dWin, &gScrapChunk );
	if( gScrapChunk )
	{
		KeyMap keys;	//LR 1.72 - shift to copy hex w/o formatting.
		Boolean shift;
		OSErr anErr;

#if TARGET_API_MAC_CARBON
		ScrapRef scrapRef;

		anErr = ClearCurrentScrap();
		if( !anErr )
			anErr = GetCurrentScrap( &scrapRef );
#else
		ZeroScrap();
#endif

		// LR: -- sure wish I remembered who sent me this code!!!
		// LR: v1.6.5 Now a menu selection, with ASCII the default :)
		GetKeys( keys );
		shift =  keys[1] & (1<<0);

		if( EM_Hex == dWin->editMode )
		{
			Handle tmp;
			const char *src;
			char *dest, bit;
			long i, len = ( *gScrapChunk )->size * (shift ? 2 : 3);	//LR 1.72 -- size depends on how copied

			tmp = NewHandle( len );
			if( tmp )
			{
				HLock( ( *gScrapChunk )->data );
				HLock( tmp );

				src = ( const char * ) *( *gScrapChunk )->data;
				dest = *tmp;

				len = ( *gScrapChunk )->size;
				for( i=0; i<len; ++i, ++src )
				{
					 bit = ( src[0] & 0xF0 ) >> 4;
					 *dest++ = bit > 9 ? ( bit-10+'A' ) : ( bit+'0' );
					 bit = ( src[0] & 0x0F );
					 *dest++ = bit > 9 ? ( bit-10+'A' ) : ( bit+'0' );
					 if( !shift )
						 *dest++ = (i + 1) % kBytesPerLine == 0 ? '\r' : ' ';
				}

				HUnlock( ( *gScrapChunk )->data );

#if TARGET_API_MAC_CARBON
				anErr = PutScrapFlavor( scrapRef, kScrapFlavorTypeText, kScrapFlavorMaskNone, GetHandleSize( tmp ), *tmp );
#else
				anErr = PutScrap( GetHandleSize( tmp ), kScrapFlavorTypeText, *tmp );
#endif
				HUnlock( tmp );
				DisposeHandle( tmp );
			}
		}
		else
		{
			 HLock( ( *gScrapChunk )->data );
#if TARGET_API_MAC_CARBON
			anErr = PutScrapFlavor( scrapRef, kScrapFlavorTypeText, kScrapFlavorMaskNone, (*gScrapChunk)->size, *(*gScrapChunk)->data );
#else
			anErr = PutScrap( (*gScrapChunk)->size, kScrapFlavorTypeText, *(*gScrapChunk)->data );
#endif
			 HUnlock( ( *gScrapChunk )->data );
		}

		 // LR: the "correct" way to do things ( UniversalHeaders )
/* 1.65 LR -- this is not needed for anything
#if TARGET_API_MAC_CARBON
		{
			anErr = GetScrapFlavorSize( gScrapRef, kScrapFlavorTypeText, &gScrapCount );
		}
#else
		 ScrapInfo = InfoScrap();
		 gScrapCount = ScrapInfo->scrapCount;
// 		gScrapCount = ScrapInfo.scrapCount;
#endif
*/
		 ( *gScrapChunk )->lastCtr = 0;	// Flag as shorternal

	}
}


/*** COPY OPERATION ***/
void CopyOperation( EditWindowPtr dWin, EditChunk ***scrapChunk )
{
	EditChunk	**fc, **ec, **nc, **tc;
	// Unload current scrap
	ReleaseEditScrap( dWin, scrapChunk );

	// Copy current selection shorto scrapChunk
	// Identify Starting Chunk
	fc = GetChunkByAddr( dWin, dWin->startSel );
	dWin->curChunk = fc;		// Optimize chunk searches

	// Identify Ending Chunk
	ec = GetChunkByAddr( dWin, dWin->endSel );

	// If Chunks are the same
	nc = NewChunk( dWin->endSel - dWin->startSel, 0, 0, CT_Unwritten );
	if( !nc ) return;

	*scrapChunk = nc;

	if( fc == ec )
	{
		LoadChunk( dWin, fc );
		BlockMove( *( *fc )->data + ( dWin->startSel - ( *fc )->addr ), 
					*( *nc )->data, ( *nc )->size );
	}
	else
	{
		// First Chunk to End
		tc = fc;
		LoadChunk( dWin, tc );
		BlockMove( *( *tc )->data + ( dWin->startSel - ( *tc )->addr ), *( *nc )->data, 
					( *tc )->size - ( dWin->startSel - ( *tc )->addr ) );
		tc = ( *tc )->next;

		// Middle Chunks, If Any
		while( tc != ec )
		{
			LoadChunk( dWin, tc );
			BlockMove( *( *tc )->data, *( *nc )->data + ( ( *tc )->addr - dWin->startSel ), 
						( *tc )->size );
			tc = ( *tc )->next;
		}

		// Last Chunk
		LoadChunk( dWin, tc );
		BlockMove( *( *tc )->data, *( *nc )->data + ( ( *tc )->addr - dWin->startSel ), 
					dWin->endSel - ( *tc )->addr );
	}
}

/*** CUT SELECTION ***/
void CutSelection( EditWindowPtr dWin )
{
	RememberOperation( dWin, EO_Cut, &gUndo );
	CopyOperation( dWin, &gScrapChunk );		// Copy shorto paste buffer
	DeleteSelection( dWin );
	dWin->dirtyFlag = true;
	ScrollToSelection( dWin, dWin->startSel, true, false );
}

// LR: v1.6.5 -- New code for getting scrap in Carbon & Classic styles. Here
// because it didn't make sense to grab it all the time in the idle routine

/*** MY GET SCRAP ***/
void MyGetScrap( EditWindowPtr dWin )
{
	long scrapSize = 0;
	OSErr anErr;

#if TARGET_API_MAC_CARBON
	ScrapRef scrapRef;
	ScrapFlavorFlags flavorFlags;

	anErr = GetCurrentScrap( &scrapRef );
	if( !anErr )
		anErr = GetScrapFlavorFlags( scrapRef, kScrapFlavorTypeText, &flavorFlags );		// non-blocking check for scrap data
	if( !anErr )
		anErr = GetScrapFlavorSize( scrapRef, kScrapFlavorTypeText, &scrapSize );	// blocking call to get size
#else
	long		offset;

	scrapSize = GetScrap( NULL, kScrapFlavorTypeText, &offset );
#endif

	if( scrapSize > 0 )
	{
		EditChunk	**nc;

		nc = NewChunk( scrapSize, 0, 0, CT_Unwritten );
		if( !nc ) ErrorAlert( ES_Caution, errMemory );
		else
		{
			ReleaseEditScrap( dWin, &gScrapChunk );
			gScrapChunk = nc;
#if TARGET_API_MAC_CARBON
			SetHandleSize( (*gScrapChunk)->data, scrapSize );
			anErr = MemError();
			if( !anErr )
			{
				HLock( (*gScrapChunk)->data );
				anErr = GetScrapFlavorData( scrapRef, kScrapFlavorTypeText, &scrapSize, *(*gScrapChunk)->data );
				HUnlock( (*gScrapChunk)->data );
			}
#else
			anErr = GetScrap( (*gScrapChunk)->data, kScrapFlavorTypeText, &offset );
#endif
			if( anErr >= 0 )
				( *gScrapChunk )->lastCtr = 1;	// Flag as external
			else
			{
				ReleaseEditScrap( dWin, &gScrapChunk );	// error!
				ErrorAlert( ES_Caution, errPaste, (int)anErr );
			}
		}
	}
}

// High Level Paste

/*** PASTE SELECTION ***/
void PasteSelection( EditWindowPtr dWin )
{
	MyGetScrap( dWin );	// LR: v1.6.5 get scrap only as needed

	if( gScrapChunk )	// LR: 1.7 due to bug (?) in Carbon, scrap may not be available!
	{
		// LR: v1.6.5 moved from PasteOperation to avoid bad Undo
		// Hex Pasting Mode for Outside Pastes
		if( EM_Hex == dWin->editMode && ( *gScrapChunk )->lastCtr == 1 )
		{
// LR: v1.6.5 failure not a problem!		if( !HexConvertScrap( dWin, gScrapChunk ) ) return;
			HexConvertScrap( dWin, gScrapChunk );
		}

		// Do actual paste
		RememberOperation( dWin, EO_Paste, &gUndo );

		PasteOperation( dWin, gScrapChunk );
		dWin->dirtyFlag = true;
		ScrollToSelection( dWin, dWin->startSel, true, false );
	}
}

/*** HEX CONVERT SCRAP ***/
Boolean HexConvertScrap( EditWindowPtr dWin, EditChunk **scrapChunk )
{
	#pragma unused( dWin )	// LR: fix warnings

	Handle	rh = NULL;
	Ptr		sp, dp, esp;
	short	val;
	Boolean	loFlag;

	rh = NewHandle( ( *scrapChunk )->size );
	if( !rh )
	{
		ErrorAlert( ES_Caution, errMemory );
		return false;
	}
	HLock( rh );
	HLock( ( *scrapChunk )->data );
	sp = *( *scrapChunk )->data;
	esp = sp + ( *scrapChunk )->size;
	dp = *rh;
	loFlag = false;
	for( ; sp < esp; ++sp )
	{
		if( *sp == '0' && *( sp +1 ) == 'x' )
		{
			loFlag = 0;
			++sp;
			continue;
		}
		if( isspace( *sp ) || ispunct( *sp ) )
		{
			loFlag = 0;
			continue;
		}
		if( *sp >= '0' && *sp <= '9' )		val = *sp - '0';
		else if( *sp >= 'A' && *sp <= 'F' )	val = 0x0A + ( *sp - 'A' );
		else if( *sp >= 'a' && *sp <= 'f' )	val = 0x0A + ( *sp - 'a' );
		else goto HexError;
		if( loFlag )
		{
			*( dp-1 ) = ( *( dp-1 ) << 4 ) | val;
			loFlag = 0;
		}			
		else
		{
			*dp = val;
			++dp;
			loFlag = 1;
		}
	}
	if( dp - *rh == 0 ) goto HexError;

	( *scrapChunk )->size = dp - *rh;

// LR: v1.6.5	HUnlock( rh );
	BlockMove( *rh, *( *scrapChunk )->data, ( *scrapChunk )->size );
	HUnlock( ( *scrapChunk )->data );
	DisposeHandle( rh );

	( *scrapChunk )->lastCtr = 0;		// Mark as internal
	return true;
	
HexError:
// LR: v1.6.5	HUnlock( rh );
	DisposeHandle( rh );
	HUnlock( ( *scrapChunk )->data );
// LR: v1.6.5 no need to alert user, we paste anyway!	ErrorAlert( ES_Caution, "Only valid Hex values may be pasted here" );
	return false;
}

void PasteOperation( EditWindowPtr dWin, EditChunk **scrapChunk )
{
	EditChunk **fc, **ec, **nc;

	// Create duplicate scrap attached to nc->nec
	nc = NewChunk( ( *scrapChunk )->size, 0, 0, CT_Unwritten );
	if( !nc ) return;

	BlockMove( *( *scrapChunk )->data, *( *nc )->data, ( *nc )->size );

	DeleteSelection( dWin );

	// Insert paste buffer shorto selStart

	fc = GetChunkByAddr( dWin, dWin->startSel );
	if( ( *fc )->addr < dWin->startSel )
	{
		// Split 'em up
		// Unload it
		UnloadChunk( dWin, fc, true );

		// Split shorto two chunks
		if( dWin->startSel < ( *fc )->addr + ( *fc )->size )
		{
			ec = NewChunk( ( *fc )->size - ( dWin->startSel - ( *fc )->addr ), 0, 
							( *fc )->filePos + ( dWin->startSel - ( *fc )->addr ), ( *fc )->type );
			( *ec )->prev = fc;
			( *ec )->next = ( *fc )->next;
			if( ( *ec )->next ) ( *( *ec )->next )->prev = ec;
		}
		else ec = ( *fc )->next;

		( *fc )->next = ec;
		( *fc )->size = dWin->startSel - ( *fc )->addr;
	}
	else
	{
		ec = fc;
		fc = ( *fc )->prev;
	}

	// Insert fc->nc->ec
	if( fc )
	{
		( *fc )->next = nc;
		( *nc )->prev = fc;
		( *nc )->addr = ( *fc )->addr + ( *fc )->size;
	}
	else
	{
		dWin->firstChunk = nc;
		( *nc )->addr = 0L;
	}

	if( ec )
	{
		( *nc )->next = ec;
		( *ec )->prev = nc;
	}

	// Correct addresses
	RewriteAddressChain( nc );

	// Reset Selection
	dWin->startSel = dWin->endSel = ( *nc )->addr + ( *nc )->size;

	// Update other stuff
	dWin->fileSize += ( *scrapChunk )->size;
	dWin->dirtyFlag = true;
}

/*** CLEAR SELECTION ***/
void ClearSelection( EditWindowPtr dWin )
{
	RememberOperation( dWin, EO_Clear, &gUndo );
	DeleteSelection( dWin );
	dWin->dirtyFlag = true;
	ScrollToSelection( dWin, dWin->startSel, true, false );
}

// Remember current state for Undo of following operation

/*** REMEMBER OPERATION ***/
void RememberOperation( EditWindowPtr dWin, short opType, UndoPtr ur )
{
	Str31	undoStr, menuStr;
	MenuRef editMenu;

	//LR: 1.66 - total re-write to be localizable!

	// Assume undo
	GetIndString( menuStr, strUndo, EO_Undo );

	// check for Redo (ie, if Undo change to Redo)
	editMenu = GetMenuRef( kEditMenu );
	if( ur == &gRedo )
	{
		Str31 tempStr;

		GetMenuItemText( editMenu, EM_Undo, tempStr );
		if( tempStr[1] == 'U' )
			GetIndString( menuStr, strUndo, EO_Redo );
	}

	// Now, get operation string and create menu string
	GetIndString( undoStr, strUndo, opType );
	BlockMove( &undoStr[1], &menuStr[menuStr[0] + 1], undoStr[0] );
	menuStr[0] += undoStr[0];
	SetMenuItemText( editMenu, EM_Undo, menuStr );

	ReleaseEditScrap( dWin, &ur->undoScrap );

	// Clear Undo Stuff
	ur->undoScrap = NULL;
	ur->type = opType;
	ur->startSel = dWin->startSel;
	ur->endSel = dWin->endSel;
	ur->fileSize = dWin->fileSize;
	ur->theWin = dWin;

	CopyOperation( dWin, &ur->undoScrap );

	( *ur->undoScrap )->lastCtr= 0;

	dWin->lastTypePos = -1;	// Clear Special Editing Modes
	dWin->loByteFlag = false;
}

/*** UNDO OPERATION ***/
void UndoOperation( void )
{
	WindowRef win;
	EditWindowPtr dWin;

	if( !gUndo.theWin )	//LR: 1.66 -- can be NULL!
		return;

	dWin = gUndo.theWin;
	if( gUndo.type == 0 ) return;

	//LR: 1.66 check for null front window!
	win = FrontNonFloatingWindow();
	if( !win || dWin != (EditWindowPtr)GetWRefCon( win ) )
		SelectWindow( dWin->oWin.theWin );
	
	switch( gUndo.type )
	{
		case EO_Typing:
		case EO_Paste:
		case EO_Insert:
			dWin->startSel = gUndo.startSel;
			dWin->endSel = dWin->fileSize - ( gUndo.fileSize - gUndo.endSel );
			RememberOperation( dWin, EO_Delete, &gRedo );
			DeleteSelection( dWin );
			PasteOperation( dWin, gUndo.undoScrap );
			break;
		case EO_Cut:
		case EO_Clear:
		case EO_Delete:
			dWin->startSel = dWin->endSel = gUndo.startSel;
			RememberOperation( dWin, EO_Insert, &gRedo );
			PasteOperation( dWin, gUndo.undoScrap );
			break;
	}

	ReleaseEditScrap( dWin, &gUndo.undoScrap );
	gUndo = gRedo;
	gRedo.undoScrap = NULL;

	ScrollToSelection( dWin, dWin->startSel, true, false );
}