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
extern short CompareFlag;
extern unsigned char	gSearchBuffer[256];

WindowRef		CompWind1 = NULL,
				CompWind2 = NULL;
Boolean			WeFoundWind1 = false,
				WeFoundWind2 = false;

/*** PERFORM TEXT DIFFERENCE COMPARE ***/
Boolean PerformTextDifferenceCompare( EditWindowPtr dWin, EditWindowPtr dWin2 )
{
//	returns differences in data of two edit windows
	Byte		ch, ch2;
	short		matchIdx, matchCnt;
	long		addr, addr2, matchAddr;

	MySetCursor( C_Watch );

	// Search in Direction gPrefs.searchForward for text gSearchBuffer

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
	matchCnt = gPrefs.searchSize+1;
	if( gPrefs.searchSize == CM_Long ) matchCnt += 1;


	matchIdx = 0;
	while ( !CheckForAbort() )		// LR: 1.7 -- allow aborting compares!
	{
		ch = GetByte( dWin, addr );
		ch2 = GetByte( dWin2, addr2 );
		if( ch != ch2 ) {				// change this if you want compare for similarities
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
			if( matchIdx )
			{
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
	ScrollToSelection( dWin, dWin->startSel, true, true );

	SelectWindow( dWin2->oWin.theWin );
	dWin2->startSel = matchAddr;
	dWin2->endSel = dWin2->startSel + gPrefs.searchSize + 1;
	if( gPrefs.searchSize==CM_Long ) dWin2->endSel += 1;
	ScrollToSelection( dWin2, dWin2->startSel, true, true );

	MySetCursor( C_Arrow );
	return true;
}

/*** PERFORM TEXT MATCH COMPARE ***/
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
	ScrollToSelection( dWin, dWin->startSel, true, true );

	SelectWindow( dWin2->oWin.theWin );
	dWin2->startSel = matchAddr;
	dWin2->endSel = dWin2->startSel + gPrefs.searchSize + 1;
	if( gPrefs.searchSize==CM_Long ) dWin2->endSel += 1;
	ScrollToSelection( dWin2, dWin2->startSel, true, true );

	MySetCursor( C_Arrow );
	return true;
}

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
	// show the contents of the windows
	ShowWindow( GetDialogWindow( pDlg ) );

	DrawPage( (EditWindowPtr) GetWRefCon( CompWind1 ) );
	UpdateOnscreen( CompWind1 );
	DrawPage( (EditWindowPtr) GetWRefCon( CompWind2 ) );
	UpdateOnscreen( CompWind2 );
			
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
		
		if ( GetDialogWindow( pDlg ) == theWin ||
			IsDialogEvent( &theEvent ) )
		{
			if( theWin != FrontNonFloatingWindow() )
			{
				// Make it so...
				SelectWindow( theWin );
			}
			
			DialogSelect( &theEvent, &pDlg, &iType );
		
			if( iType==1 )			// handle find forward here
			{
				gPrefs.searchForward = true;
				if( gPrefs.searchType == CM_Match )
					PerformTextMatchCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );
				else
					PerformTextDifferenceCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );

				SelectWindow( (WindowRef)pDlg );
			}
			if( iType==3 )			// handle find backward here
			{
				gPrefs.searchForward = false;

				if( gPrefs.searchType == CM_Match )
					PerformTextMatchCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );
				else
					PerformTextDifferenceCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );

				SelectWindow( (WindowRef)pDlg );
			}
		}
		else DoEvent( &theEvent );

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
Boolean GetCompareFiles( void )
{
//	main handler for the compare of the contents of two windows
	short 		iType;
	EditWindowPtr ew1, ew2;
	KeyMap keys;
	Boolean newWindows;

	// LR 177 -- find out if we want to use existing or new windows (ie, read option key)
	GetKeys( keys );
	newWindows = ( keys[1] & (1<<2) );

	// close previous file compare windows if open

	// NP 177 -- if windows are open use them instead of asking.
	// LR 177 -- if 'Option' pressed then we do it the old-fashion way :)

	if( newWindows )
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
		CompareFlag=1;
		iType = AskEditWindow();
		if( iType==-1 )
		{
			CompareFlag = 0;
			return false;		// if Cancel, exit
		}
	}
	
	if( ! CompWind2 )
	{
		CompareFlag=2;
		iType = AskEditWindow();
		if( iType==-1 )
		{
			if( CompWind1 )
				CloseEditWindow( CompWind1 );	// Cancel = exit (1.65, did retry .. but was confusing)
			return false;
		}
	}

	ew1 = (EditWindowPtr)GetWRefCon( CompWind1 );
	ew2 = (EditWindowPtr)GetWRefCon( CompWind2 );

	// LR: v1.6.5 don't allow comparing a file to itself!
	if( ew1 && ew2	&& (ew1->fsSpec.vRefNum == ew2->fsSpec.vRefNum)
					&& (ew1->fsSpec.parID == ew2->fsSpec.parID)
					&& !MacCompareString( ew1->fsSpec.name, ew2->fsSpec.name, NULL ) )
	{
		if( CompWind1 && ! WeFoundWind1 )
			CloseEditWindow( CompWind1 );		// close windows (1.7 vs disposing them!) if done ( ie, not editing )
		if( CompWind2 && ! WeFoundWind2 )
			CloseEditWindow( CompWind2 );
		
		CompWind1 = CompWind2 = NULL;
		WeFoundWind1 = WeFoundWind2 = false;
		CompareFlag = 0;

		return false;
	}

	CompareFlag = 0;
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
