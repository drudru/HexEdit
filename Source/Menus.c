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
 * Copyright (C) Copyright © 1996-2002.
 * All Rights Reserved.
 *
 * Modified: $Date$
 * Revision: $Id$
 *
 * Contributor(s):
 *		Lane Roathe
 *		Nick Shanks (NS)
 *		Scott E. Lasley (SEL) 
 */

// 05/10/01 - GAB: MPW environment support
#ifdef __MPW__
#include "MPWIncludes.h"
#endif

#include "Prefs.h"
#include "Menus.h"
#include "EditWindow.h"
#include "EditRoutines.h"
#include "HexCompare.h"
#include "HexSearch.h"
#include "AboutBox.h"
#include "Utility.h"

static _cmCheckedItem = 0;

// Menu Handles
static MenuRef appleMenu, fileMenu, editMenu, findMenu, optionsMenu, colorMenu, windowMenu;

// Externals
extern WindowRef CompWind1, CompWind2;

/*** SMART ENABLE MENU ITEM ***/
static OSStatus _enableMenuItem( MenuRef menu, short item, short ok )
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

#if TARGET_API_MAC_CARBON // SEL: 1.7 - carbon session based printing

/*------------------------------------------------------------------------------
    Allow the user define the page setup for printing to current printer

    Parameters:
        printSession    -   current printing session
        pageFormat      -   a PageFormat object addr

    Description:
        If the caller passes in an empty PageFormat object, create a new one,
        otherwise validate the one provided by the caller.
        Invokes the Page Setup dialog and checks for Cancel.
        Flattens the PageFormat object so it can be saved with the document.
        Note that the PageFormat object is modified by this function.

------------------------------------------------------------------------------*/
static void _doPageSetupDialog(PMPageFormat* pageFormat)
{
	OSStatus    		status;
	Boolean     		accepted;
	PMPrintSession	printSession;

	status = PMCreateSession(&printSession);
	if ( noErr != status )
	{
		PostPrintingErrors(status);
		return;
	}

	//  Set up a valid PageFormat object.
	if (*pageFormat == kPMNoPageFormat)
	{
	  status = PMCreatePageFormat(pageFormat);

	  //  Note that PMPageFormat is not session-specific, but calling
	  //  PMSessionDefaultPageFormat assigns values specific to the printer
	  //  associated with the current printing session.
		if ((status == noErr) && (*pageFormat != kPMNoPageFormat))
			status = PMSessionDefaultPageFormat(printSession, *pageFormat);
	}
	else
	{
		status = PMSessionValidatePageFormat(printSession, *pageFormat, kPMDontWantBoolean);
	}

	//  Display the Page Setup dialog.
	if (status == noErr)
	{
		status = PMSessionPageSetupDialog(printSession, *pageFormat, &accepted);
		if (!accepted)
		{
			status = kPMCancel; // user clicked Cancel button
		}
	}

	status = PMRelease(printSession);
	return;
}

/*------------------------------------------------------------------------------
    Display error alert to end user

    Parameters:
        status  -   error code

    Description:
        This is where we post an alert to report any problem encountered by the Printing Manager.

------------------------------------------------------------------------------*/
void PostPrintingErrors( OSStatus status )
{
	ErrorAlert(ES_Caution, status == kPMNoDefaultPrinter ? errDefaultPrinter : errGenericPrinting, "Error = %d", status );
}

#endif


/* --------------------
 Return resID of a color menu item
	( also, marks the menu item passed in as checked )

	ENTRY:	menu item
	 EXIT:	resID
*/

short GetColorMenuResID( short menuItem )
{
	Handle	h;
	Str255	menuText;
	short	resID;
	ResType	resType;

	GetMenuItemText( colorMenu, menuItem, menuText );
	h = GetNamedResource( 'HEct', menuText );
	if( h )
	{
		GetResInfo( h, &resID, &resType, menuText );
		return resID;
	}
	else
		return( CM_StartingResourceID );
}

/* --------------------
	Return the ID of the window menu item with the passed title

	ENTRY:	ptr to pascal string to match
	 EXIT:	item ID, or 0 if not found
*/

short GetWindowMenuItemID( StringPtr title )
{
	short i;
	Str255 menuItemTitle;

	i = CountMenuItems( windowMenu );
	while( i )
	{
		GetMenuItemText( windowMenu, i, menuItemTitle );		// if you open more than one file with the same name (or edit the other fork)É
		if( EqualPStrings( title, menuItemTitle ) )
			break;
		i--;
	}
	return( i );
}


/*** INITALISE MENUBAR ***/
OSStatus InitMenubar( void )
{
	// set menu bar
	Handle	menuBar;

#if TARGET_API_MAC_CARBON
	long result;
	if ((Gestalt(gestaltMenuMgrAttr, &result) == noErr) && (result & gestaltMenuMgrAquaLayoutMask))
		menuBar = GetNewMBar( kMenuXBaseID);
	else
#endif
		menuBar = GetNewMBar( kMenuBaseID );
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

	theWin = FrontNonFloatingWindow();
	if( theWin )
	{
		isGotoWin = (g.gotoDlg && theWin == GetDialogWindow( g.gotoDlg ));		//LR: 1.7 - don't get window info on NULL!
		isFindWin = (g.searchDlg && theWin == GetDialogWindow( g.searchDlg ));

		windowKind = GetWindowKind( theWin );
		isDA = ( windowKind < 0 );
		isObjectWin = GetWindowKind( theWin ) == kHexEditWindowTag;
		if( isObjectWin )
		{
			dWin = (EditWindowPtr)GetWRefCon( theWin );	//LR: 1.66 - don't set unless an edit window!
			selection = dWin->endSel > dWin->startSel;
		}
		else
		{
			selection = (isGotoWin || isFindWin);
		}
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
			anErr = GetScrapFlavorFlags( scrapRef, kScrapFlavorTypeText, &flavorFlags );	// non-blocking check for scrap data
		if( !anErr )
			anErr = GetScrapFlavorSize( scrapRef, kScrapFlavorTypeText, &scrapSize );		// blocking call to get size
#else
		long offset;

		scrapSize = GetScrap( NULL, 'TEXT', &offset );
#endif
		scrapExists = scrapSize > 0;
	}
	else
		scrapExists = false;

	undoExists = (isObjectWin && gUndo.type != 0 && gUndo.theWin == dWin);	// check for NULL gUndo!
	
// LR: - enable file menu items during search, via Aaron D.
// LR:	_enableMenuItem( fileMenu, FM_New, g.searchDlg == NULL );
// LR:	_enableMenuItem( fileMenu, FM_Open, g.searchDlg == NULL );

// LR: 1.65 moved print names to string for localization
	GetIndString( menuStr, strPrint, (isObjectWin && dWin->startSel < dWin->endSel) ? 2 : 1 );

	SetMenuItemText( fileMenu, FM_Print, menuStr );

//LR 188 -- page setup should always be enabled
//	_enableMenuItem( fileMenu, FM_PageSetup, isObjectWin );	//SEL: 1.7 - enabled for carbon
	_enableMenuItem( fileMenu, FM_Print, isObjectWin );

	_enableMenuItem( fileMenu, FM_OtherFork, isObjectWin );
	_enableMenuItem( fileMenu, FM_Close, isDA || isObjectWin || isFindWin || isGotoWin );	// LR: v1.6.5 rewrite via Max Horn
	_enableMenuItem( fileMenu, FM_Save, isObjectWin && dWin->dirtyFlag );
	_enableMenuItem( fileMenu, FM_SaveAs, isObjectWin );
	_enableMenuItem( fileMenu, FM_Revert, isObjectWin && dWin->refNum && dWin->dirtyFlag );

	_enableMenuItem( editMenu, 0, theWin != NULL );
	_enableMenuItem( editMenu, EM_Undo, isDA || undoExists );
	_enableMenuItem( editMenu, EM_Cut,  isDA || (selection && (!gPrefs.overwrite || (gPrefs.overwrite && !gPrefs.nonDestructive))) );
	_enableMenuItem( editMenu, EM_Copy, isDA || selection );
	_enableMenuItem( editMenu, EM_Paste, isDA || scrapExists );
	_enableMenuItem( editMenu, EM_Clear, isDA || selection );

	_enableMenuItem( editMenu, EM_SelectAll, isDA || isObjectWin || isFindWin || isGotoWin );

	_enableMenuItem( findMenu, 0, isObjectWin || isFindWin || isGotoWin );
/* 1.65
	_enableMenuItem( findMenu, SM_Find, isObjectWin );
	_enableMenuItem( findMenu, SM_GotoAddress, isObjectWin );
*/
	_enableMenuItem( findMenu, SM_FindForward, isObjectWin && dWin->fileSize && g.searchBuffer[0] );	//LR 1.72 -- only enable w/something to search :)
	_enableMenuItem( findMenu, SM_FindBackward, isObjectWin && dWin->fileSize && g.searchBuffer[0] );

	_enableMenuItem( optionsMenu, OM_NonDestructive, gPrefs.overwrite );	//LR 1.74 -- only available in overwrite mode

	CheckMenuItem( optionsMenu, OM_HiAscii, gPrefs.asciiMode );
	CheckMenuItem( optionsMenu, OM_DecimalAddr, gPrefs.decimalAddr );
	CheckMenuItem( optionsMenu, OM_Backups, gPrefs.backupFlag );
	CheckMenuItem( optionsMenu, OM_WinSize, gPrefs.constrainSize );
	CheckMenuItem( optionsMenu, OM_Overwrite, gPrefs.overwrite );
	CheckMenuItem( optionsMenu, OM_NonDestructive, !gPrefs.nonDestructive );	//LR 190 -- REVERSE (updated text, not code)
	CheckMenuItem( optionsMenu, OM_MoveOnlyPaging, gPrefs.moveOnlyPaging );	//LR 180 -- optional move only paging
	CheckMenuItem( optionsMenu, OM_Unformatted, !gPrefs.formatCopies );
	CheckMenuItem( optionsMenu, OM_VertBars, gPrefs.vertBars );

	// LR: v1.6.5 Lots of re-writing on handling the color scheme menu
#if !TARGET_API_MAC_CARBON
	// no color usage if not displayable!
	if( !g.colorQDFlag )
		gPrefs.useColor = false;
#endif

// LR: v1.6.5	CheckMenuItem( gColorMenu, CM_UseColor, gPrefs.useColor );	// allow turning on even if not usable
// LR: v1.6.5 Try to show status of color in new windows to help "intuitive" nature of menu
	GetIndString( menuStr, strColor, (gPrefs.useColor) ? 2 : 1 );
	SetMenuItemText( colorMenu, CM_UseColor, menuStr );

	//LR 181 -- show the current window color (or default if no windows)
	if( _cmCheckedItem )
		CheckMenuItem( colorMenu, _cmCheckedItem, false );
	_cmCheckedItem = isObjectWin ? dWin->csMenuID : gPrefs.csMenuID;
	CheckMenuItem( colorMenu, _cmCheckedItem, true );

	selection = gPrefs.useColor;	//LR 190 -- && isObjectWin && dWin->csResID > 0;
	i = CountMenuItems( colorMenu );
	do
	{
		_enableMenuItem( colorMenu, i, selection );	// LR: v1.6.5 only enable for color windows
	} while( --i > 2 );
	
	// NS: v1.6.6 checkmark front window in window menu
	if( theWin )
		GetWTitle( theWin, frontWindowName );	//LR: 1.66 - don't use NULL window!
	else
		frontWindowName[0] = 0;

	i = CountMenuItems( windowMenu );
	_enableMenuItem(windowMenu, 0, i != 0);	// dim out Window menu if no windows up.
	while( i )
	{
		GetMenuItemText( windowMenu, i, menuItemTitle );		// if you open more than one file with the same name (or edit the other fork)É
		namesMatch = EqualPStrings( frontWindowName, menuItemTitle );	// Éyou will have multiple items in the menu with the same text, and all will be checkmarked
		CheckMenuItem( windowMenu, i, namesMatch );

		i--;
	}

	return( noErr );
}

// 	Handle the menu selection. mSelect is what MenuSelect() and
// 	MenuKey() return: the high word is the menu ID, the low word
// 	is the menu item

/*** HANDLE MENU ***/
// LR 1.66 -- complete rewrite (basically) of this entire routine...it was UGLY!
OSStatus HandleMenu( long mSelect, short modifiers )
{
	short			menuID = HiWord( mSelect );
	short			menuItem = LoWord( mSelect );
	short 		colorResID;
	WindowRef		frontWindow;
	DialogPtr		dlgRef = NULL;
	EditWindowPtr	dWin = NULL;

	Str255			currentWindowName, newFrontWindowName;		// NS: v1.6.6, for window menu
	WindowRef		currentWindow;								// NS:			this too
	
	// Predetermine what type of window we have to work with
	frontWindow = FrontNonFloatingWindow();
	if( frontWindow )
	{
		DialogPtr dlg = GetDialogFromWindow( frontWindow );

		if( kHexEditWindowTag == GetWindowKind( frontWindow ) )
			dWin = (EditWindowPtr) GetWRefCon( frontWindow );
		else if( g.gotoDlg == dlg || g.searchDlg == dlg )
			dlgRef = dlg;
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
			AskEditWindow( kWindowNormal );
			break;

		case FM_OtherFork:	// LR: I want to see both!
			if( dWin )
			{
				short fork;
//LR 180				EditWindowPtr ewin;

				if( dWin->fork == FT_Data )
					fork = FT_Resource;
				else
					fork = FT_Data;

/*LR 180 -- OpenEditWindow checks for this
				if( NULL != (ewin = LocateEditWindow( &dWin->fsSpec, fork )) )	// LR: 1.7 - boolean typecast causes failure!
				{
					SelectWindow( ewin->oWin.theWin );	// just select existing theWin
				}
				else	// try to open other fork in new theWin!
*/				{
					g.forkMode = fork;
					OpenEditWindow( &dWin->fsSpec, kWindowNormal, true );
				}
			}
			break;

		case FM_CompareFiles:		//LR 180 -- now pass in modifiers to allow select override
			if( GetCompareFiles( modifiers ) )
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
			if( dWin && dWin->oWin.Revert )	//LR 1.72 -- check before reverting (could be dangerous!)
			{
				ParamText( dWin->fsSpec.name, NULL, NULL, NULL );
				switch( CautionAlert( alertRevert, NULL ) )
				{
					case ok:
						dWin->oWin.Revert( frontWindow );
						break;
				}
			}
			break;

		case FM_Close:
			if( dWin )
				CloseEditWindow( frontWindow );
			else if( dlgRef )
			{
				HideWindow( frontWindow );	//LR: 1.7 -- no need.GetDialogWindow( dlgRef ) );
			}
			break;

		case FM_Quit:
			if( CloseAllEditWindows() )
				g.quitFlag = true;
			break;

		case FM_PageSetup:
#if TARGET_API_MAC_CARBON  // sel - carbon session based printing
			_doPageSetupDialog(&g.pageFormat);
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
#endif
		{
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
					TEToScrap();
					break;

				case EM_Copy:
					DialogCopy( dlgRef );
					TEToScrap();
					break;

				case EM_Paste:
					TEFromScrap();
					DialogPaste( dlgRef );
					break;

				case EM_Clear:
					DialogDelete( dlgRef );
					break;

				case EM_SelectAll:
					break;
			}
		}
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
					gPrefs.searchForward = true;
					PerformTextSearch( dWin, kSearchUpdateUI );
				}
				break;

			case SM_FindBackward:
				if( dWin )
				{
					gPrefs.searchForward = false;
					PerformTextSearch( dWin, kSearchUpdateUI );
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
				gPrefs.asciiMode = !gPrefs.asciiMode;
				if( gPrefs.asciiMode )	g.highChar = 0xFF;
				else					g.highChar = 0x7F;
				UpdateEditWindows();
				break;

			case OM_DecimalAddr:
				gPrefs.decimalAddr = !gPrefs.decimalAddr;
				UpdateEditWindows();
				break;

			case OM_Backups:
				gPrefs.backupFlag = !gPrefs.backupFlag;
				break;

			case OM_WinSize:
				gPrefs.constrainSize = !gPrefs.constrainSize;
				break;

			case OM_Overwrite:
				gPrefs.overwrite = !gPrefs.overwrite;
				break;

			case OM_NonDestructive:
				gPrefs.nonDestructive = !gPrefs.nonDestructive;
				break;

			case OM_MoveOnlyPaging:
				gPrefs.moveOnlyPaging = !gPrefs.moveOnlyPaging;
				break;

			case OM_Unformatted:
				gPrefs.formatCopies = !gPrefs.formatCopies;
				break;

			case OM_VertBars:
				gPrefs.vertBars = !gPrefs.vertBars;
				UpdateEditWindows();
				break;

			case OM_ComparePref:	// LR: compare options
				ComparisonPreferences();
				break;
		}
		break;

	// LR: Add color scheme menu
	case kColorMenu:
		colorResID = GetColorMenuResID( menuItem );

		if( menuItem == CM_UseColor )
		{
			gPrefs.useColor = !gPrefs.useColor;		// toggle color usage
		}
		else if( dWin && dWin->csResID > 0 )		// can't color B&W windows!
		{
			if( _cmCheckedItem )
				CheckMenuItem( colorMenu, _cmCheckedItem, false );

			if( (modifiers & optionKey) )	// option down == change all windows (set default color)
			{
				EditWindowPtr eWin = FindFirstEditWindow();

				while( eWin )
				{
					if( GetWindowKind( eWin->oWin.theWin ) == kHexEditWindowTag )
					{
						eWin->csResID = colorResID;
						eWin->csMenuID = menuItem;	//LR 181 -- for menu tagging
					}

					eWin = FindNextEditWindow( eWin );
				}
				goto savepref;
			}
			else	//LR 181 -- default is (back) to changing color of a single window!
			{
				if( GetWindowKind( dWin->oWin.theWin ) == kHexEditWindowTag )
				{
					dWin->csResID = colorResID;
					dWin->csMenuID = menuItem;	//LR 181 -- for menu tagging
				}
			}
		}
		else
		{
savepref:	//LR 190 -- no window open == set preferred color
			gPrefs.csResID = colorResID;	//LR 180 -- save prefs when changing all
			gPrefs.csMenuID = menuItem;
		}

		UpdateEditWindows();
		break;

	// LR : 1.7 - rewrite with bug checking (could crash accessing NULL window)
	case kWindowMenu:
		GetMenuItemText( windowMenu, menuItem, newFrontWindowName );
		currentWindow = FrontNonFloatingWindow();
		while( currentWindow )
		{
			GetWTitle( currentWindow, currentWindowName );
			if( EqualPStrings( currentWindowName, newFrontWindowName ) )
			{
				SelectWindow( currentWindow );
				break;
			}
			currentWindow = GetNextWindow( currentWindow );
		}
		break;
	}

	HiliteMenu( 0 );
	AdjustMenus();

	return( noErr );
}
