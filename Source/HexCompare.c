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
 * Copyright (C) Copyright © 1996-2002.
 * All Rights Reserved.
 *
 * Modified: $Date$
 * Revision: $Id$
 *
 * Contributor(s):
 *		Lane Roathe
 *		Nick Shanks
 *		Nick Pissaro Jr. (NP)
 */

// 05/10/01 - GAB: MPW environment support
#ifdef __MPW__
#include "MPWIncludes.h"
#endif

#include "HexCompare.h"
#include "EditRoutines.h"
#include "EditScrollbar.h"
#include "Main.h"
#include "Prefs.h"
#include "Utility.h"

// variables
//LR 180 extern short CompareFlag;
extern unsigned char	gSearchBuffer[256];

WindowRef		CompWind1 = NULL,
				CompWind2 = NULL;
Boolean			WeFoundWind1 = false,
				WeFoundWind2 = false;

/*** PERFORM TEXT COMPARE ***/
//LR 185 -- this one routine now performs EITHER match or diff comparisons...and WAY faster :)
Boolean PerformTextCompare( EditWindowPtr dWin1, EditWindowPtr dWin2 )
{
//	returns differences in data of two edit windows
	Byte		ch1, ch2;
	short		matchIdx, matchCnt;
	long		addr1, addr2, matchAddr1 = 0, matchAddr2 = 0, adjust;
	register 	EditChunk **c1, **c2;

	MySetCursor( C_Watch );

	// Search in Direction gPrefs.searchForward for text gSearchBuffer

	addr1 = dWin1->startSel;
	addr2 = dWin2->startSel;

	if( gPrefs.searchForward )
		adjust = 1;
	else
		adjust = -1;
	
	// 1 = byte, 2 = words, 4 = longs, ect...
	matchCnt = gPrefs.searchSize;

	//LR 185 -- we handle the chucks ourself to speed up searching!
	//			get the chunk for the current address & load it.

	c1 = GetChunkByAddr( dWin1, addr1 );
	c2 = GetChunkByAddr( dWin2, addr2 );
	if( !c1 || !c2 )
		goto Failure;	// should never happen, but...

	LoadChunk( dWin1, c1 );
	LoadChunk( dWin2, c2 );

	matchIdx = 0;
	addr1 += adjust;
	addr2 += adjust;

	//LR 185 -- re-write of compare loop; it was pathetically slow, etc.
	while( addr1 >= 0 && addr1 < dWin1->fileSize && addr2 >= 0 && addr2 < dWin2->fileSize )
	{
		if( !(addr1 % 4096) )		//LR 1.72 -- don't slow our searches down unnecessarily!
		{
			if( CheckForAbort() )	//LR: 1.66 - allow user to abort the search
				break;
		}

//185		ch1 = GetByte( dWin1, addr1 );
//185		ch2 = GetByte( dWin2, addr2 );
		ch1 = (Byte) (*(*c1)->data)[addr1 - (*c1)->addr];
		ch2 = (Byte) (*(*c2)->data)[addr2 - (*c2)->addr];
		if( (gPrefs.searchType == CM_Different && ch1 != ch2) || (gPrefs.searchType == CM_Match && ch1 == ch2) )
		{
			if( matchIdx == 0 )
			{
				matchAddr1 = addr1;
				matchAddr2 = addr2;
			}
			++matchIdx;

			if( matchIdx >= matchCnt )
				goto Success;

			++addr1;
			++addr2;
			if( addr1 == dWin1->fileSize )
			{
				matchIdx = 0;
				addr1 = matchAddr1;
				addr2 = matchAddr2;
			}
			else
				continue;
		}
		else if( matchIdx )	// if we were in a match, back it out!
		{
			matchIdx = 0;
			addr1 = matchAddr1;
			addr2 = matchAddr2;
		}
		addr1 += adjust;
		addr2 += adjust;

		//LR 185 -- OK, here we must handle moving to a new chunk if outside current one
		if( addr1 < (*c1)->addr )
		{
			UnloadChunk( dWin1, c1, true );
			c1 = (*c1)->prev;
			goto nc1;
		}
		else if( addr1 >= (*c1)->addr + (*c1)->size )
		{
			UnloadChunk( dWin1, c1, true );
			c1 = (*c1)->next;
nc1:
			if( !c1 )
				goto Failure;

			LoadChunk( dWin1, c1 );	// no check, most likely not loaded, and checked in routine anyway
		}

		if( addr2 < (*c2)->addr )
		{
			UnloadChunk( dWin2, c2, true );
			c2 = (*c2)->prev;
			goto nc2;
		}
		else if( addr2 >= (*c2)->addr + (*c2)->size )
		{
			UnloadChunk( dWin2, c2, true );
			c2 = (*c2)->next;
nc2:
			if( !c2 )
				goto Failure;

			LoadChunk( dWin2, c2 );	// no check, most likely not loaded, and checked in routine anyway
		}
	}

Failure:
	SysBeep( 1 );
	MySetCursor( C_Arrow );
	return false;

Success:
	SelectWindow( dWin1->oWin.theWin );
	dWin1->startSel = matchAddr1;
	dWin1->endSel = dWin1->startSel + gPrefs.searchSize;
	ScrollToSelection( dWin1, dWin1->startSel, true );

	SelectWindow( dWin2->oWin.theWin );
	dWin2->startSel = matchAddr2;
	dWin2->endSel = dWin2->startSel + gPrefs.searchSize;
	ScrollToSelection( dWin2, dWin2->startSel, true );

	MySetCursor( C_Arrow );
	return true;
}

/*** PERFORM TEXT MATCH COMPARE ***/
/*185
Boolean PerformTextMatchCompare( EditWindowPtr dWin, EditWindowPtr dWin2 )
{
//	returns matches in data of two edit windows
	Byte		ch, ch2;
	short			matchIdx, matchCnt;
	long		addr, addr2, matchAddr;

	MySetCursor( C_Watch );

	// Search in Direction gPrefs.searchForward
	// for text gSearchBuffer

	if( gPrefs.searchForward )
	{
		addr = dWin->endSel;
		addr2 = dWin2->endSel;
	}
	else
	{
		addr = dWin->startSel - 1;
		addr2 = dWin2->startSel - 1;
		if( addr < 0 )	return false;
		if( addr2 < 0 )	return false;
	}
	
	// 1 = byte, 2 = words, 4 = longs, ect...
	matchCnt=gPrefs.searchSize+1;
	if( gPrefs.searchSize==CM_Long ) matchCnt += 1;

	matchIdx = 0;
	while ( !CheckForAbort() )		// LR: 1.7 -- allow aborting compares!
	{
		ch = GetByte( dWin, addr );
		ch2 = GetByte( dWin2, addr2 );
		if( ch == ch2 )
		{
			if( matchIdx == 0 ) matchAddr = addr;
			++matchIdx;
			if( matchIdx >= matchCnt )
				goto Success;
			++addr;
			++addr2;
			if( addr == dWin->fileSize )
			{
				matchIdx = 0;
				addr = matchAddr;
			}
			else
				continue;
		}
		else
		{
			if( matchIdx ) {
				matchIdx = 0;
				addr = matchAddr;
			}
		}
		if( gPrefs.searchForward )
		{
			++addr;
			++addr2;
			if( addr2 == dWin2->fileSize )
				goto Failure;
			if( addr == dWin->fileSize )
				goto Failure;
		}
		else
		{
			--addr;
			--addr2;
			if( addr < 0 )
				goto Failure;
			if( addr2 < 0 )
				goto Failure;
		}
	}

Failure:
	SysBeep( 1 );
	MySetCursor( C_Arrow );
	return false;

Success:
	SelectWindow( dWin->oWin.theWin );
	dWin->startSel = matchAddr;
	dWin->endSel = dWin->startSel + gPrefs.searchSize + 1;
	if( gPrefs.searchSize==CM_Long ) dWin->endSel += 1;
	ScrollToSelection( dWin, dWin->startSel, true );

	SelectWindow( dWin2->oWin.theWin );
	dWin2->startSel = matchAddr;
	dWin2->endSel = dWin2->startSel + gPrefs.searchSize + 1;
	if( gPrefs.searchSize==CM_Long ) dWin2->endSel += 1;
	ScrollToSelection( dWin2, dWin2->startSel, true );

	MySetCursor( C_Arrow );
	return true;
}
*/

/*** DO COMPARISON ***/
void DoComparison( void )
{
	GrafPtr		oldPort;
	
	WindowRef	theWin;
	
	DialogPtr	pDlg;
// LR: v1.6.5	Handle 		iHandle;
// LR: v1.6.5	Rect 		iRect;
	short iType, oldDir = gPrefs.searchForward;
	EventRecord	theEvent;

	// put up dialog and let user shorteract

	GetPort( &oldPort );

	pDlg = GetNewDialog ( dlgCompare, 0L, kFirstWindowOfClass );

	MoveWindow( GetDialogWindow( pDlg ), 22, g.maxHeight -64 +8, true );
//LR 1.73	SetPort( (GrafPtr)GetDialogPort( pDlg ) );
	SetPortDialogPort( pDlg );

	SetDialogDefaultItem( pDlg, 1 );	// LR: v1.6.5 LR -- correct way of showing default button
/*
	GetDialogItem ( pDlg, 1, &iType, &iHandle, &iRect );	// ring around OK
	PenSize( 3, 3 );
	InsetRect( &iRect, -4, -4 );
	FrameRoundRect( &iRect, 16, 16 );
*/
//LR 180	DrawPage( (EditWindowPtr) GetWRefCon( CompWind1 ) );
	UpdateOnscreen( CompWind1 );
//LR 180	DrawPage( (EditWindowPtr) GetWRefCon( CompWind2 ) );
	UpdateOnscreen( CompWind2 );
			
	// show the contents of the windows
	ShowWindow( GetDialogWindow( pDlg ) );
	DrawDialog( pDlg );

	// handle event processing
	do
	{
		theWin = NULL;
		WaitNextEvent( everyEvent, &theEvent, 10L, NULL );

		if( !CompWind1 || !CompWind2 )	//1.73 LR :exit if user closes one of the windows!
			iType = 4;
		else
			iType = 0;

		// If the click is in the dialog window, make sure it always processes the click, regardless
		// of whether it is on top or not.--NPJr.
		if( theEvent.what == mouseDown )
			FindWindow( theEvent.where, &theWin );
		
		if ( GetDialogWindow( pDlg ) == theWin || IsDialogEvent( &theEvent ) )
		{
			if( theWin != FrontNonFloatingWindow() )
				SelectWindow( theWin );
			
			DialogSelect( &theEvent, &pDlg, &iType );
		
			if( iType==1 )			// handle find forward here
			{
				gPrefs.searchForward = true;
				PerformTextCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );

/*185				if( gPrefs.searchType == CM_Match )
					PerformTextMatchCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );
				else
					PerformTextDifferenceCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );
*/
				SelectWindow( (WindowRef)pDlg );
			}
			if( iType==3 )			// handle find backward here
			{
				gPrefs.searchForward = false;
				PerformTextCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );

/*185				if( gPrefs.searchType == CM_Match )
					PerformTextMatchCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );
				else
					PerformTextDifferenceCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );
*/
				SelectWindow( (WindowRef)pDlg );
			}
		}
		else
			DoEvent( &theEvent );

	} while (!g.quitFlag && (iType != 2) && (iType != 4));	// 2 = Done, 4 = Drop out and edit
			
	// Close dialog
	DisposeDialog( pDlg );
	SetPort( oldPort );

	if( iType == 2 )
	{
		if( CompWind1 && ! WeFoundWind1 )
			CloseEditWindow( CompWind1 );		// close windows (1.7 vs disposing them!) if done ( ie, not editing )
		if( CompWind2 && ! WeFoundWind2 )
			CloseEditWindow( CompWind2 );
	}

	CompWind1 = CompWind2 = NULL;
	WeFoundWind1 = WeFoundWind2 = false;
	gPrefs.searchForward = oldDir;
}

/**** GET COMPARE FILES ***/
//LR 180 -- pass in modifiers so we can see if we want to force file selection dialogs

Boolean GetCompareFiles( short modifiers )
{
//	main handler for the compare of the contents of two windows
//LR 180	short 		iType;
	EditWindowPtr ew1, ew2;

	// close previous file compare windows if open

	// NP 177 -- if windows are open use them instead of asking.
	// LR 180 -- if 'Option' pressed then we do it the old-fashion way :)

	if( modifiers & optionKey )
	{	
		if( CompWind1 )	DisposeEditWindow( CompWind1 );
		if( CompWind2 )	DisposeEditWindow( CompWind2 );
	}
	else	// NP 177 -- See if there are windows we can use already. Rearrange them for doing the comparison.
	{
		CompWind1 = CompWind2 = NULL;
		WeFoundWind1 = WeFoundWind2 = false;
		
		ew1 = FindFirstEditWindow();			// look for 1st window
		if( ew1 )
		{
			CompWind1 = ew1->oWin.theWin;
			WeFoundWind1 = true;				// We found it, we shouldn't destroy it.
			SizeEditWindow( CompWind1, kWindowCompareTop );

			ew2 = FindNextEditWindow( ew1 );	// look for 2nd window
			if( ew2 )
			{
				CompWind2 = ew2->oWin.theWin;
				WeFoundWind2 = true;
				SizeEditWindow( CompWind2, kWindowCompareBtm );
			}
		}
	}

	// NP 177 -- Open files for comparing if we did not find windows to start with.
	
	if( ! CompWind1 )
	{
//LR 180		CompareFlag=1;
		if( -1 == AskEditWindow( kWindowCompareTop ) )
		{
//LR 180			CompareFlag = 0;
			return false;		// if Cancel, exit
		}
	}
	
	if( ! CompWind2 )
	{
//LR 180		CompareFlag=2;
		if( -1 == AskEditWindow( kWindowCompareBtm ) )
			goto compexit;
	}

	if( CompWind1 == CompWind2 || !CompWind1 || !CompWind2 )	//LR 180 -- if same window we compare to an open file
		goto compexit;

	ew1 = (EditWindowPtr)GetWRefCon( CompWind1 );
	ew2 = (EditWindowPtr)GetWRefCon( CompWind2 );

	// LR: v1.6.5 don't allow comparing a file to itself!
	if( ew1 && ew2	&& (ew1->fsSpec.vRefNum == ew2->fsSpec.vRefNum)
					&& (ew1->fsSpec.parID == ew2->fsSpec.parID)
					&& !MacCompareString( ew1->fsSpec.name, ew2->fsSpec.name, NULL ) )
	{
compexit:
		if( CompWind1 && ! WeFoundWind1 )
			CloseEditWindow( CompWind1 );		// close windows (1.7 vs disposing them!) if done ( ie, not editing )
		if( CompWind2 && ! WeFoundWind2 )
			CloseEditWindow( CompWind2 );
		
		CompWind1 = CompWind2 = NULL;
		WeFoundWind1 = WeFoundWind2 = false;
//LR 180		CompareFlag = 0;

		return false;
	}

//LR 180	CompareFlag = 0;
	return true;
}

/*** COMPARISON PREFERENCES ***/
void ComparisonPreferences( void )
{
//	handler for the options dialog for compare.
	GrafPtr		oldPort;
	DialogPtr	pDlg;
// LR: v1.6.5	Handle 		iHandle;
	short 		iType, radio1, radio2;
// LR: v1.6.5	Rect 		iRect;

	GetPort( &oldPort );
	// make dialog
	pDlg = GetNewDialog ( dlgComparePref, 0L, kFirstWindowOfClass );

	SetPortDialogPort( pDlg );

	radio1 = gPrefs.searchSize;
	radio2 = gPrefs.searchType;

	// init radio buttons to current settings...
	SetControl( pDlg, CP_Bytes, gPrefs.searchSize == CM_Byte );
	SetControl( pDlg, CP_Words, gPrefs.searchSize == CM_Word );
	SetControl( pDlg, CP_Longs, gPrefs.searchSize == CM_Long );
	SetControl( pDlg, CP_Different, gPrefs.searchType == CM_Different );
	SetControl( pDlg, CP_Match, gPrefs.searchType == CM_Match );
	SetControl( pDlg, CP_Case, gPrefs.searchCase );

	SetDialogDefaultItem( pDlg, CP_Done );	// LR: v1.6.5 LR -- correct way of showing default button

	ShowWindow( GetDialogWindow( pDlg ) );	// LR 1.73 -- dialog now hidden at first launch!

	// handle event processing
	do
	{
		ModalDialog( NULL, &iType );
		switch ( iType )
		{
			// handle each radio button...
			case CP_Bytes:
				radio1 = CM_Byte;
				SetControl( pDlg, CP_Bytes, 1 );
				SetControl( pDlg, CP_Words, 0 );
				SetControl( pDlg, CP_Longs, 0 );
				break;
			
			case CP_Words:
				radio1 = CM_Word;
				SetControl( pDlg, CP_Bytes, 0 );
				SetControl( pDlg, CP_Words, 1 );
				SetControl( pDlg, CP_Longs, 0 );
				break;
			
			case CP_Longs:
				radio1 = CM_Long;
				SetControl( pDlg, CP_Bytes, 0 );
				SetControl( pDlg, CP_Words, 0 );
				SetControl( pDlg, CP_Longs, 1 );
				break;
				
			case CP_Different:
				radio2 = CM_Different;
				SetControl( pDlg, CP_Different, 1 );
				SetControl( pDlg, CP_Match, 0 );
				break;
			
			case CP_Match:
				radio2 = CM_Match;
				SetControl( pDlg, CP_Different, 0 );
				SetControl( pDlg, CP_Match, 1 );
				break;

			case CP_Case:
				gPrefs.searchCase ^= 1;
				SetControl( pDlg, CP_Case, gPrefs.searchCase );
				break;
		}
	} while( ( iType != CP_Done ) && ( iType != CP_Cancel ) );
	
	if( iType==CP_Done )
	{
		// change flags based on which one is selected
		gPrefs.searchSize = radio1;
		gPrefs.searchType = radio2;
	}		
	// close theWin
	DisposeDialog( pDlg );
	SetPort( oldPort );
}
