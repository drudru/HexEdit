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
 * Copyright (C) Copyright © 1996-2000.
 * All Rights Reserved.
 * 
 * Contributor(s):
 *		Nick Shanks
 */

#include "HexCompare.h"
#include "EditRoutines.h"
#include "EditScrollbar.h"
#include "Main.h"
#include "Utility.h"

extern globals g;
extern prefs_t prefs;

// variables
extern short CompareFlag;
extern WindowRef CompWind1, CompWind2;
extern unsigned char	gSearchBuffer[256];

/*** PERFORM TEXT DIFFERENCE COMPARE ***/
Boolean PerformTextDifferenceCompare( EditWindowPtr dWin, EditWindowPtr dWin2 )
{
//	returns differences in data of two edit windows
	Byte		ch, ch2;
	short		matchIdx, matchCnt;
	long		addr, addr2, matchAddr;

	MySetCursor( C_Watch );

	// Search in Direction prefs.searchForward for text gSearchBuffer

	if( prefs.searchForward )
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
	matchCnt = prefs.searchSize+1;
	if( prefs.searchSize == CM_Long ) matchCnt += 1;


	matchIdx = 0;
	while ( 1 )
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
		if( prefs.searchForward )
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
	dWin->endSel = dWin->startSel + prefs.searchSize + 1;
	if( prefs.searchSize==CM_Long ) dWin->endSel += 1;
	ScrollToSelection( dWin, dWin->startSel, true, true );

	SelectWindow( dWin2->oWin.theWin );
	dWin2->startSel = matchAddr;
	dWin2->endSel = dWin2->startSel + prefs.searchSize + 1;
	if( prefs.searchSize==CM_Long ) dWin2->endSel += 1;
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

	// Search in Direction prefs.searchForward
	// for text gSearchBuffer

	if( prefs.searchForward )
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
	matchCnt=prefs.searchSize+1;
	if( prefs.searchSize==CM_Long ) matchCnt += 1;

	matchIdx = 0;
	while ( 1 )
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
		if( prefs.searchForward )
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
	dWin->endSel = dWin->startSel + prefs.searchSize + 1;
	if( prefs.searchSize==CM_Long ) dWin->endSel += 1;
	ScrollToSelection( dWin, dWin->startSel, true, true );

	SelectWindow( dWin2->oWin.theWin );
	dWin2->startSel = matchAddr;
	dWin2->endSel = dWin2->startSel + prefs.searchSize + 1;
	if( prefs.searchSize==CM_Long ) dWin2->endSel += 1;
	ScrollToSelection( dWin2, dWin2->startSel, true, true );

	MySetCursor( C_Arrow );
	return true;
}

/*** DO COMPARISON ***/
void DoComparison( void )
{
	GrafPtr		oldPort;
	DialogPtr	pDlg;
// LR: v1.6.5	Handle 		iHandle;
// LR: v1.6.5	Rect 		iRect;
	short iType, oldDir = prefs.searchForward;
	EventRecord	theEvent;

	// put up dialog and let user shorteract

	GetPort( &oldPort );

	pDlg = GetNewDialog ( dlgCompare, 0L, (WindowRef)-1 );
#if TARGET_API_MAC_CARBON
	MoveWindow( GetDialogWindow( pDlg ), 22, g.maxHeight -64 +8, true );
	ShowWindow( GetDialogWindow( pDlg ) );
	SetPort( GetDialogPort( pDlg ) );
#else
	MoveWindow( pDlg, 22, g.maxHeight -64 +8, true );		// move dialog to 14, 400?
	ShowWindow( pDlg );
	SetPort( pDlg );
#endif

	SetDialogDefaultItem( pDlg, 1 );	// LR: v1.6.5 LR -- correct way of showing default button
/*
	GetDialogItem ( pDlg, 1, &iType, &iHandle, &iRect );	// ring around OK
	PenSize( 3, 3 );
	InsetRect( &iRect, -4, -4 );
	FrameRoundRect( &iRect, 16, 16 );
*/
	// show the contents of the windows

	DrawPage( (EditWindowPtr) GetWRefCon( CompWind1 ) );
	UpdateOnscreen( CompWind1 );
	DrawPage( (EditWindowPtr) GetWRefCon( CompWind2 ) );
	UpdateOnscreen( CompWind2 );
			
	// handle event processing
	do
	{
		WaitNextEvent( everyEvent, &theEvent, 10L, NULL );
		iType=0;
		if( IsDialogEvent( &theEvent ) )
		{
			DialogSelect( &theEvent, &pDlg, &iType );
		
			if( iType==1 )			// handle find forward here
			{
				prefs.searchForward = true;
				if( prefs.searchType == CM_Match )
					PerformTextMatchCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );
				else
					PerformTextDifferenceCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );

				SelectWindow( (WindowRef) pDlg );
			}
			if( iType==3 )			// handle find backward here
			{
				prefs.searchForward = false;

				if( prefs.searchType == CM_Match )
					PerformTextMatchCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );
				else
					PerformTextDifferenceCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );

				SelectWindow( (WindowRef) pDlg );
			}
		}
		else DoEvent( &theEvent );

	} while (!g.quitFlag && (iType != 2) && (iType != 4));	// 2 = Done, 4 = Drop out and edit
			
	// Close dialog
	DisposeDialog( pDlg );
	SetPort( oldPort );

	if( iType == 2 )
	{
		if( CompWind1 )	DisposeEditWindow( CompWind1 );		// close windows if done ( ie, not editing )
		if( CompWind2 )	DisposeEditWindow( CompWind2 );
	}

	CompWind1 = CompWind2 = NULL;
	prefs.searchForward = oldDir;
}

/**** GET COMPARE FILES ***/
Boolean GetCompareFiles( void )
{
//	main handler for the compare of the contents of two windows
	short 		iType;
	EditWindowPtr ew1, ew2;

	// close previous file compare windows if open

	if( CompWind1 )	DisposeEditWindow( CompWind1 );
	if( CompWind2 )	DisposeEditWindow( CompWind2 );

	// Open files ( and assoc. windows ) for comparing

	CompWind1 = CompWind2 = NULL;
	CompareFlag=1;
	iType = AskEditWindow();
	if( iType==-1 )
	{
		CompareFlag = 0;
		return false;		// if Cancel, exit
	}

	CompareFlag=2;
	iType = AskEditWindow();
	if( iType==-1 )
	{
		if( CompWind1 )
			CloseEditWindow( CompWind1 );	// Cancel = exit (1.65, did retry .. but was confusing)
		return false;
	}

	ew1 = (EditWindowPtr)GetWRefCon( CompWind1 );
	ew2 = (EditWindowPtr)GetWRefCon( CompWind2 );


	// LR: v1.6.5 don't allow comparing a file to itself!
	if( ew1 && ew2	&& (ew1->fsSpec.vRefNum == ew2->fsSpec.vRefNum)
					&& (ew1->fsSpec.parID == ew2->fsSpec.parID)
					&& !MacCompareString( ew1->fsSpec.name, ew2->fsSpec.name, NULL ) )
	{
		CloseEditWindow( CompWind1 );
		CloseEditWindow( CompWind2 );
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
	pDlg = GetNewDialog ( dlgComparePref, 0L, (WindowRef)-1 );

	SetPortDialogPort( pDlg );

	radio1 = prefs.searchSize;
	radio2 = prefs.searchType;

	// init radio buttons to current settings...
	SetControl( pDlg, CP_Bytes, prefs.searchSize == CM_Byte );
	SetControl( pDlg, CP_Words, prefs.searchSize == CM_Word );
	SetControl( pDlg, CP_Longs, prefs.searchSize == CM_Long );
	SetControl( pDlg, CP_Different, prefs.searchType == CM_Different );
	SetControl( pDlg, CP_Match, prefs.searchType == CM_Match );
	SetControl( pDlg, CP_Case, prefs.searchCase );

	SetDialogDefaultItem( pDlg, CP_Done );	// LR: v1.6.5 LR -- correct way of showing default button
			
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
				prefs.searchCase ^= 1;
				SetControl( pDlg, CP_Case, prefs.searchCase );
				break;
		}
	} while( ( iType != CP_Done ) && ( iType != CP_Cancel ) );
	
	if( iType==CP_Done )
	{
		// change flags based on which one is selected
		prefs.searchSize = radio1;
		prefs.searchType = radio2;
	}		
	// close theWin
	DisposeDialog( pDlg );
	SetPort( oldPort );
}
