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

#include "Prefs.h"
#include "Menus.h"
#include "EditWindow.h"
#include "EditRoutines.h"
#include "HexCompare.h"
#include "HexSearch.h"
#include "AboutBox.h"
#include "Utility.h"

// Menu Handles
static MenuRef appleMenu, fileMenu, editMenu, findMenu, optionsMenu, colorMenu, windowMenu;

// Externals
extern WindowRef CompWind1, CompWind2;

/*** INITALISE MENUBAR ***/
OSStatus InitMenubar( void )
{
	// set menu bar
	Handle	menuBar;

	menuBar = GetNewMBar( MenuBaseID );
	SetMenuBar( menuBar );

	// get menu references
	appleMenu	= GetMenuRef( kAppleMenu );
	fileMenu	= GetMenuRef( kFileMenu );
	editMenu	= GetMenuRef( kEditMenu );
	findMenu	= GetMenuRef( kFindMenu );
	optionsMenu	= GetMenuRef( kOptionsMenu );
	colorMenu	= GetMenuRef( kColorMenu );
	windowMenu	= GetMenuRef( kWindowMenu );

#if !TARGET_API_MAC_CARBON
	AppendResMenu( appleMenu, 'DRVR' );
#endif
	AppendResMenu( colorMenu, 'HEct' );	// LR: add color scheme menu

	DrawMenuBar();
	return noErr;
}

/*** SMART ENABLE MENU ITEM ***/
OSStatus SmartEnableMenuItem( MenuRef menu, short item, short ok )
{
//	Code to simplify enabling/disabling menu items.
	if( ok )
		EnableMenuItem( menu, item );
	else
		DisableMenuItem( menu, item );

	if( item == 0 )
		DrawMenuBar();

	return noErr;
}

/*** ADJUST MENUS ***/
OSStatus AdjustMenus( void )
{
	register		WindowRef theWin;
	short 			windowKind;
	Boolean 		isDA, isObjectWin, selection, scrapExists, undoExists, isGotoWin, isFindWin;
	EditWindowPtr	dWin = NULL;
	Str31			menuStr;
	short 			i;
	long			scrapSize;	// LR: v1.6.5
	Str255			frontWindowName, menuItemTitle;
	Boolean			namesMatch;

	theWin = FrontWindow();
	if( theWin )
	{
	#if TARGET_API_MAC_CARBON
		isGotoWin = (theWin == GetDialogWindow( g.gotoWin ));
		isFindWin = (theWin == GetDialogWindow( g.searchWin ));
	#else
		isGotoWin = (theWin == g.gotoWin);
		isFindWin = (theWin == g.searchWin);
	#endif

		windowKind = GetWindowKind( theWin );
		isDA = ( windowKind < 0 );
		isObjectWin = GetWindowKind( theWin ) == HexEditWindowID;
		selection = ( isObjectWin && dWin->endSel > dWin->startSel );
		if( isObjectWin )
			dWin = (EditWindowPtr)GetWRefCon( theWin );	//LR: 1.66 - don't set unless an edit window!
	}
	else	// LR: v1.6.5 if no window is visible, then nothing is true!
	{
		isGotoWin = isFindWin = isObjectWin = isDA = selection = 0;
	}

	// LR: v1.6.5 - rewrite of scrap check
	if( isObjectWin || isFindWin || isGotoWin )
	{
#if TARGET_API_MAC_CARBON
		ScrapFlavorFlags flavorFlags;
		ScrapRef scrapRef;
		OSErr anErr;

		anErr = GetCurrentScrap( &scrapRef );
		if( !anErr )
			anErr = GetScrapFlavorFlags( scrapRef, kScrapFlavorTypeText, &flavorFlags );		// non-blocking check for scrap data
		if( !anErr )
			anErr = GetScrapFlavorSize( scrapRef, kScrapFlavorTypeText, &scrapSize );	// blocking call to get size
#else
		long offset;

		scrapSize = GetScrap( NULL, 'TEXT', &offset );
#endif
		scrapExists = scrapSize > 0;
	}
	else
		scrapExists = false;

	undoExists = (gUndo.type != 0);	// check for NULL gUndo!
	
// LR: - enable file menu items during search, via Aaron D.
// LR:	SmartEnableMenuItem( fileMenu, FM_New, g.searchWin == NULL );
// LR:	SmartEnableMenuItem( fileMenu, FM_Open, g.searchWin == NULL );

// LR: 1.65 moved print names to string for localization
	GetIndString( menuStr, strPrint, (isObjectWin && dWin->startSel < dWin->endSel) ? 2 : 1 );

	SetMenuItemText( fileMenu, FM_Print, menuStr );

// bug: 1.65 NO PRINTING YET! (so disable menus for now!)
#if TARGET_API_MAC_CARBON
	SmartEnableMenuItem( fileMenu, FM_PageSetup, false );
	SmartEnableMenuItem( fileMenu, FM_Print, false );
#else
	SmartEnableMenuItem( fileMenu, FM_Print, isObjectWin );
#endif

	SmartEnableMenuItem( fileMenu, FM_OtherFork, isObjectWin );
	SmartEnableMenuItem( fileMenu, FM_Close, isDA || isObjectWin || isFindWin || isGotoWin );	// LR: v1.6.5 rewrite via Max Horn
	SmartEnableMenuItem( fileMenu, FM_Save, isObjectWin && dWin->dirtyFlag );
	SmartEnableMenuItem( fileMenu, FM_SaveAs, isObjectWin );
	SmartEnableMenuItem( fileMenu, FM_Revert, isObjectWin && dWin->refNum && dWin->dirtyFlag );

	SmartEnableMenuItem( editMenu, 0, theWin != NULL );
	SmartEnableMenuItem( editMenu, EM_Undo, isDA || undoExists );
	SmartEnableMenuItem( editMenu, EM_Cut, isDA || selection );
	SmartEnableMenuItem( editMenu, EM_Copy, isDA || selection );
	SmartEnableMenuItem( editMenu, EM_Paste, isDA || scrapExists );
	SmartEnableMenuItem( editMenu, EM_Clear, isDA || selection );

	SmartEnableMenuItem( editMenu, EM_SelectAll, isDA || isObjectWin );

	SmartEnableMenuItem( findMenu, 0, isObjectWin || isFindWin || isGotoWin );
/* 1.65
	SmartEnableMenuItem( findMenu, SM_Find, isObjectWin );
	SmartEnableMenuItem( findMenu, SM_FindForward, isObjectWin );
	SmartEnableMenuItem( findMenu, SM_FindBackward, isObjectWin );
	SmartEnableMenuItem( findMenu, SM_GotoAddress, isObjectWin );
*/
	CheckMenuItem( optionsMenu, OM_HiAscii, prefs.asciiMode );
	CheckMenuItem( optionsMenu, OM_DecimalAddr, prefs.decimalAddr );
	CheckMenuItem( optionsMenu, OM_Backups, prefs.backupFlag );
	CheckMenuItem( optionsMenu, OM_Overwrite, prefs.overwrite );
	CheckMenuItem( optionsMenu, OM_VertBars, prefs.vertBars );

	// LR: v1.6.5 Lots of re-writing on handling the color scheme menu
#if !TARGET_API_MAC_CARBON
	// no color usage if not displayable!
	if( !g.colorQDFlag )
		prefs.useColor = false;
#endif

// LR: v1.6.5	CheckMenuItem( gColorMenu, CM_UseColor, prefs.useColor );	// allow turning on even if not usable
// LR: v1.6.5 Try to show status of color in new windows to help "intuitive" nature of menu
	GetIndString( menuStr, strColor, (prefs.useColor) ? 2 : 1 );

	SetMenuItemText( colorMenu, CM_UseColor, menuStr );

	selection = prefs.useColor && isObjectWin && dWin->csResID > 0;
	i = CountMenuItems( colorMenu );
	do
	{
		SmartEnableMenuItem( colorMenu, i, selection );	// LR: v1.6.5 only enable for color windows
	} while( --i > 2 );
	
	// NS: v1.6.6 checkmark front window in window menu
	if( theWin )
		GetWTitle( theWin, frontWindowName );	//LR: 1.66 - don't use NULL window!
	else
		frontWindowName[0] = 0;

	i = CountMenuItems( windowMenu );
	while( i )
	{
		GetMenuItemText( windowMenu, i, menuItemTitle );		// if you open more than one file with the same name (or edit the other fork)É
		namesMatch = EqualPStrings( frontWindowName, menuItemTitle );	// Éyou will have multiple items in the menu with the same text, and all will be checkmarked
		CheckMenuItem( windowMenu, i, namesMatch );

		i--;
	}

	return( noErr );
}

/* --------------------
| GetColorMenuResID -- return resID of a color menu item
|						( also, marks the menu item passed in as checked )
|	ENTRY:	menu item
|	 EXIT:	resID
*/

/*** GET COLOUR MENU RES ID ***/
short GetColorMenuResID( short menuItem )
{
	Handle	h;
	Str255	menuText;
	short	resID;
	ResType	resType;

	CheckMenuItem( colorMenu, menuItem, true );
	GetMenuItemText( colorMenu, menuItem, menuText );
	h = GetNamedResource( 'HEct', menuText );
	if( h )
	{
		GetResInfo( h, &resID, &resType, menuText );
		return resID;
	}
	else return CM_StartingResourceID;
}

// 	Handle the menu selection. mSelect is what MenuSelect() and
// 	MenuKey() return: the high word is the menu ID, the low word
// 	is the menu item

/*** HANDLE MENU ***/
// LR 1.66 -- complete rewrite (basically) of this entire routine...it was UGLY!
OSStatus HandleMenu( long mSelect )
{
	short			menuID = HiWord( mSelect );
	short			menuItem = LoWord( mSelect );
	WindowRef		frontWindow;
	DialogPtr		dlgRef = NULL;
	EditWindowPtr	dWin = NULL;

	Str255			currentWindowName, newFrontWindowName;		// NS: v1.6.6, for window menu
	WindowRef		currentWindow;								// NS:			this too
	
	// Predetermine what type of window we have to work with
	frontWindow = FrontWindow();
	if( frontWindow )
	{
		if( HexEditWindowID == GetWindowKind( frontWindow ) )
			dWin = (EditWindowPtr) GetWRefCon( frontWindow );
		else if( g.gotoWin == (DialogPtr)frontWindow || g.searchWin == (DialogPtr)frontWindow )
			dlgRef = (DialogPtr)frontWindow;
	}

	switch( menuID )
	{
		case kAppleMenu:
			if( menuItem == AM_About )
				HexEditAboutBox();
#if !TARGET_API_MAC_CARBON
			else
			{
				GrafPtr savePort;
				Str255 name;

				GetPort( &savePort );
				GetMenuItemText( appleMenu, menuItem, name );
				OpenDeskAcc( name );
				SetPort( savePort );
			}
#endif
			break;
		
	case kFileMenu:
		switch( menuItem )
		{
		case FM_New:
			NewEditWindow();
			break;

		case FM_Open:
			AskEditWindow();
			break;

		case FM_OtherFork:	// LR: I want to see both!
			if( dWin )
			{
				short fork;
				EditWindowPtr ewin;

				if( dWin->fork == FT_Data )
					fork = FT_Resource;
				else
					fork = FT_Data;

				if( (Boolean) (ewin = LocateEditWindow( &dWin->fsSpec, fork )) )
				{
					SelectWindow( ewin->oWin.theWin );	// just select existing theWin
				}
				else	// try to open other fork in new theWin!
				{
					g.forkMode = fork;
					OpenEditWindow( &dWin->fsSpec, true );
				}
			}
			break;

		case FM_CompareFiles:		// LR: file compares
			if( GetCompareFiles() )
				DoComparison();
			break;

		//LR: 1.66 - NOTE: dWin == NULL == frontWindow!
		case FM_Save:
			if( dWin && dWin->oWin.Save )
				dWin->oWin.Save( frontWindow );
			break;

		case FM_SaveAs:
			if( dWin && dWin->oWin.SaveAs )
				dWin->oWin.SaveAs( frontWindow );
			break;

		case FM_Revert:
			if( dWin && dWin->oWin.Revert )
				dWin->oWin.Revert( frontWindow );
			break;

		case FM_Close:
			if( dWin )
				CloseEditWindow( frontWindow );
			else if( dlgRef )
			{
#if TARGET_API_MAC_CARBON
				HideWindow( GetDialogWindow( dlgRef ) );
#else
				HideWindow( dlgRef );
#endif
			}
			break;

		case FM_Quit:
			if( CloseAllEditWindows() )
				g.quitFlag = true;
			break;

		case FM_PageSetup:
#if TARGET_API_MAC_CARBON
// bug: printing
#else
			PrOpen();
			PrStlDialog( g.HPrint );
			PrClose();
#endif
			break;

		case FM_Print:
			if( dWin )
				PrintWindow( dWin );
			break;
		}
		break;

	case kEditMenu:
#if !TARGET_API_MAC_CARBON
		if( !SystemEdit( menuItem -1 ) )
		{
#endif
			if( dWin ) switch( menuItem ) 
			{
				case EM_Undo:
					UndoOperation();
					break;

				case EM_Cut:
					CutSelection( dWin );				
					break;

				case EM_Copy:
					CopySelection( dWin );	
					break;

				case EM_Paste:
					PasteSelection( dWin );
					break;

				case EM_Clear:
					ClearSelection( dWin );			
					break;

				case EM_SelectAll:
					dWin->startSel = 0;
					dWin->endSel = dWin->fileSize;
					UpdateOnscreen( dWin->oWin.theWin );
					break;
			}
			else if( dlgRef ) switch( menuItem )
			{
				case EM_Cut:
					DialogCut( dlgRef );
					break;

				case EM_Copy:
					DialogCopy( dlgRef );
					break;

				case EM_Paste:
					DialogPaste( dlgRef );
					break;

				case EM_Clear:
					break;

				case EM_SelectAll:
					break;
			}

#if !TARGET_API_MAC_CARBON
		}
#endif
		break;

	case kFindMenu:
		switch ( menuItem )
		{
			case SM_Find:
				OpenSearchDialog();
				break;

			case SM_FindForward:
				if( dWin )
				{
					prefs.searchForward = true;
					PerformTextSearch( dWin );
				}
				break;

			case SM_FindBackward:
				if( dWin )
				{
					prefs.searchForward = false;
					PerformTextSearch( dWin );
				}
				break;

			case SM_GotoAddress:
				OpenGotoAddress();
				break;
		}
		break;

	case kOptionsMenu:
		switch ( menuItem )
		{
			case OM_HiAscii:
				prefs.asciiMode = !prefs.asciiMode;
				if( prefs.asciiMode )	g.highChar = 0xFF;
				else					g.highChar = 0x7F;
				UpdateEditWindows();
				break;

			case OM_DecimalAddr:
				prefs.decimalAddr = !prefs.decimalAddr;
				UpdateEditWindows();
				break;

			case OM_Backups:
				prefs.backupFlag = !prefs.backupFlag;
				break;

			case OM_Overwrite:
				prefs.overwrite = !prefs.overwrite;
				break;

			case OM_VertBars:
				prefs.vertBars = !prefs.vertBars;
				UpdateEditWindows();
				break;

			case OM_ComparePref:	// LR: compare options
				ComparisonPreferences();
				break;
		}
		break;

	// LR: Add color scheme menu
	case kColorMenu:
		if( menuItem == CM_UseColor )
			prefs.useColor = !prefs.useColor;
		else if( dWin && dWin->csResID > 0 )		// can't color B&W windows!
		{
			CheckMenuItem( colorMenu, prefs.csMenuID, false );

			prefs.csResID = GetColorMenuResID( menuItem );
			prefs.csMenuID = menuItem;
			
			if( GetWindowKind( dWin->oWin.theWin ) == HexEditWindowID )
				dWin->csResID = prefs.csResID;
		}
		UpdateEditWindows();
		break;
	
	case kWindowMenu:
		GetMenuItemText( windowMenu, menuItem, newFrontWindowName );
		currentWindow = GetWindowList();
		GetWTitle( currentWindow, currentWindowName );
		while( !EqualPStrings( currentWindowName, newFrontWindowName ) )
		{
			currentWindow = GetNextWindow( currentWindow );
			GetWTitle( currentWindow, currentWindowName );
		}
		SelectWindow( currentWindow );
		break;
	}

	HiliteMenu( 0 );
	AdjustMenus();

	return( noErr );
}
