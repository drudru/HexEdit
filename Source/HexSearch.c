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

#include <stdio.h>
#include <ctype.h>

#include "HexSearch.h"
#include "EditRoutines.h"
#include "EditScrollbar.h"
#include "Menus.h"
#include "Prefs.h"
#include "Utility.h"

/*** SET SEARCH BUTTONS ***/
void SetSearchButtons( void )
{
// Prevent flashing, adapted from Max Horn's mod
	if( g.searchWin )
	{
		if( *g.searchText && g.searchDisabled )
		{
			EnableButton( g.searchWin, SearchForwardItem );
			EnableButton( g.searchWin, SearchBackwardItem );
			g.searchDisabled = false;
		}
		else if( !*g.searchText && !g.searchDisabled )
		{
			DisableButton( g.searchWin, SearchForwardItem );
			DisableButton( g.searchWin, SearchBackwardItem );
			g.searchDisabled = true;
		}
	}
}

// ----------------------------
void OpenSearchDialog( void )
{
	MySetCursor( C_Arrow );

	if( !FindFirstEditWindow() )
		return;

	// If Dialog Window isn't open
	if( !g.searchWin )
	{
		// Open Dialog Window
		g.searchWin = GetNewDialog( dlgSearch, NULL, (WindowRef) -1L );
		if( g.searchWin )
		{
			// Convert Existing Search Scrap, if it exists to text
			SetText( g.searchWin, SearchTextItem, g.searchText );

			// Set Radio Buttons
			SetControl( g.searchWin, HexModeItem, prefs.searchMode == EM_Hex );
			SetControl( g.searchWin, AsciiModeItem, prefs.searchMode == EM_Ascii );
			SetControl( g.searchWin, MatchCaseItem, prefs.searchCase );
			if( EM_Hex == prefs.searchMode )
				DisableButton( g.searchWin, MatchCaseItem );	// LR 1.65
			else
				EnableButton( g.searchWin, MatchCaseItem );	// LR 1.65

			// NS: added disabling of buttons if no text is entered
// LR: not needed			GetText( g.searchWin, SearchTextItem, g.searchText );
			SelectDialogItemText( g.searchWin, SearchTextItem, 0, 32767 );	// LR: make immediately editable
			g.searchDisabled = false;
			SetSearchButtons();
		}
	}
#if TARGET_API_MAC_CARBON
	SelectWindow( GetDialogWindow( g.searchWin ) );
	ShowWindow( GetDialogWindow( g.searchWin ) );
#else
	SelectWindow( g.searchWin );
	ShowWindow( g.searchWin );
#endif
}

/*** PERFORM TEXT SEARCH ***/
void PerformTextSearch( EditWindowPtr dWin )
{
	short		ch, matchIdx;
	long		addr, matchAddr;

	// LR: v1.6.5 if not passed a window, get first one
	if( !dWin )
	{
		dWin = FindFirstEditWindow();
		if( !dWin )
			return;
	}

	// Search in Direction prefs.searchForward
	// for text g.searchBuffer

	if( prefs.searchForward )
		addr = dWin->startSel;
	else
		addr = dWin->endSel - 1;

	// LR: 1.7 -- make sure we are searching in OK memory (ie, empty window bug fix)
	if( addr < 0 || !dWin->fileSize)
		goto Failure;

	MySetCursor( C_Watch );

	matchIdx = 0;
	while( !CheckForAbort() )	//LR: 1.66 - allow user to abort the search
	{
		ch = GetByte( dWin, addr );
		if( !prefs.searchCase && prefs.searchMode != EM_Hex )
			ch = toupper( ch );
		if( ch == g.searchBuffer[matchIdx+1] )
		{
			if( matchIdx == 0 )
				matchAddr = addr;
			++matchIdx;
			if( matchIdx >= g.searchBuffer[0] )
				goto Success;
			++addr;
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
			if( addr >= dWin->fileSize )
				goto Failure;
		}
		else
		{
			--addr;
			if( addr < 0 ) goto Failure;
		}
	}

Failure:
	SysBeep( 1 );
	MySetCursor( C_Arrow );
	return;

Success:
	if( dWin != (EditWindowPtr) GetWRefCon( FrontNonFloatingWindow() ) )
		SelectWindow( dWin->oWin.theWin );

	dWin->startSel = matchAddr;
	dWin->endSel = dWin->startSel + g.searchBuffer[0];

	ScrollToSelection( dWin, dWin->startSel, true, true );
	MySetCursor( C_Arrow );
}

#define GGotoAddr		1
#define GAddrItem		3
#define GHexItem		4
#define GDecimalItem	5
// #define GUserItem		6

/*** GOTO USER ITEM ***/
/* 1.65 LR -- removed ... what was this?
static pascal void GotoUserItem( DialogPtr dp, short itemNbr )
{
	switch( itemNbr )
	{
		case GUserItem:
// 			MyOutlineButton( dp, 1, qd.black );
			SetDialogDefaultItem( dp, itemNbr );		// NS: above function removed
			break;
	}
}
*/

/*** OPEN GOTO ADDRESS ***/
OSStatus OpenGotoAddress( void )
{
// 	GrafPtr		savePort;
// 	short		itemHit;
// 	short		t;
// 	Handle		h;
// 	Rect		r;

// 	GetPort( &savePort );
	MySetCursor( C_Arrow );

	if( !FindFirstEditWindow() )
		return paramErr;

	if( !g.gotoWin )
	{
		g.gotoWin = GetNewDialog( dlgGoto, NULL, kFirstWindowOfClass );
		if( !g.gotoWin ) return paramErr;
	// 	GetDialogItem( g.gotoWin, GUserItem, &t, &h, &r );
	// 	SetDialogItem( g.gotoWin, GUserItem, t, (Handle) GotoUserItem, &r );

		SetText( g.gotoWin, GAddrItem, g.gotoText );
		// Set Radio Buttons
		SetControl( g.gotoWin, GHexItem, prefs.gotoMode == EM_Hex );
		SetControl( g.gotoWin, GDecimalItem, prefs.gotoMode == EM_Decimal );
		SelectDialogItemText( g.gotoWin, GAddrItem, 0, 32767 );
	}
#if TARGET_API_MAC_CARBON
	else SelectWindow( GetDialogWindow( g.gotoWin ) );

	ShowWindow( GetDialogWindow( g.gotoWin ) );
#else
	else SelectWindow( g.gotoWin );

		ShowWindow( g.gotoWin );
#endif
	return noErr;
}

/*** DO MODELESS DIALOG EVENT ***/
void DoModelessDialogEvent( EventRecord *theEvent )
{
	DialogPtr	whichDlog;
	short		itemHit;

//#if TARGET_API_MAC_CARBON	// LR: v1.6.5 do this for all refs
//LR: 1.7 -- fix for OS X, which requires below code	whichDlog = (DialogRef)FrontWindow();
	whichDlog = GetDialogFromWindow( FrontNonFloatingWindow() );
//#else
//	whichDlog = FrontWindow();
//#endif

	// Do Event Filtering
	if( whichDlog && theEvent->what == keyDown )	//LR: 1.66 avoid NULL reference
	{
		// Process Edit Keys
		if( ((theEvent->message & charCodeMask) == kReturnCharCode) ||
			((theEvent->message & charCodeMask) == kEnterCharCode) )
		{
			itemHit = 1;
			SimulateButtonPress( whichDlog, 1 );
			goto ButtonHit;
		}
		if( theEvent->modifiers & cmdKey )
		{
/*LR - allow all menu options active
			switch ( ( theEvent->message & keyCodeMask ) >> 8 ) {
			case 0x0D:	// W
				DisposeDialog( g.searchWin );
				g.searchWin = NULL;
				return;
			case 0x0C:	// Q
				if( CloseAllEditWindows() )
					g.quitFlag = true;
				return;
			}
*/
			AdjustMenus();
			HandleMenu( MenuKey( (char) (theEvent->message & charCodeMask) ) );
			return;
		}
	}

	// NS: added disabling of buttons if no text is entered
	if( theEvent->what == nullEvent && whichDlog == g.searchWin )	// LR: v1.6.5 must check which dialog!
	{
		GetText( g.searchWin, SearchTextItem, g.searchText );
		SetSearchButtons();
	}

	if( DialogSelect( theEvent, &whichDlog, &itemHit ) ) {
ButtonHit:
		if( whichDlog == g.searchWin )
		{
			switch( itemHit )
			{
			case SearchForwardItem:
			case SearchBackwardItem:
				prefs.searchForward = ( itemHit == SearchForwardItem );
				GetText( g.searchWin, SearchTextItem, g.searchText );
				if( StringToSearchBuffer( prefs.searchCase ) )
					PerformTextSearch( NULL );
				break;
			case HexModeItem:
				prefs.searchMode = EM_Hex;
				DisableButton( g.searchWin, MatchCaseItem );	// LR 1.65
				goto setmode;

			case AsciiModeItem:
				prefs.searchMode = EM_Ascii;
				EnableButton( g.searchWin, MatchCaseItem );	// LR 1.65
setmode:
				SetControl( g.searchWin, HexModeItem, prefs.searchMode == EM_Hex );
				SetControl( g.searchWin, AsciiModeItem, prefs.searchMode == EM_Ascii );
				break;
			case MatchCaseItem:
				prefs.searchCase ^= 1;
				SetControl( g.searchWin, MatchCaseItem, prefs.searchCase );
				break;
			case SearchTextItem:
				break;
			}
		}
		else if( whichDlog == g.gotoWin )	// LR: v1.6.5 moved here to allow modeless dialog
		{
			switch( itemHit )
			{
			case GGotoAddr:
				{
					long		addr = -1;
					short		r;
					EditWindowPtr dWin;

					dWin = FindFirstEditWindow();
					if( dWin )
					{
						GetText( g.gotoWin, GAddrItem, g.gotoText );

						CopyPascalStringToC( g.gotoText, (char *)g.gotoText );

						if( prefs.gotoMode == EM_Hex )
							r = sscanf( (char *) g.gotoText, "%lx", &addr );
						else
							r = sscanf( (char *) g.gotoText, "%ld", &addr );

						CopyCStringToPascal( (char *) g.gotoText, g.gotoText );

						if( !r )	// LR: v1.6.5 show why values don't work
							ErrorAlert( ES_Caution, errHexValues );
						else if( addr >= 0 && addr < dWin->fileSize )
						{
							dWin->startSel = dWin->endSel = addr;
							SelectWindow( dWin->oWin.theWin );
							ScrollToSelection( dWin, addr, true, true );
						}
					}
				}
				break;
				
			case GHexItem:
				prefs.gotoMode = EM_Hex;
				goto setgmode;
				break;
				
			case GDecimalItem:
				prefs.gotoMode = EM_Decimal;
setgmode:
				SetControl( g.gotoWin, GHexItem, prefs.gotoMode == EM_Hex );
				SetControl( g.gotoWin, GDecimalItem, prefs.gotoMode == EM_Decimal );
				break;
			}
		}
	}
	else if( theEvent->what == updateEvt )	// LR: v1.6.5
	{
		if( whichDlog == g.searchWin )
			SetDialogDefaultItem( g.searchWin, SearchForwardItem );
		else if( whichDlog == g.gotoWin )
			SetDialogDefaultItem( g.gotoWin, GGotoAddr );
	}
}

/*** STRING TO SEARCH BUFFER ***/
Boolean StringToSearchBuffer( Boolean matchCase )
{
	Ptr		sp, dp;
	short	i;
	short	val;
	Boolean	loFlag;

	// Convert String to g.searchBuffer
	if( prefs.searchMode == EM_Hex )
	{
		sp = (Ptr) &g.searchText[1];
		dp = (Ptr) &g.searchBuffer[1];
		loFlag = false;
		for( i = 0; i < g.searchText[0]; ++i, ++sp )
		{
			if( *sp == '0' && *( sp+1 ) == 'x' )
			{
				loFlag = 0;
				++sp;
				++i;
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
		g.searchBuffer[0] = (long) dp - (long) &g.searchBuffer[1];
		if( g.searchBuffer[0] == 0 )
			goto HexError;
	}
	else
	{
		BlockMove( g.searchText, g.searchBuffer, g.searchText[0]+1 );
		if( !matchCase )
			UppercaseText( (Ptr) g.searchBuffer, g.searchText[0] + 1, smSystemScript );	// NS: function name changed and script constant added
	}
	return true;
	
HexError:
	ErrorAlert( ES_Caution, errHexValues );
	return false;
}
