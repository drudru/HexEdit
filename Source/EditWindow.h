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
 * Portions created by Lane Roathe (LR) are
 * Copyright (C) Copyright © 1996-2000.
 * All Rights Reserved.
 * 
 * Contributor(s):
 *		Nick Shanks (NS)
 *		Scott E. Lasley (SEL) 
 */

#include "ObjectWindow.h"

#ifndef _HexEdit_EditWindow_
#define _HexEdit_EditWindow_

/*** EDIT CHUNK ***/
typedef struct EditChunk
{
	struct EditChunk	**prev,**next;
	Boolean				loaded;			// Flag if chunk is currently loaded
	short				type;			// 0=Orig File, 1=Work File, 2=Unwritten
	Handle				data;			// Handle to Chunk Data
	long				size;			// Size of Chunk
	long				allocSize;		// Size of allocated Pointer
	long				addr;			// Start Addr in updated File
	long				filePos;		// Start Addr in actual File
	long				lastCtr;		// Use Counter
}	EditChunk;

/*** EDIT WINDOW ***/
typedef struct
{
	ObjectWindowRecord	oWin;			// Window Record
	ControlHandle		vScrollBar;		// Vertical Scroll Bar
	EditChunk			**firstChunk;	// File's First Chunk
	EditChunk			**curChunk;		// File's Current Chunk
	FSSpec				fsSpec,workSpec;// File Specs for Original, Work File
	FSSpec				destSpec;		// File Spec for Save, Save As
	long				fileSize;		// Total File Size
	long				fileType;		// File Type
	long				creator;		// File Creator
	unsigned long		creationDate;	// Creation Date
	long				useCtr;			// Chunk access Counter
										// Chunks are unloaded from memory based on usage
	long				totLoaded;		// Amount of bytes in Memory
	long				editOffset;		// Display Offset
	long				startSel;		// First Character of Selection
	long				endSel;			// First Character AFTER Selection
	long				lastTypePos;	// Last Typing Insertion Point
	short				refNum;			// File's Reference Number
	short				workRefNum;		// Work File's Reference Number
	short				workBytesWritten;	// Size of Work File
	short				linesPerPage;	// Lines that fit in the theWin
	short				editMode;		// 0=Hex, 1=Ascii
	short				fork;			// 0=data 1=resource
	short				lastNybble;		// Last Hex Edit Nibble
	Boolean				loByteFlag;		// Editing Low Byte for Hex Editor
	Boolean				dirtyFlag;		// File has been modified
	GWorldPtr			offscreen;
	short				csResID;		// LR: color table res ID for drawing theWin
}	EditWindowRecord, *EditWindowPtr;

void InitializeEditor( void );
void CleanupEditor( void );
GWorldPtr NewCOffScreen( short width, short height );
void NewEditWindow( void );
pascal short SourceDLOGHook( short item, DialogPtr theDialog );
pascal Boolean SourceDLOGFilter( DialogPtr dlg, EventRecord *event, short *item );
short AskEditWindow( void );
OSStatus OpenEditWindow( FSSpec *fsSpec, Boolean showerr );
void DisposeEditWindow( WindowRef theWin );
Boolean	CloseEditWindow( WindowRef theWin );
Boolean CloseAllEditWindows( void );
EditWindowPtr LocateEditWindow( FSSpec *fs, short fork );
EditWindowPtr FindFirstEditWindow( void );
OSStatus InitColorTable( HEColorTablePtr ct );
OSStatus GetColorInfo( EditWindowPtr dWin );
void DrawHeader( EditWindowPtr dWin, Rect *r );
void DrawFooter( EditWindowPtr dWin, Rect *r, short pageNbr, short nbrPages );
OSStatus DrawDump( EditWindowPtr dWin, Rect *r, long sAddr, long eAddr );
void DrawPage( EditWindowPtr dWin );
void MyDraw( WindowRef theWin );
void UpdateOnscreen( WindowRef theWin );
void MyIdle( WindowRef theWin, EventRecord *er );
void MyHandleClick( WindowRef theWin, Point where, EventRecord *er );
void InvertSelection( EditWindowPtr	dWin );
void PrintWindow( EditWindowPtr dWin );
void OffsetSelection( EditWindowPtr dWin, short offset, Boolean shiftFlag );
void MyProcessKey( WindowRef theWin, EventRecord *er );
void CursorOff( WindowRef theWin );
void CursorOn( WindowRef theWin );
OSStatus CopyFork( FSSpec *srcSpec, FSSpec *dstSpec, short forkType );
void SaveContents( WindowRef theWin );
void SaveAsContents( WindowRef theWin );
void RevertContents( WindowRef theWin );
void MyActivate( WindowRef theWin, Boolean active );
void UpdateEditWindows( void );

#endif