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
 *		Brian Bergstrand (BB) 
 *		Nick Pissaro Jr. (NP)
 */

// 05/10/01 - GAB: MPW environment support
#ifdef __MPW__
#include "MPWIncludes.h"
#endif

#include <stdio.h>
#include <stdlib.h> // BB: bring in abs()

#include "EditWindow.h"
#include "EditRoutines.h"
#include "EditScrollbar.h"
#include "HexSearch.h"
#include "HexCompare.h"
#include "Menus.h"
#include "Prefs.h"
#include "Utility.h"

// Create a new main theWin using a 'WIND' template from the resource fork

//LR 177 SInt16			CompareFlag = 0;

HEColorTableHandle ctHdl = NULL;	// LR: global to file, for speed

RGBColor black = { 0, 0, 0 };
RGBColor white = { 0xFFFF, 0xFFFF, 0xFFFF };

// BB: selector to determine Nav dialog type
#define kNavOpenDialogType ((NavCallBackUserData)-1L)
enum
{
	kNavDITLID = 15000,
	kNavDITLHeight = 40,//18,
	kNavDITLWidth = 350,//234
	kDataForkRadioID = 2,
	kRsrcForkRadioID = 3,
	kAutoForkRadioID = 4,
	kNavDITLNumControls = 4//3 radio buttons + radio group control
};

/*** NEW OFFSCREEN GWORLD ***/
// LR: create offscreen drawing surface (so drawing to screen is not flickery)
//LR 180 -- removed old comments, now always creates offscreen at current color depth

static GWorldPtr _newCOffScreen( short width, short height )
{
	OSStatus	error;
	GWorldPtr	theGWorld;
	Rect		rect;
	
	SetRect( &rect, 0, 0, width, height );
	error = NewGWorld( &theGWorld, /*LR 177gPrefs.useColor? 0:1*/ 0, &rect, NULL, NULL, keepLocal );

	if( error != noErr ) return NULL;
	return theGWorld;
}

/*** ENSURE NAME IS UNIQUE ***/	
// LR: complete re-write as this function used to create bogus names and overwrite memory
// LR: 1.66 -- another rewrite, to create more "readable" names
static void _ensureNameIsUnique( FSSpec *tSpec )
{
	OSStatus err;
	FInfo fInfo;
	int i = tSpec->name[0], num = 1;

	// Weird compiler bug, having the OS call in the while loop can caues it to fail w/o error
	do
	{
		err = HGetFInfo( tSpec->vRefNum, tSpec->parID, tSpec->name, &fInfo );
		if( err != fnfErr )
		{
			Str31 numstr;

			NumToString( num++, numstr );	// get string equiv

			if( i > 30 - numstr[0] )
				i = 30 - numstr[0];		// don't make too long of a name

			tSpec->name[i + 1] = ' ';
			BlockMoveData( numstr + 1, &tSpec->name[i + 2], numstr[0] );

			tSpec->name[0] = i + 1 + numstr[0];
		}
	}while( err != fnfErr && i );
}

/*** Set the window title (making sure it's good ***/	
static void _setWindowTitle( EditWindowPtr dWin )
{
	Str255 wintitle;		// NOTE: static so we can pass back pointer (ie, it's not on stack!)

	int i,j,l;

	// LR: 1.66 make sure the name is good (for instance "icon/r" is bad!)
	l = (int)dWin->fsSpec.name[0];

	//LR 181 -- change first char if bad for menus
	if( '(' == dWin->fsSpec.name[0] )
		dWin->fsSpec.name[0] = '{';		// don't disable item

	if( '<' == dWin->fsSpec.name[0] )
		dWin->fsSpec.name[0] = '²';		// don't disable menu

	if( '/' == dWin->fsSpec.name[0] )
		dWin->fsSpec.name[0] = '|';		// don't create shortcut

	if( '-' == dWin->fsSpec.name[0] )
		dWin->fsSpec.name[0] = 'Ñ';		// don't create seperator line

	if( ';' == dWin->fsSpec.name[0] )
		dWin->fsSpec.name[0] = ':';		// not sure what it does, but ; is bad!

	//LR 1.72 -- don't copy bad chars (can cause menu to mess up!)
	for( i = 1, j= 1; i <= l; i++ )
	{
		if( dWin->fsSpec.name[i] >= ' ' && '!' != dWin->fsSpec.name[i] && '^' != dWin->fsSpec.name[i] )
			wintitle[j++] = dWin->fsSpec.name[i];
	}

	// LR: 1.7 Append fork in use to title
	if( j < 255 - 8 )
	{
		Str31 str2;

		GetIndString( (StringPtr) str2, strHeader, dWin->fork );

		BlockMoveData( &str2[1], &wintitle[j], str2[0] );

		wintitle[0] = j + str2[0] - 1;
	}

	SetWTitle( dWin->oWin.theWin, wintitle );

	// NS: 1.6.6 add window to window menu
	MacAppendMenu( GetMenuHandle(kWindowMenu), wintitle );
}

/*** SETUP NEW EDIT WINDOW ***/
//LR: 1.7 - static, and remove title (always fsSpec->name)
//LR 177 -- Accept window type instead of checking global var

static OSStatus _setupNewEditWindow( EditWindowPtr dWin, tWindowType type )
{
	WindowRef theWin;
	ObjectWindowPtr objectWindow;
	Rect r;

	// NS 1.7.1; check for appearance and create appropriate window
	theWin = InitObjectWindow( (g.useAppearance && g.systemVersion >= kMacOSEight) ? kAppearanceWindow : kSystem7Window, (ObjectWindowPtr) dWin, false );
	if( !theWin )
		ErrorAlert( ES_Fatal, errMemory );

	// LR: 1.7 set window title & get text for window menu
	_setWindowTitle( dWin );

	objectWindow = (ObjectWindowPtr)dWin;
	
	objectWindow->Draw			= MyDraw;
	objectWindow->Idle			= MyIdle;
	objectWindow->HandleClick	= MyHandleClick;
	objectWindow->Dispose		= DisposeEditWindow;
	objectWindow->ProcessKey	= MyProcessKey;
	objectWindow->Save			= SaveContents;
	objectWindow->SaveAs		= SaveAsContents;
	objectWindow->Revert		= RevertContents;
	objectWindow->Activate		= MyActivate;

//LR 180	if( gPrefs.useColor )
	dWin->csResID = gPrefs.csResID;	// LR: 1.5 - color selection
	dWin->csMenuID = gPrefs.csMenuID;
//LR 180	else
//LR 180		dWin->csResID = -1;	// LR: if created w/o color then offscreen is 1 bit, NO COLOR possible!

	// Make it the current grafport
	SetPortWindowPort( theWin );
	
//LR 180	dWin->offscreen = _newCOffScreen( kHexWindowWidth - kSBarSize, g.maxHeight - kHeaderHeight );	// LR: 1.7 - areas for scroll bar & header not needed!
//LR 180	if( !dWin->offscreen )
//LR 180			ErrorAlert( ES_Fatal, errMemory );

	SizeEditWindow( theWin, type );

	GetWindowPortBounds( theWin, &r );

//LR: 1.7 -fix lpp calculation!	dWin->linesPerPage = ( r.bottom - TopMargin - BotMargin - ( kHeaderHeight-1 ) ) / kLineHeight + 1;

/*LR 175 -- done in SetupScrollBars (this was causing bogus scrollbar values!)
	dWin->linesPerPage = (maxheight - kHeaderHeight) / kLineHeight;
	dWin->startSel = dWin->endSel = 0L;
	dWin->editMode = EM_Hex;
	dWin->lastTypePos = -1;	//LR 1.72 -- allow insertion before first char to get into undo buffer
*/
	//LR: 1.7 - what was this??? ((WStateData *) *((WindowPeek)theWin)->dataHandle)->stdState.left + kHexWindowWidth;

	LocalToGlobal( (Point *)&r.top );
	LocalToGlobal( (Point *)&r.bottom );

	//LR 180 -- restrict zoomed (1/2 size) state to full lines
	r.bottom = (r.top + (((r.bottom - r.top) - kHeaderHeight) / (kLineHeight * 2)) * kLineHeight) + kHeaderHeight;

	SetWindowStandardState( theWin, &r );

	return noErr;
}

#pragma mark -

#define DataItem		11
#define RsrcItem		12
#define SmartItem		13

#if !defined(__MC68K__) && !defined(__SC__)		//LR 1.73 -- not available for 68K (won't even link!)
/*** NAV SERVICES EVENT FILTER ***/
static pascal void _navEventFilter( NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD )
{
	static Handle navDITL = NULL; // BB: custom control DITL
	
	// BB: setup and handle custom NavGet controls - Fixes bug #229519
	if( callBackUD == kNavOpenDialogType )
	{
		//short theItem;
		short itemCount;
		ControlRef theControl;
		DialogPtr theDialog;
		switch( callBackSelector )
		{
			case kNavCBCustomize://Negotiate the custom area size
			 	if( callBackParms->customRect.bottom == 0 )
			 	{
					callBackParms->customRect.bottom = callBackParms->customRect.top+kNavDITLHeight;
					callBackParms->customRect.right = callBackParms->customRect.left+kNavDITLWidth;
				}
				break;
			
			case kNavCBStart:
				navDITL = GetResource( 'DITL', kNavDITLID );	//LR 1.73 -- no need to keep loaded, resource manager does that for us!
				if( navDITL != NULL )
				{
					theDialog = GetDialogFromWindow( callBackParms->window );
					itemCount = CountDITL( theDialog );
					if( NavCustomControl( callBackParms->context, kNavCtlAddControlList, navDITL ) == noErr)
					{
						//since Nav Services won't work w/o the AppearanceMgr, we go ahead and use Appearance calls w/o checking
						short i = itemCount+kDataForkRadioID;
						//init the controls
						for( ; i <= (itemCount+kRsrcForkRadioID); ++i )
						{
							GetDialogItemAsControl( theDialog, i, &theControl );
							if( theControl != NULL )
								SetControlValue( theControl, 0 );
						}
						GetDialogItemAsControl( theDialog, itemCount+kAutoForkRadioID, &theControl );
						if( theControl != NULL )
							SetControlValue( theControl, 1 );
					}
				}	
				break;
			
			case kNavCBTerminate:
				if( navDITL != NULL )
				{
					short i;
					theDialog = GetDialogFromWindow( callBackParms->window );
					itemCount = CountDITL( theDialog );
					//set the fork mode based on the selection
					for( i = itemCount; i >= itemCount - 2; --i )
					{
						GetDialogItemAsControl( theDialog, i, &theControl );
						if( theControl != NULL )
						{
							if( GetControlValue( theControl) >= 1 )
							{
								g.forkMode = abs( (i - itemCount) ) + 1; //dependant on ForkModes being 1,2,3
								//the FM_Data and FM_Smart modes will be reversed
								if( g.forkMode == FM_Smart ) g.forkMode = FM_Data;
								else if( g.forkMode == FM_Data ) g.forkMode = FM_Smart;
								break;
							}
						}
					}
					ReleaseResource( navDITL );
					navDITL = NULL;
				}
				break;
		}
	}
}

/*** NAV SERVICES PREVIEW FILTER ***/
/*
static pascal Boolean _navPreviewFilter( NavCBRecPtr callBackParms, void *callBackUD )
{
	#pragma unused( callBackParms, callBackUD )
	return false;
}
*/

/*** NAV SERVICES FILE FILTER ***/
/*
static pascal Boolean _navFileFilter( AEDesc* theItem, void* info, void *callBackUD, NavFilterModes filterMode )
{
	#pragma unused( theItem, info, callBackUD, filterMode )
	return true;
}
*/
#endif	//POWERPC

#if !TARGET_API_MAC_CARBON		// standard file callbacks not applicable with carbon

/*** SOURCE DLOG HOOK ***/
//LR 1.72 -- at some point the fork mode was changed from zero to one based,
//			probably in my contants cleanup. This caused the WRONG control to be
//			selected for non-appearance cases ... and then CRASH!
static pascal short _sourceDLOGHook( short item, DialogPtr theDialog )
{
	switch( item )
	{
		case DataItem:
		case RsrcItem:
		case SmartItem:
			SetControl( theDialog, g.forkMode + DataItem - 1, false );
			g.forkMode = item - DataItem;
			SetControl( theDialog, g.forkMode + DataItem - 1, true );
			return sfHookNullEvent;	/* Redraw the List */
		
		case sfHookFirstCall:
			SetControl( theDialog, g.forkMode + DataItem - 1, true );
			return sfHookNullEvent;
	}
	return item;
}

/*** SOURCE DLOG FILTER ***/
/*LR 180 -- removed, now set prompt string at launch
static pascal Boolean _sourceDLOGFilter( DialogPtr dlg, EventRecord *event, short *item )
{
	Str63		prompt;

	if( activateEvt == event->what )
	{
		GetIndString( prompt, strPrompt, CompareFlag+2 );	// LR: v1.6.5 localizable way!
		SetText( dlg, 14, prompt );
	}

	return StdFilterProc( dlg, event, item );
}
*/

#endif

/*** ASK EDIT WINDOW ***/
#if !defined(__MC68K__) && !defined(__SC__)		//LR 1.73 -- not available for 68K (won't even link!)
short AskEditWindowNav( tWindowType type )
{
//#if TARGET_API_MAC_CARBON	// LR: v1.6  -- BB no longer used
	OSStatus error = noErr;
	NavReplyRecord		reply;
	NavDialogOptions	dialogOptions;
 	NavEventUPP			eventProc = NewNavEventUPP( _navEventFilter );
	NavPreviewUPP		previewProc = NULL;
	NavObjectFilterUPP	filterProc = NULL;
	NavTypeListHandle	openTypeList = NULL;

#if TARGET_API_MAC_CARBON	//LR 175 -- just to require v1.1 (we crash under v1.0x!)
	NavDialogCreationOptions opt;
	NavGetDefaultDialogCreationOptions( &opt );
#endif
	
	NavGetDefaultDialogOptions( &dialogOptions );
	dialogOptions.dialogOptionFlags |= kNavNoTypePopup+kNavAllowInvisibleFiles; // BB: allow invisible files - fixes bug #425256
	dialogOptions.dialogOptionFlags |= kNavSupportPackages+kNavAllowOpenPackages;   // benh57: allow browsing into packages. (496196)
	GetIndString( dialogOptions.message, strPrompt, type + 2 );	//LR 180 -- modify prompt

	error = NavGetFile( NULL, &reply, &dialogOptions, eventProc, previewProc, filterProc, openTypeList, kNavOpenDialogType);
	if( reply.validRecord || !error )
	{
		AEKeyword 	keyword;
		DescType 	descType;
		FSSpec		openedSpec;	
		Size 		actualSize;
		
		error = AEGetNthPtr( &(reply.selection), 1, typeFSS, &keyword, &descType, &openedSpec, sizeof(FSSpec), &actualSize );
		if( !error )
			OpenEditWindow( &openedSpec, type, true );

		NavDisposeReply( &reply );
	}
	else error = ioErr;		// user cancelled
	DisposeNavEventUPP( eventProc );
	AdjustMenus();
	return error == noErr? 0 : -1;
}
#endif	//POWERPC

#if !TARGET_API_MAC_CARBON	// BB:  replaced
short AskEditWindowSF( tWindowType type )
{
	SFReply		macSFReply;
	FSSpec		fSpec;
	long		procID;
	Point		where = { -1, -1 };
	Str255		prompt;

	DlgHookUPP myGetFileUPP;		// LR: v1.6.5 limited to this routine
//LR 180	ModalFilterUPP myFilterUPP;

	myGetFileUPP = NewDlgHookProc( _sourceDLOGHook );
//LR 180	myFilterUPP = NewModalFilterProc( _sourceDLOGFilter );

	// LR: make less of a hack!
// LR: v1.6.5 localization	= { "\pFile to Open:", "\pFirst File to Compare:", "\pSecond File to Compare:"};
/*
	// LR: hacks for requesting different files for comparingÉ

	if( CompareFlag == 1 )
		SFPGetFile( where, "\pFirst File to Compare:", NULL, -1, NULL, myGetFileUPP, &macSFReply, dlgGetFile, NULL );
	else if( CompareFlag == 2 )
		SFPGetFile( where, "\pSecond File to Compare:", NULL, -1, NULL, myGetFileUPP, &macSFReply, dlgGetFile, NULL );
	else
*/
	GetIndString( prompt, strPrompt, type + 2 );	//LR 177 -- send now, instead of modifying dialog later

	SFPGetFile( where, prompt, NULL, -1, NULL, myGetFileUPP, &macSFReply, dlgGetFile, /*LR 180 myFilterUPP*/ NULL );

	DisposeDlgHookUPP( myGetFileUPP );
//LR 180	DisposeModalFilterUPP( myFilterUPP );

	if( macSFReply.good )
	{
		BlockMove( macSFReply.fName, fSpec.name, macSFReply.fName[0]+1 );
		GetWDInfo( macSFReply.vRefNum, &fSpec.vRefNum, &fSpec.parID, &procID );
		OpenEditWindow( &fSpec, type, true );
	}
	else return -1;

	AdjustMenus();
	return 0;
}
#endif

#pragma mark -

/*** CLEANUP EDITOR ***/
void CleanupEditor( void )
{
	PrefsSave();

	// LR: v1.6.5 now need to dispose of these at exit since they never truly "close"
	if( g.searchDlg )
	{
		DisposeDialog( g.searchDlg );
		g.searchDlg = NULL;
	}

	if( g.gotoDlg )
	{
		DisposeDialog( g.gotoDlg );
		g.gotoDlg = NULL;
	}
}

/*** INITIALIZE EDITOR ***/
void InitializeEditor( void )
{
//LR 175	CursHandle	cursorHandle = NULL;
	Str255		str;
	SInt32		val;
	FontInfo	finfo;
	WindowRef	newWin;
#if !TARGET_API_MAC_CARBON	// LR: v1.6
	PScrapStuff			ScrapInfo;

	ScrapInfo = InfoScrap();
	if( ScrapInfo->scrapState < 0 )
		ZeroScrap();
#endif

	// Start Profiling
#if PROFILE			// 6/15 Optional profiling support
	freopen( "profile.log", "w", stdout );		// If console isn't wanted
	InitProfile( 200, 200 );
	_profile = 0;
	// cecho2file( "profile.log", false, stdout );	// If console is wanted
#endif

#if TARGET_API_MAC_CARBON	// LR: v1.6
{
/*LR 180 -- system call takes into account doc!
	BitMap qdScreenBits;

	GetQDGlobalsScreenBits( &qdScreenBits );
	g.maxHeight = qdScreenBits.bounds.bottom - qdScreenBits.bounds.top - 24;
*/
	Rect r;

	GetAvailableWindowPositioningBounds( GetMainDevice(), &r );
	g.maxHeight = r.bottom - r.top;
}
#else
	g.maxHeight = qd.screenBits.bounds.bottom - qd.screenBits.bounds.top - 24;	// LR: add 'qd.'
#endif

	//LR 1.72 -- more flexability in font usage, get from string and find width/height from actual data

	newWin = GetNewCWindow( (g.useAppearance && g.systemVersion >= kMacOSEight) ? kAppearanceWindow : kSystem7Window, NULL, kLastWindowOfClass );	//LR 1.72 don't change system font!
	SelectWindow( newWin );
	SetPortWindowPort( newWin );

	GetIndString( str, strFont, 1 );
	GetFNum( str, &g.fontFaceID );		// 1.7 carsten-unhardcoded font name & size
	GetIndString( str, strFont, 2 );
	StringToNum( str, &val );			//LR 1.72 -- get font info from resource
	g.fontSize = (short)val;
	TextFont( g.fontFaceID );
	TextSize( g.fontSize );
	GetFontInfo( &finfo );
	g.charWidth = CharWidth( '0' );	//LR -- should, but doesn't, work -> finfo.widMax;
	g.lineHeight = finfo.ascent + finfo.descent;

	DisposeWindow( newWin );	// done w/temp window, get rid of it

	//LR 177 -- create global offscreen drawing surface

	g.offscreen = _newCOffScreen( kHexWindowWidth - kSBarSize, g.maxHeight - kHeaderHeight );	// LR: 1.7 - areas for scroll bar & header not needed!
	if( !g.offscreen )
			ErrorAlert( ES_Fatal, errMemory );

	//LR 160 -- start up printing in classic mode

#if !TARGET_API_MAC_CARBON
	PrOpen();
	g.HPrint = (THPrint) NewHandle( sizeof(TPrint) );
	if( !g.HPrint )
		ErrorAlert( ES_Fatal, errMemory );

	PrintDefault( g.HPrint );
	PrClose();
#endif
}

/*** SIZE A WINDOW APPR. TO SITUATION ***/
//LR 175 -- seperated to allow calling from OpenWindow to handle compare requests on open windows
//NP 177 -- Global for compare's usage
//LR 177 -- added window type parameter from NP's suggestion :)

void SizeEditWindow( WindowRef theWin, tWindowType type )
{
	EditWindowPtr dWin = (EditWindowPtr)GetWRefCon( theWin );
	short maxheight = g.maxHeight / 2 - 96;
	Rect r;

	// LR:	Hack for comparing two files
	if( kWindowCompareTop == type )
	{
		MoveWindow( theWin, 14, 48, true );
		CompWind1 = theWin;
	}
	else if( kWindowCompareBtm == type )
	{
		MoveWindow( theWin, 14, maxheight + 48, true );
		CompWind2 = theWin;
		maxheight += (maxheight + 48);		//LR 180 -- required to keep window of correct height
	}
	else	// kWindowNormal
	{
		maxheight = g.maxHeight;
	}

	//LR 180 -- make sure bottom of window doesn't go off end of screen
	//	NOTE:	Carbon top is positive, classic is negative!
	GetWindowBounds( theWin, kWindowStructureRgn, &r );
#if TARGET_API_MAC_CARBON
		maxheight -= (r.top + 24);	// need menu bar
#else
		maxheight += r.top;
#endif

	// Check for best window size
	if( (dWin->fileSize / kBytesPerLine) * kLineHeight < maxheight )
	{
		maxheight = (((dWin->fileSize + (kBytesPerLine - 1)) / kBytesPerLine) * kLineHeight);

		// Make sure window is at least of some usable size!
		if( maxheight < (kLineHeight * 10) )
			maxheight = (kLineHeight * 10);
	}

	// LR: v1.6.5 round this to a size showing only full lines
	maxheight = ((maxheight / kLineHeight) * kLineHeight) + kHeaderHeight;

	SizeWindow( theWin, kHexWindowWidth, maxheight, true );

	// Show the theWin
	SelectWindow( theWin );
	ShowWindow( theWin );

	SetupScrollBars( dWin );
}

/*** CLOSE EDIT WINDOW ***/
Boolean	CloseEditWindow( WindowRef theWin )
{
	short			i, n;
//	Str63			fileName;
	Str255			windowName, menuItemTitle;
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	MenuRef			windowMenu;

	MySetCursor( C_Arrow );

	if( dWin->dirtyFlag )
	{
//LR 1.72		GetWTitle( theWin, fileName );
		if( !g.useNavServices ) // BB: use Nav Services ?
		{
			ParamText( dWin->fsSpec.name, NULL, NULL, NULL );
			switch( CautionAlert( alertSave, NULL ) )
			{
				case ok:
					SaveContents( theWin );	
					break;
					
				case cancel:
					return false;
					
				case 3:
					// Discard
					break;
			}
		}
#if !defined(__MC68K__) && !defined(__SC__)		//LR 1.73 -- not available for 68K (won't even link!)
		else		// BB: code to support Nav Services
		{
			OSStatus error = noErr;
			NavAskSaveChangesResult		reply;
			NavDialogOptions	dialogOptions;
			NavEventUPP			eventProc = NewNavEventUPP( _navEventFilter );

			NavGetDefaultDialogOptions( &dialogOptions );
			BlockMoveData( dWin->fsSpec.name, dialogOptions.savedFileName, dWin->fsSpec.name[0]+1 );//set the file name string
			error = NavAskSaveChanges( &dialogOptions, kNavSaveChangesClosingDocument, &reply, eventProc, NULL );
			if( error != noErr )	return false; //on error, make sure we don't destroy the contents
			switch( reply )
			{
				case kNavAskSaveChangesSave:
					SaveContents( theWin );
					break;
				
				case kNavAskSaveChangesCancel:
					return false;
					
				case kNavAskSaveChangesDontSave:
					break;				
			}
			
			DisposeNavEventUPP( eventProc );
		}
#endif	//POWERPC
	}

	// NS: v1.6.6, remove window from menu on closing
	GetWTitle( theWin, windowName );
	windowMenu = GetMenuHandle( kWindowMenu );
	n = CountMenuItems(windowMenu);
	for( i = 1; i <= n; i++ )
	{
		GetMenuItemText( windowMenu, i, menuItemTitle );
		if( EqualPStrings( windowName, menuItemTitle ) )
		{
			DeleteMenuItem( windowMenu, i );
			n = i+1;
		}
	}

	//LR 1.73 :if a compare window, clear ptr so compare routine can exit!
	if( theWin == CompWind1 )
		CompWind1 = NULL;
	if( theWin == CompWind2 )
		CompWind2 = NULL;

	((ObjectWindowPtr)dWin)->Dispose( theWin );

	// LR: v1.7 -- if no edit window available, close find windows
	if( !FindFirstEditWindow() )
	{
		if( g.gotoDlg )
			HideWindow( GetDialogWindow( g.gotoDlg ) );
		if( g.searchDlg )
			HideWindow( GetDialogWindow( g.searchDlg ) );
	}

	return true;
}

/*** CLOSE ALL EDIT WINDOWS ***/
Boolean CloseAllEditWindows( void )
{
	WindowRef next, theWin = FrontNonFloatingWindow();

	while( theWin )
	{
		long windowKind = GetWindowKind( theWin );

		next = GetNextWindow( theWin );

/*LR 1.7 -- now closed if no windows open!
		if( (DialogPtr)theWin == g.searchDlg )
		{
			DisposeDialog( g.searchDlg );
			g.searchDlg = NULL;
		}
		else*/
		if( windowKind == kHexEditWindowTag )
			if( !CloseEditWindow( theWin ) )
				return false;

		theWin = next;
	}

	return true;
}

/*** OPEN EDIT WINDOW ***/
// LR 177 -- Now accepts window kind as a parameter instead of using global var CompareFlag

OSStatus OpenEditWindow( FSSpec *fsSpec, tWindowType type, Boolean showerr )
{
// LR: 1.5	WindowRef	theWin;
	EditWindowPtr		dWin;
	OSStatus			error;
	short				refNum=0;	//LR 181 -- , redo = false;
//LR 175	Point				where={-1, -1};
	HParamBlockRec		pb;
	FSSpec				workSpec;
	Str31				tempStr;
	long				fileEOF;	//LR 175
// LR: 1.5 	Rect		r, offRect;

	//LR 175 -- try to find the file in an open window first, and use it if found
	if( NULL != (dWin = LocateEditWindow( fsSpec, g.forkMode == FM_Smart ? -1 : g.forkMode )) )
	{
//LR 180		SizeEditWindow( dWin->oWin.theWin, type );
		SelectWindow( dWin->oWin.theWin );
		return( noErr );
	}

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 

	pb.fileParam.ioCompletion = 0l;
	pb.fileParam.ioNamePtr = fsSpec->name;
	pb.fileParam.ioVRefNum = fsSpec->vRefNum;
	pb.fileParam.ioDirID = fsSpec->parID;
	pb.fileParam.ioFDirIndex = 0;

	if( ( error = PBHGetFInfoSync( &pb ) ) != noErr )
	{
		if( showerr )
			ErrorAlert( ES_Caution, errFileInfo, error );

		return( error );
	}

	// Allocate our edit window storage
	dWin = (EditWindowPtr) NewPtrClear( sizeof(EditWindowRecord) );
	if( !dWin )
	{
		error = MemError();
		if( showerr )
			ErrorAlert( ES_Caution, errMemory );

		return( error );
	}

	// Handle the Opening of the Data Fork

	if( g.forkMode == FM_Data || (pb.fileParam.ioFlLgLen > 0 && g.forkMode == FM_Smart) )
	{
		dWin->fork = FT_Data;
//LR 175		error = HOpenDF( fsSpec->vRefNum, fsSpec->parID, fsSpec->name, fsRdPerm, &refNum );
		error = FSpOpenDF( fsSpec, fsRdWrPerm, &refNum );
		if( error )
		{
			dWin->readOnlyFlag = true;
			error = FSpOpenDF( fsSpec, fsRdPerm, &refNum );	//LR 180 -- on error try to open read-only
		}
		if( !error )
			error = GetEOF( refNum, &fileEOF );		//LR 175

		if( error == fnfErr || (!error && 0 == fileEOF) )
		{
			if( showerr )
			{
				GetIndString( tempStr, strFiles, FN_DATA );
				ParamText( fsSpec->name, tempStr, NULL, NULL );

				if( StopAlert( alertNoFork, NULL ) == 2 )
					goto contData;
			}

			error = fnfErr;		//LR 180 -- save lots of dup'd code (and prevent future missed mods)
			goto exitErr;

/*LR 175 -- empty forks are always opened, so we just have to warn about them being empty!

//LR 175			error = HCreate( fsSpec->vRefNum, fsSpec->parID, fsSpec->name, 
			error = FSpCreate( fsSpec, pb.fileParam.ioFlFndrInfo.fdCreator, pb.fileParam.ioFlFndrInfo.fdType, smSystemScript );
			if( error != noErr )
			{
				ErrorAlert( ES_Caution, errCreate, error );
				return error;
			}
//LR 175			error = HOpenDF( fsSpec->vRefNum, fsSpec->parID, fsSpec->name, fsRdPerm, &refNum );
			error = FSpOpenDF( fsSpec, fsRdWrPerm, &refNum );
*/
		}
		else if( error != noErr )
		{
contErr:
			if( showerr )
				ErrorAlert( ES_Caution, errOpen, error );
exitErr:
			if( dWin )
				DisposePtr( (Ptr)dWin );		//LR 180 -- no memory leaks!

			if( refNum )
				FSClose( refNum );		//LR 175

			return( error );
		}
contData:
		dWin->fileSize = pb.fileParam.ioFlLgLen;
	}
	else		// otherwise, Open the Resource Fork
	{
		dWin->fork = FT_Resource;
//LR 175		error = HOpenRF( fsSpec->vRefNum, fsSpec->parID, fsSpec->name, fsRdPerm, &refNum );
		error = FSpOpenRF( fsSpec, fsRdWrPerm, &refNum );
		if( error )
		{
			dWin->readOnlyFlag = true;
			error = FSpOpenRF( fsSpec, fsRdPerm, &refNum );	//LR 180 -- on error try to open read-only
		}
		if( !error )
			error = GetEOF( refNum, &fileEOF );		//LR 175

		if( error == fnfErr || (!error && 0 == fileEOF) )
		{
			if( showerr )
			{
				GetIndString( tempStr, strFiles, FN_RSRC );
				ParamText( fsSpec->name, tempStr, NULL, NULL );
				if( StopAlert( alertNoFork, NULL ) == 2 )
					goto contRsrc;
			}

			error = fnfErr;		//LR 180 -- save lots of dup'd code (and prevent future missed mods)
			goto exitErr;

/*LR 175 -- empty forks are always opened, so we just have to warn about them being empty!

//LR 175			HCreateResFile( fsSpec->vRefNum, fsSpec->parID, fsSpec->name );
			FSpCreateResFile( fsSpec, pb.fileParam.ioFlFndrInfo.fdCreator, pb.fileParam.ioFlFndrInfo.fdType, smSystemScript );
			if( ( error = ResError() ) != noErr )
			{
				ErrorAlert( ES_Caution, errCreate, error );
				return error;
			}
//LR 175			error = HOpenRF( fsSpec->vRefNum, fsSpec->parID, fsSpec->name, fsRdPerm, &refNum );
			error = FSpOpenRF( fsSpec, fsRdWrPerm, &refNum );
*/
		}
		else if( error != noErr )
			goto contErr;
contRsrc:
		dWin->fileSize = pb.fileParam.ioFlRLgLen;
	}

	/* if we get here, we have a valid file and data to read, or an empty file to work with */
	/* now, for OS X, we need to get the catalog information for later restoration when saving (file permissions) */

#if !defined(__MC68K__) && !defined(__SC__)
	if( FSGetCatalogInfo )	/* not available in all systems (OS 9 and later only it seems) */
	{
		FSRef ref;

		error = FSpMakeFSRef( fsSpec, &ref );
		if( !error )
		{
			error = FSGetCatalogInfo( &ref, kFSCatInfoGettableInfo, &dWin->catinfo, NULL/*name*/, NULL/*FSSpec*/, NULL/*parent*/ );
			dWin->OKToSetCatInfo = !error;
		}
	}
#endif

	/* Get a working file, in the temporary folder */
	dWin->refNum = refNum;
	workSpec = *fsSpec;
	error = FindFolder( kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &workSpec.vRefNum, &workSpec.parID );
	if( error != noErr )
	{
		if( showerr )
			ErrorAlert( ES_Caution, errFindFolder, error );
		return error;
	}

	if( workSpec.name[0] < 31 )
	{
		workSpec.name[0]++;
		workSpec.name[workSpec.name[0]] = '^';	//LR 1.72 -- temp filenames end with ^
	}
	else
		workSpec.name[31] ^= 0x10;

	_ensureNameIsUnique( &workSpec );
//LR 175	error = HCreate( workSpec.vRefNum, workSpec.parID, workSpec.name, kAppCreator, '????' );
	error = FSpCreate( &workSpec, kAppCreator, '????', smSystemScript );
	if( error != noErr )
	{
		if( showerr )
			ErrorAlert( ES_Caution, errCreate, error );
		return error;
	}
//LR 181	redo = false;

//LR 175	error = HOpen( workSpec.vRefNum, workSpec.parID, workSpec.name, fsRdWrPerm, &refNum );
	error = FSpOpenDF( &workSpec, fsRdWrPerm, &refNum );
	if( error != noErr )
	{
		if( showerr )
			ErrorAlert( ES_Caution, errOpen, error );
		return error;
	}

	/* setup our window varaiables */
	dWin->workSpec = workSpec;
	dWin->workRefNum = refNum;
	dWin->workBytesWritten = 0L;

	dWin->fileType = pb.fileParam.ioFlFndrInfo.fdType;
	dWin->creator = pb.fileParam.ioFlFndrInfo.fdCreator;
	dWin->creationDate = pb.fileParam.ioFlCrDat;

	dWin->fsSpec =
	dWin->destSpec = *fsSpec;

	error = _setupNewEditWindow( dWin, type );	// LR: 1.5 -make maintenence easier!
	if( !error )
		LoadFile( dWin );

	return error;
}

/*** DISPOSE EDIT WINDOW ***/
void DisposeEditWindow( WindowRef theWin )
{
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );

	UnloadFile( dWin );
	if( dWin->refNum ) FSClose( dWin->refNum );
	if( dWin->workRefNum )
	{
		FSClose( dWin->workRefNum );
		FSpDelete( &dWin->workSpec );
//LR 175		HDelete( dWin->workSpec.vRefNum, dWin->workSpec.parID, dWin->workSpec.name );
	}

	//LR 1.72 -- release undo if associated with this window
	if( dWin == gUndo.theWin )
	{
		ReleaseEditScrap( dWin, &gUndo.undoScrap );
		gUndo.type = 0;
	}
	if( dWin == gRedo.theWin )
	{
		ReleaseEditScrap( dWin, &gRedo.undoScrap );
		gRedo.type = 0;
	}

//LR 180	DisposeGWorld( dWin->offscreen );
	DefaultDispose( theWin );
	AdjustMenus();
}

/*** NEW EDIT WINDOW ***/
void NewEditWindow( void )
{
	EditWindowPtr		dWin;
	OSStatus			error;
	short				refNum = 0;	// 05/10/01 - GAB: NULL is a pointer type, and doesn't fit in a short
//LR 175	Point				where = { -1, -1 };
	FSSpec				workSpec;
// LR: 1.5	Rect				r, offRect;

	// Get the Template & Create the Window, initially set to the file's data fork

	dWin = (EditWindowPtr) NewPtrClear( sizeof(EditWindowRecord) );
	if( !dWin )
	{
		FSClose( refNum );
		ErrorAlert( ES_Caution, errMemory );
		return;
	}

	dWin->fork = FT_Data;
	dWin->fileSize = 0L;
	dWin->refNum = 0;

	// Initialize WorkSpec
	workSpec = dWin->workSpec;
	error = FindFolder( kOnSystemDisk, kTemporaryFolderType, kCreateFolder, &workSpec.vRefNum, &workSpec.parID );
	if( error != noErr )
	{
		ErrorAlert( ES_Caution, errFindFolder, error );
		return;
	}
	GetIndString( workSpec.name, strFiles, FN_Untitled );
//LR: 1.66	BlockMove( "\pUntitledw", workSpec.name, 10 );
	_ensureNameIsUnique( &workSpec );
//LR 175	HCreate( workSpec.vRefNum, workSpec.parID, workSpec.name, kAppCreator, '????' );
	error = FSpCreate( &workSpec, kAppCreator, '????', smSystemScript );
	if( error != noErr )
	{
		ErrorAlert( ES_Caution, errCreate, error );
		return;
	}
//LR 175	error = HOpenDF( workSpec.vRefNum, workSpec.parID, workSpec.name, fsRdWrPerm, &refNum );
	error = FSpOpenDF( &workSpec, fsRdWrPerm, &refNum );
	if( error != noErr )
	{
		ErrorAlert( ES_Caution, errOpen, error );
		return;
	}

	dWin->workSpec = dWin->fsSpec = workSpec;
	dWin->workRefNum = refNum;
	dWin->workBytesWritten = 0L;

	dWin->fileType = kDefaultFileType;
	dWin->creator = kAppCreator;
	dWin->creationDate = 0L;

	_setupNewEditWindow( dWin, kWindowNormal );	//LR 1.66 "\pUntitled" );	// LR: 1.5 -make mashortenence easier!

	dWin->firstChunk = NewChunk( 0L, 0L, 0L, CT_Unwritten );
	dWin->curChunk = dWin->firstChunk;
}

#pragma mark -

// Locate Edit Window	( LR 951121 )
// 
// ENTRY:	File's refnum of Edit theWin to find, and fork open ( -1 == ignore fork )
// 	EXIT:	ptr to edit theWin, or NULL if not found

EditWindowPtr LocateEditWindow( FSSpec *fs, short fork )
{
	WindowRef theWin = FrontNonFloatingWindow();

	while( theWin )
	{
		if( kHexEditWindowTag == GetWindowKind( theWin ) )	// LR: v1.6.5 fix search
		{
			EditWindowPtr dWin = (EditWindowPtr)GetWRefCon( theWin );

			if( dWin && (fork < 0 || dWin->fork == fork) )	// simple checks
			{
				if( EqualString( fs->name, dWin->fsSpec.name, false, true ) &&
						fs->vRefNum == dWin->fsSpec.vRefNum && fs->parID == dWin->fsSpec.parID )	// tedious, but only wayÉ
					return dWin;
			}
		}
		theWin = GetNextWindow( theWin );
	}
	return NULL;
}

/*** FIND NEXT EDIT WINDOW ***/
// NP 177 -- Added FineNextEditWindow, FindFirst calls w/NULL for first window

EditWindowPtr FindNextEditWindow( EditWindowPtr curr )
{
	WindowRef theWin, editWin = NULL;

	// Find and Select Top Window
	//LR: 1.66 total re-write to avoid null window references!

	if( ! curr )
	theWin = FrontNonFloatingWindow();
	else
		theWin =  GetNextWindow( curr->oWin.theWin );
	
	if( theWin ) do
	{
		if( GetWindowKind( theWin ) == kHexEditWindowTag )
			editWin = theWin;

		theWin = GetNextWindow( theWin );

	}while( theWin && !editWin  );

	if( !editWin )
		return( NULL );

	return( (EditWindowPtr)GetWRefCon( editWin ) );
}

/*** FIND FIRST EDIT WINDOW ***/
EditWindowPtr FindFirstEditWindow( void )
{
	return FindNextEditWindow( NULL );
}

/*** UPDATE EDIT WINDOWS ***/
//LR: 1.66 - avoid NULL window ref, DrawPage with CURRENT dWin (not first!)
void UpdateEditWindows( void )
{
	WindowRef		theWin = FrontNonFloatingWindow();
	EditWindowPtr	dWin;

	while( theWin )
	{
		long windowKind = GetWindowKind( theWin );
		if( windowKind == kHexEditWindowTag )
		{
			dWin = (EditWindowPtr)GetWRefCon( theWin );
//LR 180			DrawPage( dWin );
			UpdateOnscreen( theWin );
		}
		theWin = GetNextWindow( theWin );
	}
}

/*** MY ACTIVATE ***/
void MyActivate( WindowRef theWin, Boolean active )
{
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );

	if( dWin->vScrollBar )
		HiliteControl( dWin->vScrollBar, active? 0 : 255 );
	DefaultActivate( theWin, active );
}

#pragma mark -

/*** OFFSET SELECTION ***/
static void _offsetSelection( EditWindowPtr dWin, short offset, Boolean shiftFlag )
{
//LR 180	long	selWidth;
//LR 180	Boolean	fullUpdate;

//LR 180	selWidth = dWin->endSel - dWin->startSel;
//LR 180	fullUpdate = shiftFlag || selWidth > 1;

	if( offset < 0 )
	{
		if( dWin->startSel > 0 )
		{
			if( (short) 0xFEED == offset )	// LR: v1.6.5 LR	-"special code"
				dWin->startSel = 0;
			else
				dWin->startSel += offset;

			if( dWin->startSel < 0 )
				dWin->startSel = 0;
			if( !shiftFlag )
			{
				dWin->endSel = dWin->startSel;
				CursorOff( dWin->oWin.theWin );
			}
			if( !shiftFlag )
				CursorOn( dWin->oWin.theWin );
		}
		else
		{
			SysBeep( 1 );
			if( !shiftFlag )
				dWin->endSel = dWin->startSel;		//LR 1.72 -- deselect anyway
		}
		ScrollToSelection( dWin, dWin->startSel, false );
	}
	else
	{
		if( dWin->endSel < dWin->fileSize )
		{
			if( 0xBED == offset )	// LR: v1.6.5 LR	-"special code"
				dWin->endSel = dWin->fileSize;
			else
				dWin->endSel += offset;

			if( dWin->endSel > dWin->fileSize )
				dWin->endSel = dWin->fileSize;
			if( !shiftFlag )
			{
				dWin->startSel = dWin->endSel;
				CursorOff( dWin->oWin.theWin );
			}
			if( !shiftFlag )
				CursorOn( dWin->oWin.theWin );
		}
		else
		{
			SysBeep( 1 );
			if( !shiftFlag )
				dWin->startSel = dWin->endSel;		//LR 1.72 -- deselect anyway
		}
		ScrollToSelection( dWin, dWin->endSel, false );
	}
}

//LR 185 -- macros to easy playing with hiliting
//#define SETHILITE()	{char c = LMGetHiliteMode(); BitClr( &c, pHiliteBit ); LMSetHiliteMode( c ); }
//#define SETHILITE()
#define HILITERECT(r) PaintRect(r)
//#define HILITERECT(r) SETHILITE(); InvertRect(r)

/*** INVERT SELECTION ***/
//LR 180 -- changes to draw offscreen instead of directly to window
static void _hiliteSelection( EditWindowPtr	dWin )
{
	Rect	r;
	long	start, end;
	short	startX, endX;
	Boolean	frontFlag;
	RGBColor hColor;
//185	RGBColor invertColor;

	frontFlag = (dWin->oWin.theWin == FrontNonFloatingWindow() && dWin->oWin.active);

	if( dWin->endSel <= dWin->startSel )
		return;

	GetPortHiliteColor( GetWindowPort( dWin->oWin.theWin ), &hColor );
	RGBForeColor( &hColor );
	PenMode( adMin );

	// Set our inversion color
	if( ctHdl )
		RGBBackColor( &(*ctHdl)->body );
/*185
	if( ctHdl )
		invertColor = ( *ctHdl )->body;
	else
		invertColor = white;
	
	InvertColor( &invertColor );
*/	
	start = dWin->startSel - dWin->editOffset;
	if( start < 0 )
		start = 0;
	end = ( dWin->endSel-1 ) - dWin->editOffset;
	if( end > ( (dWin->linesPerPage + 1) * kBytesPerLine )-1 )
		end = ( (dWin->linesPerPage + 1) * kBytesPerLine )-1;
	
	startX = COLUMN( start );
	endX = COLUMN( end );
	
	// Are we the frontmost window? (ie, draw filled selection)
	if( frontFlag )
	{
		if( dWin->editMode == EM_Hex )	// color hex area?
		{
			if( LINENUM( start ) < LINENUM( end ) )	// yes, do we have more than one line?
			{
				// Invert Hex
				r.top = /*(kHeaderHeight / 2) +*/ LINENUM( start ) * kLineHeight;
				r.bottom = r.top + kLineHeight;
				r.left = kDataDrawPos + HEXPOS( startX ) - 3;
				r.right = kDataDrawPos + HEXPOS( kBytesPerLine ) - 3;
				HILITERECT( &r );

				// Outline Box around Ascii
				r.left = kTextDrawPos + CHARPOS( startX ) - 1;
				r.right = kTextDrawPos + CHARPOS( kBytesPerLine );
				
				MoveTo( kTextDrawPos, r.bottom );
				LineTo( r.left, r.bottom );
	
				LineTo( r.left, r.top );
				if( dWin->startSel >= dWin->editOffset )
					LineTo( r.right, r.top );
				else
					MoveTo( r.right, r.top );
				LineTo( r.right, r.bottom );
	
				// Invert Hex portion block (ie, multiple lines)
				if( LINENUM( start ) < LINENUM( end )-1 )
				{
					r.top = /*(kHeaderHeight / 2) +*/ LINENUM( start ) * kLineHeight + kLineHeight;
					r.bottom = /*(kHeaderHeight / 2) +*/ LINENUM( end ) * kLineHeight;
					r.left = kDataDrawPos - 3;
					r.right = kDataDrawPos + HEXPOS( kBytesPerLine ) - 3;
					HILITERECT( &r );
	
					r.left = kTextDrawPos - 1;
					r.right = kTextDrawPos + CHARPOS( kBytesPerLine );
					MoveTo( r.left, r.top );
					LineTo( r.left, r.bottom );
					MoveTo( r.right, r.top );
					LineTo( r.right, r.bottom );
				}
				r.top = /*(kHeaderHeight / 2) +*/ LINENUM( end ) * kLineHeight;
				r.bottom = r.top + kLineHeight;
				r.left = kDataDrawPos - 3;
				r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;
				HILITERECT( &r );
	
				r.left = kTextDrawPos - 1;
				r.right = kTextDrawPos + CHARPOS( endX ) + kCharWidth;	//LR 180 - 1;
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom-1 );
				if( dWin->endSel < dWin->editOffset + dWin->linesPerPage * kBytesPerLine )
				{
					LineTo( r.right, r.bottom-1 );
				}
				else
					MoveTo( r.right, r.bottom-1 );
				LineTo( r.right, r.top );
				LineTo( kTextDrawPos + CHARPOS( kBytesPerLine ), r.top );
			}
			else	// we only have a single line or less!
			{
				r.top = /*(kHeaderHeight / 2) +*/ LINENUM( start ) * kLineHeight;
				r.bottom = r.top + kLineHeight;
				r.left = kDataDrawPos + HEXPOS( startX ) - 3;
				r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;
				HILITERECT( &r );
	
				r.left = kTextDrawPos + CHARPOS( startX )-1;
				r.right = kTextDrawPos + CHARPOS( endX ) + kCharWidth;	//LR 180 - 1;
	
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom-1 );
				if( dWin->endSel < dWin->editOffset + dWin->linesPerPage * kBytesPerLine )
				{
					LineTo( r.right, r.bottom-1 );
				}
				else
					MoveTo( r.right, r.bottom-1 );
				LineTo( r.right, r.top );
				if( dWin->startSel >= dWin->editOffset )
				{
					LineTo( r.left, r.top );
				}
			}
		}
		else	// color in the ASCII area
		{
			if( LINENUM( start ) < LINENUM( end ) )		// more than one line?
			{
				// Outline Hex
				r.top = /*(kHeaderHeight / 2) +*/ LINENUM( start ) * kLineHeight;
				r.bottom = r.top + kLineHeight;
				r.left = kDataDrawPos + HEXPOS( startX ) - 3;
				r.right = kDataDrawPos + HEXPOS( kBytesPerLine ) - 3;
	
				MoveTo( kDataDrawPos - 3, r.bottom );
				LineTo( r.left, r.bottom );
				LineTo( r.left, r.top );
				if( dWin->startSel >= dWin->editOffset )
				{
					LineTo( r.right, r.top );
				}
				else
					MoveTo( r.right, r.top );
				LineTo( r.right, r.bottom );
	
				// Invert Ascii
				r.left = kTextDrawPos + CHARPOS( startX ) - 1;
				r.right = kTextDrawPos + CHARPOS( kBytesPerLine );
				HILITERECT( &r );
	
				if( LINENUM( start ) < LINENUM( end )-1 )
				{
					r.top = /*(kHeaderHeight / 2) +*/ LINENUM( start ) * kLineHeight + kLineHeight;
					r.bottom = /*(kHeaderHeight / 2) +*/ LINENUM( end ) * kLineHeight;
					r.left = kDataDrawPos - 3;
					r.right = kDataDrawPos + HEXPOS( kBytesPerLine ) - 3;
					MoveTo( r.left, r.top );
					LineTo( r.left, r.bottom );
					MoveTo( r.right, r.top );
					LineTo( r.right, r.bottom );
	
					r.left = kTextDrawPos - 1;
					r.right = kTextDrawPos + CHARPOS( kBytesPerLine );
					HILITERECT( &r );
				}
				r.top = /*(kHeaderHeight / 2) +*/ LINENUM( end ) * kLineHeight;
				r.bottom = r.top + kLineHeight;
				r.left = kDataDrawPos - 3;
				r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom );
				if( dWin->endSel < dWin->editOffset + dWin->linesPerPage * kBytesPerLine )
				{
					LineTo( r.right, r.bottom );
				}
				else
					MoveTo( r.right, r.bottom );
				LineTo( r.right, r.top );
				LineTo( kDataDrawPos + HEXPOS( kBytesPerLine ) - 3, r.top );
	
				r.left = kTextDrawPos - 1;
				r.right = kTextDrawPos + CHARPOS( endX ) + kCharWidth;	//LR 180 - 1;
				HILITERECT( &r );
			}
			else	// one line only
			{
				r.top = /*(kHeaderHeight / 2) +*/ LINENUM( start ) * kLineHeight;
				r.bottom = r.top + kLineHeight;
				r.left = kDataDrawPos + HEXPOS( startX ) - 3;
				r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom );
				if( dWin->endSel < dWin->editOffset + dWin->linesPerPage * kBytesPerLine )
				{
					LineTo( r.right, r.bottom );
				}
				else
					MoveTo( r.right, r.bottom );
				LineTo( r.right, r.top );
				if( dWin->startSel >= dWin->editOffset )
				{
					LineTo( r.left, r.top );
				}
	
				r.left = kTextDrawPos + CHARPOS( startX ) - 1;
				r.right = kTextDrawPos + CHARPOS( endX ) + kCharWidth;
				HILITERECT( &r );
			}
		}
	}
	else	// We are in the background
	{
		if( LINENUM( start ) < LINENUM( end ) )
		{
			// Outline Hex
			r.top = /*(kHeaderHeight / 2) +*/ LINENUM( start ) * kLineHeight;
			r.bottom = r.top + kLineHeight;
			r.left = kDataDrawPos + HEXPOS( startX ) - 3;
			r.right = kDataDrawPos + HEXPOS( kBytesPerLine ) - 3;

			MoveTo( kDataDrawPos - 3, r.bottom );
			LineTo( r.left, r.bottom );
			LineTo( r.left, r.top );
			if( dWin->startSel >= dWin->editOffset )
			{
				LineTo( r.right, r.top );
			}
			else
				MoveTo( r.right, r.top );
			LineTo( r.right, r.bottom );

			// Outline Box around Ascii
			r.left = kTextDrawPos + CHARPOS( startX ) - 1;
			r.right = kTextDrawPos + CHARPOS( kBytesPerLine );
			
			MoveTo( kTextDrawPos, r.bottom );
			LineTo( r.left, r.bottom );

			LineTo( r.left, r.top );
			if( dWin->startSel >= dWin->editOffset )
				LineTo( r.right, r.top );
			else
				MoveTo( r.right, r.top );
			LineTo( r.right, r.bottom );

			if( LINENUM( start ) < LINENUM( end ) - 1 )
			{
				r.top = /*(kHeaderHeight / 2) +*/ LINENUM( start ) * kLineHeight + kLineHeight;
				r.bottom = /*(kHeaderHeight / 2) +*/ LINENUM( end ) * kLineHeight;
				r.left = kDataDrawPos - 3;
				r.right = kDataDrawPos + HEXPOS( kBytesPerLine ) - 3;
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom );
				MoveTo( r.right, r.top );
				LineTo( r.right, r.bottom );

				r.left = kTextDrawPos - 1;
				r.right = kTextDrawPos + CHARPOS( kBytesPerLine );
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom );
				MoveTo( r.right, r.top );
				LineTo( r.right, r.bottom );
			}
			r.top = /*(kHeaderHeight / 2) +*/ LINENUM( end ) * kLineHeight;
			r.bottom = r.top + kLineHeight;
			r.left = kDataDrawPos - 3;
			r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;
			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom );
			if( dWin->endSel < dWin->editOffset + dWin->linesPerPage * kBytesPerLine )
				LineTo( r.right, r.bottom );
			else
				MoveTo( r.right, r.bottom );
			LineTo( r.right, r.top );
			LineTo( kDataDrawPos + HEXPOS( kBytesPerLine ) - 3, r.top );

			r.left = kTextDrawPos - 1;
			r.right = kTextDrawPos + CHARPOS( endX ) + kCharWidth - 1;
			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom-1 );
			if( dWin->endSel < dWin->editOffset + dWin->linesPerPage * kBytesPerLine )
				LineTo( r.right, r.bottom-1 );
			else
				MoveTo( r.right, r.bottom-1 );
			LineTo( r.right, r.top );
			LineTo( kTextDrawPos + CHARPOS( kBytesPerLine ), r.top );
		}
		else
		{
			r.top = /*(kHeaderHeight / 2) +*/ LINENUM( start ) * kLineHeight;
			r.bottom = r.top + kLineHeight;
			r.left = kDataDrawPos + HEXPOS( startX ) - 3;
			r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;
			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom );
			if( dWin->endSel < dWin->editOffset + dWin->linesPerPage * kBytesPerLine )
				LineTo( r.right, r.bottom );
			else
				MoveTo( r.right, r.bottom );
			LineTo( r.right, r.top );
			if( dWin->startSel >= dWin->editOffset )
				LineTo( r.left, r.top );

			r.left = kTextDrawPos + CHARPOS( startX )-1;
			r.right = kTextDrawPos + CHARPOS( endX ) + kCharWidth - 1;

			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom-1 );
			if( dWin->endSel < dWin->editOffset + dWin->linesPerPage * kBytesPerLine )
			{
				LineTo( r.right, r.bottom-1 );
			}
			else
				MoveTo( r.right, r.bottom-1 );
			LineTo( r.right, r.top );
			if( dWin->startSel >= dWin->editOffset )
			{
					LineTo( r.left, r.top );
			}
		}
	}

	//LR 185 -- Ensure normal draws after we are done!
	if( ctHdl )
		RGBBackColor( &white );

	RGBForeColor( &black );
	PenMode( srcCopy );
}

/*** INIT COLOUR TABLE ***/
static OSStatus _initColorTable( HEColorTablePtr ct )
{
	ct->header.red = ct->header.green = ct->header.blue =	// default light grey scheme
	ct->bar.red = ct->bar.green = ct->bar.blue = 0xCFFF;
	ct->headerLine.red = ct->headerLine.green = ct->headerLine.blue =
	ct->barLine.red = ct->barLine.green = ct->barLine.blue = 0x7FFF;
	ct->headerText = ct->barText = ct->text = black;
	ct->body = white;
	ct->bodyDark = white;
	return noErr;
}

/*** GET COLOUR INFO ***/
static OSStatus _getColorInfo( EditWindowPtr dWin )
{
	// NS: 1.3, set hilight colour
/*	Handle vars = dWin->oWin.theWin.port.grafVars;
	RGBColor hilightColour = (** (GVarHandle) vars).rgbHiliteColor;
	RGBColor rgbBlack = { nixBlack, nixBlack, nixBlack };
	RGBForeColor( &hilightColour );
*/	
	if( gPrefs.useColor && dWin->csResID > 0 )
	{
		ctHdl = (HEColorTableHandle) GetResource( 'HEct', dWin->csResID );
		if( !ctHdl )
		{
			ctHdl = (HEColorTableHandle) NewHandle( sizeof(HEColorTable_t) );
			if( ctHdl ) _initColorTable( *ctHdl );
		}
	}
	else ctHdl = NULL;	// LR: all that's needed to be nonQD compatible?
	return noErr;
}

/*** DRAW HEADER ***/
static void _drawHeader( EditWindowPtr dWin, Rect *r )
{
	char str[256];	//LR 1.7 -- no need for fork, replaced with selection length, str2[31];

	_getColorInfo( dWin );	// LR: v1.6.5 ensure that updates are valid!

	TextFont( g.fontFaceID );
	TextSize( g.fontSize );
	TextFace( normal );	// LR: v1.6.5 LR - can't be bold 'cause then the new stuff doesn fit :0
	TextMode( srcCopy );

	// LR: if we have color table, fill in the address bar!
	if( ctHdl )
	{
		RGBBackColor( &( *ctHdl )->header );
		RGBForeColor( &( *ctHdl )->headerLine );
	}
	else
	{
		RGBForeColor( &black );
		RGBBackColor( &white );
	}

	EraseRect( r );	// uses back color

	r->right -= kSBarSize;	// LR: v1.6.5 don't overwrite scroll bar icon

	MoveTo( r->left, r->bottom );	// LR: 1.7 - only one line, darker above scroll bar looked bogus
	LineTo( r->right, r->bottom );	// uses fore color

	if( ctHdl )
		RGBForeColor( &( *ctHdl )->headerText );

	// LR: v1.6.5 LR - more stuff in the header now & strings are from an localizable resource :)

	/* NS note to self:	%8	is length of eight (pads with spaces)
						%08	is length of eight (pads with zeros)
						%l	makes the number a long
						%d	identifies as a number (decimal)
						%X	identifies as a number (hexadecimal)		*/
	
	GetIndString( (StringPtr) str, strHeader, gPrefs.decimalAddr? HD_Decimal : HD_Hex );
	CopyPascalStringToC( (StringPtr) str, str );
//1.7	GetIndString( (StringPtr) str2, strHeader, dWin->fork );
//1.7	CopyPascalStringToC( (StringPtr) str2, str2 );
	sprintf( (char *) g.buffer, str, dWin->fileSize, &dWin->fileType, &dWin->creator, /*str2,*/ dWin->startSel, dWin->endSel, dWin->endSel - dWin->startSel );
	MoveTo( 5, r->top + kLineHeight );
	DrawText( g.buffer, 0, strlen( (char *) g.buffer ) );
	
	if( ctHdl )	// reset colors to known state
	{
		RGBForeColor( &black );
		RGBBackColor( &white );
	}
}

/*** DrawFooter ***/
// only used when printing
// LR: 1.7 - use TextUtils to get date/time strings in user preferred format!
static void _drawFooter( EditWindowPtr dWin, Rect *r, short pageNbr, short nbrPages )
{
	unsigned long	dt;
	Str63	s1, s2;

	TextFont( g.fontFaceID );
	TextSize( g.fontSize );
	TextFace( normal );
	TextMode( srcCopy );

	// LR: 1.7 - if we have color table, fill in the address bar!
	if( ctHdl )
	{
		RGBBackColor( &( *ctHdl )->header );
		RGBForeColor( &( *ctHdl )->headerLine );
	}
	else
	{
		RGBForeColor( &black );
		RGBBackColor( &white );
	}

	// Draw seperator line (seperates footer from body)
	MoveTo( r->left, r->top );
	LineTo( r->right, r->top );

	if( ctHdl )
		RGBForeColor( &( *ctHdl )->headerText );

	// Draw Date & Time on left edge of footer
	GetDateTime( &dt );
	DateString( dt, abbrevDate, s1, NULL );	//LR: 1.7 - get date/time strings as users wants them
	TimeString( dt, false, s2, NULL );
	sprintf( (char *)g.buffer, "%.*s %.*s", (int)s1[0], (char *)&s1[1], (int)s2[0], (char *)&s2[1] );
	MoveTo( 10, r->top + kLineHeight );
	DrawText( g.buffer, 0, strlen( (char *) g.buffer ) );

	// Draw filename in middle of footer
	GetIndString( s1, strHeader, HD_Footer );
	GetWTitle( dWin->oWin.theWin, s2 );
	sprintf( (char *)g.buffer, "%.*s %.*s", (int)s1[0], (char *)&s1[1], (int)s2[0], (char *)&s2[1] );
	MoveTo( ( r->left + r->right ) / 2 - TextWidth( g.buffer, 0, strlen((char *)g.buffer )) / 2, r->top + kLineHeight );
	DrawText( g.buffer, 0, strlen( (char *)g.buffer ) );

	// Draw page # & count on right edge of footer
	sprintf( (char *)g.buffer, "%d of %d", pageNbr, nbrPages );
	MoveTo( r->right - TextWidth( g.buffer, 0, strlen((char *)g.buffer )) - 8, r->top + kLineHeight );
	DrawText( g.buffer, 0, strlen( (char *)g.buffer ) );

	if( ctHdl )	// reset colors to known state
	{
		RGBForeColor( &black );
		RGBBackColor( &white );
	}
}

/*** DRAW DUMP ***/
// Draws the actual hex/decimal and ASCII panes in the window

// NOTE: the kStringTextPos stuff is a bit weird, but it's because I want to draw the leading space in the body
//			color instead of the address color (and because we don't print the extra spaces every time).

static OSStatus _drawDump( EditWindowPtr dWin, Rect *r, long sAddr, long eAddr )
{
	short	i, j, y;
	short	hexPos;
	short	asciiPos;
	register short	ch, ch1, ch2;
	long	addr;
	Rect addrRect;

	TextFont( g.fontFaceID );
	TextSize( g.fontSize );
	TextFace( normal );
	TextMode( srcCopy );

	// create address border bounds rectangle
	addrRect.top = r->top;	// we need to erase seperating space
	addrRect.left = r->left;
	addrRect.right = r->left + kBodyDrawPos + StringWidth( "\p 00000000:" );	//LR 180 -- use 8 chars, not 7!
	addrRect.bottom = r->bottom;

	if( ctHdl )
		RGBBackColor( &( *ctHdl )->bar );
	EraseRect( &addrRect );

	addr = sAddr - (sAddr % kBytesPerLine);
	g.buffer[kStringTextPos - 1] = g.buffer[kStringHexPos - 1] = g.buffer[kStringHexPos + kBodyStrLen] = ' ';

	// Now, draw each line of data
	for( y = r->top + (kLineHeight - 2), j = 0; y < r->bottom && addr < eAddr; y += kLineHeight, j++ )
	{
		if( gPrefs.decimalAddr )
			sprintf( (char *)&g.buffer[0], "%9ld:", addr );
		else
			sprintf( (char *)&g.buffer[0], " %08lX:", addr );

		// draw the address (not one big string due to different coloring!)
		if( ctHdl )
		{
			RGBBackColor( &( *ctHdl )->bar );
			RGBForeColor( &( *ctHdl )->barText );
		}

		MoveTo( kBodyDrawPos, y );
		DrawText( g.buffer, 0, kStringHexPos - 1 );

		// draw the data (hex and ascii)
		if( ctHdl )
		{
			Rect r2;

			RGBBackColor( (j & 1) ? &( *ctHdl )->bodyDark : &( *ctHdl )->body );	//LR 180 -- choose appr. body bkgnd color
			RGBForeColor( &( *ctHdl )->text );

			// LR: 1.7 -- must erase for this to show up on printouts!
			r2.top = y - (kLineHeight - 3);
			r2.left = addrRect.right + 1;
			r2.bottom = y + 3;
			r2.right = r->right;
			EraseRect( &r2 );
		}

		hexPos = kStringHexPos;
		asciiPos = kStringTextPos;

		for( i = kBytesPerLine; i; --i, ++addr )
		{
			if( addr >= sAddr && addr < eAddr )
			{
				ch = GetByte( dWin, addr );
				ch1 = ch2 = ch;
				ch1 >>= 4;
				ch2 &= 0x0F;

#define SINGLE_DRAW 1

#if SINGLE_DRAW
				g.buffer[hexPos++] = ch1 + (( ch1 < 10 )? '0' : ( 'A'-10 ));
				g.buffer[hexPos++] = ch2 + (( ch2 < 10 )? '0' : ( 'A'-10 ));
				g.buffer[hexPos++] = ' ';
				g.buffer[asciiPos++] = (ch >= 0x20 && ch <= g.highChar && 0x7F != ch) ? ch : '.';	// LR: 1.7 - 0x7F doesn't draw ANYTHING! Ouch!
#else
				g.buffer[0] = ch1 + (( ch1 < 10 )? '0' : ( 'A'-10 ));
				g.buffer[1] = ch2 + (( ch2 < 10 )? '0' : ( 'A'-10 ));
				g.buffer[2] = ' ';
				g.buffer[4] = (ch >= 0x20 && ch <= g.highChar && 0x7F != ch) ? ch : '.';	// LR: 1.7 - 0x7F doesn't draw ANYTHING! Ouch!
#endif
			}
			else
			{
#if SINGLE_DRAW
				g.buffer[hexPos++] = ' ';
				g.buffer[hexPos++] = ' ';
				g.buffer[hexPos++] = ' ';
				g.buffer[asciiPos++] = ' ';
#else
				g.buffer[0] = ' ';
				g.buffer[1] = ' ';
				g.buffer[2] = ' ';
				g.buffer[4] = ' ';
#endif
			}
#if !SINGLE_DRAW
			MoveTo( kDataDrawPos - kHexWidth + (kHexWidth * i), y );
			DrawText( g.buffer, 0, 3 );
			MoveTo( kTextDrawPos + (kCharWidth * i), y );
			DrawText( g.buffer, 4, 1 );
#endif
		}

		// %% NOTE: Carsten says to move this into for loop (ie, draw each byte's data) to
		//			prevent font smoothing from messing up the spacing
#if SINGLE_DRAW
		MoveTo( kDataDrawPos - kCharWidth, y );
		DrawText( g.buffer, kStringHexPos - 1, kBodyStrLen + 3 );
#endif
	}

	// Draw left edging? (line only, background erases, but line is erased by text!)
	if( ctHdl )
	{
//LR 1.7		RGBBackColor( &( *ctHdl )->bar );
		RGBForeColor( &( *ctHdl )->barLine );

//LR 1.7 - moved above		EraseRect( &addrRect );

		MoveTo( addrRect.right, addrRect.top );
		LineTo( addrRect.right, addrRect.bottom );
	}

	// Draw vertical bars?
	// based on David Emme's vertical bar mod
	if( gPrefs.vertBars )
	{
		if( ctHdl )
			RGBForeColor( &( *ctHdl )->dividerLine );	//LR 181 -- now had it's own color!

		MoveTo( CHARPOS(kStringHexPos + 11) - (kCharWidth / 2) - 1, addrRect.top );
		LineTo( CHARPOS(kStringHexPos + 11) - (kCharWidth / 2) - 1, addrRect.bottom );

		MoveTo( CHARPOS(kStringHexPos + 23) - (kCharWidth / 2) - 1, addrRect.top );
		LineTo( CHARPOS(kStringHexPos + 23) - (kCharWidth / 2) - 1, addrRect.bottom );

		MoveTo( CHARPOS(kStringHexPos + 35) - (kCharWidth / 2) - 1, addrRect.top );
		LineTo( CHARPOS(kStringHexPos + 35) - (kCharWidth / 2) - 1, addrRect.bottom );
	}

	// LR: restore color
	if( ctHdl )
	{
		RGBForeColor( &black );
		RGBBackColor( &white );
	}
	return noErr;
}

/*** DRAW PAGE ***/
static void _drawPage( EditWindowPtr dWin )
{
	GrafPtr			savePort;
	Rect			r;
	PixMapHandle	thePixMapH; // sel, for (un)LockPixels

#if PROFILE
	_profile = 1;
#endif

	_getColorInfo( dWin );	// LR: v1.6.5 multiple routines need this

	GetPort( &savePort );
	thePixMapH = GetGWorldPixMap( /*LR 180 dWin->*/ g.offscreen );
	if ( LockPixels( thePixMapH ) )
	{
		SetPort( ( GrafPtr )/*LR 180 dWin->*/ g.offscreen );

		GetPortBounds( /*LR 180 dWin->*/ g.offscreen, &r );

		if( ctHdl )
			RGBBackColor( &( *ctHdl )->body );
		else
		{
			ForeColor( blackColor );
			BackColor( whiteColor );
		}

		// Erase only that part of the buffer that isn't drawn to!
		if( (dWin->fileSize - dWin->editOffset) / kBytesPerLine <= dWin->linesPerPage )
		{
			Rect er = r;

			er.top = (((dWin->fileSize - dWin->editOffset) / kBytesPerLine) * kLineHeight);	//LR 1.72 -- need line height!
			if( (dWin->fileSize % kBytesPerLine) )	//LR 1.72 -- if not an empty line, no need to erase current line
				er.top += kLineHeight;
			EraseRect( &er );
		}

		_drawDump( dWin, &r, dWin->editOffset, dWin->fileSize );

		if( ctHdl )
		{
			RGBForeColor( &black );
			RGBBackColor( &white );
		}

		//LR: 180 -- we can now draw the selection offscreen since we always do full updates
		if( dWin->endSel > dWin->startSel && dWin->endSel >= dWin->editOffset && dWin->startSel < dWin->editOffset + (dWin->linesPerPage * kBytesPerLine) )
			_hiliteSelection( dWin );

		UnlockPixels( thePixMapH ); // sel
		SetPort( savePort );
	}

#if PROFILE
	_profile = 0;
#endif
}

/*** UPDATE ONSCREEN ***/
void UpdateOnscreen( WindowRef theWin )
{
	Rect			r1, r2;//, r3;
	GrafPtr			oldPort;
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	PixMapHandle thePixMapH;

	//LR 180 -- all updates now must draw the entire screen!
	_drawPage( dWin );

	// Now, draw the header information
	thePixMapH = GetPortPixMap( GetWindowPort( theWin ) );
	if ( LockPixels( thePixMapH ) )
	{
		GetPortBounds( /*LR 180 dWin->*/ g.offscreen, &r1 );
		GetWindowPortBounds( theWin, &r2 );

		GetPort( &oldPort );
		SetPortWindowPort( theWin );

		g.cursorFlag = false;

		// LR: Header drawn here due to overlapping vScrollBar
		r2.bottom = r2.top + kHeaderHeight - 1;
		_drawHeader( dWin, &r2 );

		// LR: adjust for scrollbar & header
		GetWindowPortBounds( theWin, &r2 );
// LR: 1.7		r2.right -= kSBarSize - 1;
//		r2.top += kHeaderHeight;
//		SectRect( &r1, &r2, &r3 );

		// restrict draw height to height of window port!
		r2.top = kHeaderHeight;
		r2.right -= kSBarSize;
		r1.bottom = r1.top + (r2.bottom - r2.top);

	//LR 160 -- Must blit a bit differently when in Carbon

	#if TARGET_API_MAC_CARBON
		CopyBits( GetPortBitMapForCopyBits( /*LR 180 dWin->*/ g.offscreen ), GetPortBitMapForCopyBits( GetWindowPort( theWin ) ), &r1, &r2, srcCopy, 0L );
	#else
		CopyBits( ( BitMap * ) &( /*LR 180 dWin->*/ g.offscreen )->portPixMap, &theWin->portBits, &r1, &r2, srcCopy, 0L );
	#endif

		//%% LR: 1.7 -- needs to be done offscreen, but then it's  not erased -- this is a new shell todo item :)
//LR 180		if( dWin->endSel > dWin->startSel && dWin->endSel >= dWin->editOffset && dWin->startSel < dWin->editOffset + (dWin->linesPerPage * kBytesPerLine) )
//LR 180			_hiliteSelection( dWin );

		UnlockPixels( thePixMapH );
		SetPort( oldPort );
	}
}

// Respond to an update event - BeginUpdate has already been called.

/*** MY DRAW ***/
void MyDraw( WindowRef theWin )
{
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	DrawControls( theWin );

	if( !g.useAppearance )	//LR 1.72 -- must call when not using Appearance
		DrawGrowIcon( theWin );

	// EraseRect( &theWin->portRect );
//LR 180	DrawPage( dWin );
	UpdateOnscreen( theWin );
}

/*** MY IDLE ***/
void MyIdle( WindowRef theWin, EventRecord *er )
{
	EditWindowPtr	dWin = (EditWindowPtr)GetWRefCon( theWin );

// LR: v1.6.5	long			scrapCount;
	Boolean			frontWindowFlag;
	Point			w;

	frontWindowFlag = (theWin == FrontNonFloatingWindow() && dWin->oWin.active);
	if( frontWindowFlag )
	{
		w = er->where;

		SetPortWindowPort( theWin );

		GlobalToLocal( &w );
		if( w.v >= kHeaderHeight && 
			w.v < kHeaderHeight + ( dWin->linesPerPage*kLineHeight ) )
		{
				if( w.h >= kDataDrawPos &&
					w.h < kDataDrawPos + ( kHexWidth * kBytesPerLine ) ) 
					MySetCursor( C_IBeam );
				else if( w.h >= kTextDrawPos &&
						 w.h < kTextDrawPos + ( kCharWidth * kBytesPerLine ) )
					MySetCursor( C_IBeam );
				else
					MySetCursor( C_Arrow );
		}
		else
			MySetCursor( C_Arrow );

		if( dWin->startSel == dWin->endSel ) {
// LR:			if( ( Ticks & 0x1F ) < 0x10 )
		if( ( TickCount() & 0x1F ) < 0x10 )
				CursorOn( theWin );
			else
				CursorOff( theWin );
		}
		// LR: v1.6.5 removed scrap check & grab. Check now in menu update, grab is in paste code
	}
}

// Respond to a mouse-click - highlight cells until the user releases the button

/*** MY HANDLE CLICK ***/
void MyHandleClick( WindowRef theWin, Point where, EventRecord *er )
{
	Point			w;
	long			pos, anchorPos = -1,
					sPos, ePos;
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	SetPortWindowPort( theWin );

	w = where;
	GlobalToLocal( &w );
	if( MyHandleControlClick( theWin, w ) )	// clicked on a window control (ie scrollbar)?
		return;

	// No, handle editing chore
	CursorOff( theWin );
	if( w.v >= kHeaderHeight && w.v < kHeaderHeight+( dWin->linesPerPage * kLineHeight ) )
	{
		do
		{
			AutoScroll( dWin, w );

			if( w.h >= kDataDrawPos && w.h < kTextDrawPos )	//LR 1.72 -- ichy! kDataDrawPos + (kHexWidth * (kBytesPerLine + 1)) )
			{
				pos = (((w.v - kHeaderHeight) / kLineHeight) * kBytesPerLine) + (w.h - kDataDrawPos + (kHexWidth - (kCharWidth*2))) / kHexWidth;
				dWin->editMode = EM_Hex;
			}
			else if( w.h >= kTextDrawPos && w.h < kTextDrawPos + (kCharWidth * (kBytesPerLine + 1)) )
			{
				pos = (((w.v - kHeaderHeight) / kLineHeight) * kBytesPerLine) + (w.h -  kTextDrawPos + (kCharWidth / 2)) / kCharWidth;
				dWin->editMode = EM_Ascii;
			}
			else
				goto newmousepos;

			pos += dWin->editOffset;
			if( pos < dWin->editOffset )
				pos = dWin->editOffset;
			if( pos > dWin->editOffset + (dWin->linesPerPage * kBytesPerLine) )
				pos = dWin->editOffset + (dWin->linesPerPage * kBytesPerLine);
			if( pos > dWin->fileSize )
				pos = dWin->fileSize;
			if( anchorPos == -1 )
			{
				if( er->modifiers & shiftKey )
					anchorPos = ( pos < dWin->startSel ) ? dWin->endSel : dWin->startSel;
				else anchorPos = pos;
			}
			sPos = pos < anchorPos ? pos : anchorPos;
			ePos = pos > anchorPos ? pos : anchorPos;
			if( ePos > dWin->fileSize )
				ePos = dWin->fileSize;

			if( sPos != dWin->startSel || ePos != dWin->endSel )
			{
				dWin->startSel = sPos;
				dWin->endSel = ePos;

				UpdateOnscreen( theWin );
			}
newmousepos:
			GetMouse( &w );

		} while ( WaitMouseUp() );
	}
}

/*** MY PROCESS KEY ***/
void MyProcessKey( WindowRef theWin, EventRecord *er )
{
	short			charCode, keyCode;
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	
	keyCode = (er->message & keyCodeMask) >> 8;
	charCode = (er->message & charCodeMask);
	if( er->modifiers & cmdKey ) return;
	
	switch( charCode )	// NS: safer on multi-lingual keyboards
	{
		// switch contexts
		case kTabCharCode:
		case kReturnCharCode:
		case kEnterCharCode:
			if( EM_Hex == dWin->editMode )	//NP 180 -- fix editMode = !editMode as editMode is not a boolean!!!
				dWin->editMode = EM_Ascii;
			else
				dWin->editMode = EM_Hex;

			UpdateOnscreen( dWin->oWin.theWin );
			break;
		
		// move insertion point
		case kLeftArrowCharCode:
			_offsetSelection( dWin, -1, (er->modifiers & shiftKey) > 0 );
			break;
		case kRightArrowCharCode:
			_offsetSelection( dWin, 1, (er->modifiers & shiftKey) > 0 );
			break;
		case kUpArrowCharCode:
			if( er->modifiers & optionKey )		//LR 180 -- option up == home
				goto dohome;

			_offsetSelection( dWin, -kBytesPerLine, (er->modifiers & shiftKey) > 0 );
			break;
		case kDownArrowCharCode:
			if( er->modifiers & optionKey )		//LR 180 -- option down == end
				goto doend;

			_offsetSelection( dWin, kBytesPerLine, (er->modifiers & shiftKey) > 0 );
			break;
		
		// scroll document
		case kPageUpCharCode:
			if( gPrefs.moveOnlyPaging )			//LR 180 -- option to only move display, not selection point
				ScrollToPosition( dWin, dWin->editOffset - (kBytesPerLine * (dWin->linesPerPage - 1)) );
			else
				_offsetSelection( dWin, -kBytesPerLine * (dWin->linesPerPage - 1), (er->modifiers & shiftKey) > 0 );
			break;
		case kPageDownCharCode:
			if( gPrefs.moveOnlyPaging )			//LR 180 -- option to only move display, not selection point
				ScrollToPosition( dWin, dWin->editOffset + (kBytesPerLine * (dWin->linesPerPage - 1)) );
			else
				_offsetSelection( dWin, kBytesPerLine * (dWin->linesPerPage - 1), (er->modifiers & shiftKey) > 0 );
			break;
		case kHomeCharCode:
dohome:
			_offsetSelection( dWin, 0xFEED, (er->modifiers & shiftKey) > 0 );
			break;
		case kEndCharCode:
doend:
			_offsetSelection( dWin, 0xBED, (er->modifiers & shiftKey) > 0 );
			break;

		//LR 180 -- first, this is useless on read-only files!
		if( dWin->readOnlyFlag )
		{
			ErrorAlert( ES_Stop, errReadOnly );
			return;
		}

		case kClearCharCode:		//LR 180 -- clearing an area is now a seperate command
			{
				long start;
doclear:
				if( dWin->endSel > dWin->startSel )	// can only clear something if it's selected
				{
					// Create a new chunk which will be all zero by default
					EditChunk **tc = NewChunk( dWin->endSel - dWin->startSel, 0, 0, CT_Unwritten );
					if( !tc )
						ErrorAlert( ES_Caution, errMemory );
					else
					{
						(*tc)->lastCtr = 1;	// external chunk

						start = dWin->startSel;

						// now, remember for undo and past this chunk over existing space, then free the memory used
						RememberOperation( dWin, EO_Paste, &gUndo );
						PasteOperation( dWin, tc );
						DisposeChunk( dWin, tc );
						dWin->startSel = dWin->endSel = start;
						ScrollToSelection( dWin, dWin->startSel, false );
					}
				}
				else
					SysBeep(0);		// nothing to clear, signal it!
			}
			break;

		// delete characters
		//LR 1.74 -- non-destructive deletes in overwrite mode (paste appr. lenght zero buffer)
		case kBackspaceCharCode:	// normal delete

			if( er->modifiers & optionKey )				//LR 180 -- option key clears
				goto doclear;

			if( gPrefs.overwrite && gPrefs.nonDestructive )
			{
				if( dWin->endSel == dWin->startSel )	//LR 180 -- non-destructive really is now!
				{
					if( dWin->startSel > 0L )
					{
						ObscureCursor();
						--dWin->startSel;
					}
					else
						SysBeep(0);
				}

				dWin->endSel = dWin->startSel;
				UpdateOnscreen( dWin->oWin.theWin );
			}
			else if( dWin->endSel > dWin->startSel )
			{
				ClearSelection( dWin );
			}
			else if( dWin->startSel > 0L )
			{
				ObscureCursor();
				--dWin->startSel;
				ClearSelection( dWin );
			}
			else
				SysBeep(0);
			break;

		case kDeleteCharCode:	// forward delete
			if( dWin->endSel > dWin->startSel )
				ClearSelection( dWin );
			else if( dWin->startSel > 0L )
			{
				ObscureCursor();
				++dWin->endSel;
				ClearSelection( dWin );
			}
			else
				SysBeep(0);
			break;
		
		// insert/overwrite characters
		default:
			// Insert Ascii Text into Area indicated by dWin->startSel - dWin->endSel
			// Delete Current Selection if > 0
			ObscureCursor();

			if( dWin->editMode == EM_Ascii )
			{
				if( (dWin->endSel != dWin->lastTypePos ||
					dWin->startSel != dWin->lastTypePos) )
					RememberOperation( dWin, EO_Typing, &gUndo );
				if( dWin->endSel > dWin->startSel )
					DeleteSelection( dWin );
				if( gPrefs.overwrite && dWin->startSel < dWin->fileSize - 1 )
				{
					++dWin->endSel;
					DeleteSelection( dWin );
				}
				InsertCharacter( dWin, charCode );
				dWin->lastTypePos = dWin->startSel;
			}
			else
			{
				short	hexVal;

				if( charCode >= '0' && charCode <= '9' )
					hexVal = charCode - '0';
				else if( charCode >= 'A' && charCode <= 'F' )
					hexVal = 0x0A + charCode - 'A';
				else if( charCode >= 'a' && charCode <= 'f' )
					hexVal = 0x0A + charCode - 'a';
				else
				{
					SysBeep( 1 );
					return;
				}

				if( (dWin->endSel != dWin->lastTypePos ||
					dWin->startSel != dWin->lastTypePos) )
				{
					dWin->loByteFlag = false;
					RememberOperation( dWin, EO_Typing, &gUndo );
				}
				if( dWin->endSel > dWin->startSel )
					DeleteSelection( dWin );

				if( dWin->loByteFlag )
				{
					--dWin->startSel;
					DeleteSelection( dWin );
					hexVal = hexVal | ( dWin->lastNybble * kBytesPerLine );
					InsertCharacter( dWin, hexVal );
					dWin->loByteFlag = false;
				}
				else
				{
					if( gPrefs.overwrite && dWin->startSel < dWin->fileSize - 1 )
					{
						++dWin->endSel;
						DeleteSelection( dWin );
					}
					InsertCharacter( dWin, hexVal );
					dWin->lastNybble = hexVal;
					dWin->loByteFlag = true;
				}
				dWin->lastTypePos = dWin->startSel;
			}
			break;
	}
}

/*** CURSOR OFF ***/
void CursorOff( WindowRef theWin )
{
	if( g.cursorFlag )
	{
		g.cursorFlag = false;
		SetPortWindowPort( theWin );
		InvertRect( &g.cursRect );
	}
}

/*** CURSOR ON ***/
void CursorOn( WindowRef theWin )
{
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	long			start;

	if( !g.cursorFlag && dWin->startSel >= dWin->editOffset && dWin->startSel < dWin->editOffset + ( dWin->linesPerPage * kBytesPerLine ) ) 
	{
		g.cursorFlag = true;
		SetPortWindowPort( theWin );

		start = dWin->startSel - dWin->editOffset;

		if( dWin->editMode == EM_Hex )
		{
			g.cursRect.top = kHeaderHeight + LINENUM(start) * kLineHeight;
			g.cursRect.bottom = g.cursRect.top + kLineHeight;
			g.cursRect.left = kDataDrawPos /*kBodyDrawPos + CHARPOS(kStringHexPos - 1)*/ + (COLUMN(start) * kHexWidth) - 2;
			g.cursRect.right = g.cursRect.left + 2;

			InvertRect( &g.cursRect );
		}
		else
		{
			g.cursRect.top = kHeaderHeight + LINENUM(start) * kLineHeight;
			g.cursRect.bottom = g.cursRect.top + kLineHeight;
			g.cursRect.left = kTextDrawPos /*kBodyDrawPos + CHARPOS(kStringTextPos)*/ + (COLUMN(start) * kCharWidth) - 1;
			g.cursRect.right = g.cursRect.left + 2;

			InvertRect( &g.cursRect );
		}
	}
}

#pragma mark -

#if TARGET_API_MAC_CARBON

// SEL: 1.7 - added carbon printing (function rearrangment by LR)

/*------------------------------------------------------------------------------
    Get the printing information from the end user

    Parameters:
        printSession    -   current printing session
        pageFormat      -   a PageFormat object addr
        printSettings   -   a PrintSettings object addr

    Description:
        If the caller passes an empty PrintSettings object, create a new one,
        otherwise validate the one provided by the caller.
        Invokes the Print dialog and checks for Cancel.
        Note that the PrintSettings object is modified by this function.

------------------------------------------------------------------------------*/
static OSStatus _doPrintDialog( PMPrintSession printSession, PMPageFormat pageFormat, PMPrintSettings* printSettings )
{
    OSStatus    status;
    Boolean     accepted;
    UInt32      minPage = 1,
                maxPage = 9999;

    //  In this sample code the caller provides a valid PageFormat reference but in
    //  your application you may want to load and unflatten the PageFormat object
    //  that was saved at PageSetup time.  See LoadAndUnflattenPageFormat below.

    //  Set up a valid PrintSettings object.
    if (*printSettings == kPMNoPrintSettings)
    {
        status = PMCreatePrintSettings(printSettings);

        //  Note that PMPrintSettings is not session-specific, but calling
        //  PMSessionDefaultPrintSettings assigns values specific to the printer
        //  associated with the current printing session.
        if ((status == noErr) && (*printSettings != kPMNoPrintSettings))
            status = PMSessionDefaultPrintSettings(printSession, *printSettings);
    }
    else
        status = PMSessionValidatePrintSettings(printSession, *printSettings,
                    kPMDontWantBoolean);
    //  Set a valid page range before displaying the Print dialog
    if (status == noErr)
        status = PMSetPageRange(*printSettings, minPage, maxPage);

    //  Display the Print dialog.
    if (status == noErr)
    {
        status = PMSessionPrintDialog(printSession, *printSettings, pageFormat,
                    &accepted);
        if (!accepted)
            status = kPMCancel; // user clicked Cancel button
    }

    return( status );
}

/*------------------------------------------------------------------------------
	Print the pages

    Parameters:
        printSession    -   current printing session
        pageFormat      -   a PageFormat object addr
        printSettings   -   a PrintSettings object addr

    Description:
        Assumes the caller provides validated PageFormat and PrintSettings objects.
        Calculates a valid page range and prints each page by calling _drawDump.

------------------------------------------------------------------------------*/
static void _doPrintLoop( PMPrintSession printSession, PMPageFormat pageFormat, PMPrintSettings printSettings, EditWindowPtr dWin )
{
	OSStatus  	status,
	          printError;
	PMRect		pageRect;
	SInt32		startAddr, endAddr, linesPerPage, addr;
	UInt32			realNumberOfPagesinDoc,
	          pageNumber,
	          firstPage,
	          lastPage;

	//  PMGetAdjustedPaperRect returns the paper size taking into account rotation,
	//  resolution, and scaling settings.  Note this is the paper size selected
	//  the Page Setup dialog.  It is not guaranteed to be the same as the paper
	//  size selected in the Print dialog on Mac OS X.
	status = PMGetAdjustedPaperRect(pageFormat, &pageRect);

	//  PMGetAdjustedPageRect returns the page size taking into account rotation,
	//  resolution, and scaling settings.  Note this is the imageable area of the
	//  paper selected in the Page Setup dialog.
	//  DetermineNumberOfPagesInDoc returns the number of pages required to print
	//  the document.
	if (status == noErr)
	{
	  status = PMGetAdjustedPageRect(pageFormat, &pageRect);
	  if (status == noErr)
	  {
			if( dWin->startSel == dWin->endSel )
			{
				startAddr = 0;
				endAddr = dWin->fileSize;
			}
			else
			{
				startAddr = dWin->startSel;
				endAddr = dWin->endSel;
			}

			addr = startAddr;
//LR: 1.7 -fix lpp calculation!			linesPerPage = (pageRect.bottom - TopMargin - (kHeaderHeight + 1)) / kLineHeight;
			linesPerPage = ((pageRect.bottom - pageRect.top) + (kLineHeight / 3) - (kHeaderHeight + kFooterHeight)) / kLineHeight;
			realNumberOfPagesinDoc = (((endAddr - startAddr) / kBytesPerLine) / linesPerPage) + 1;
		}
	}

	//  Get the user's selection for first and last pages
	if (status == noErr)
	{
	  status = PMGetFirstPage(printSettings, &firstPage);
	  if (status == noErr)
	      status = PMGetLastPage(printSettings, &lastPage);
	}

	//  Check that the selected page range does not go beyond the actual
	//  number of pages in the document.
	if (status == noErr)
	{
		if( firstPage > realNumberOfPagesinDoc )
		{
			status = kPMValueOutOfRange;
			PMSessionSetError (printSession, kPMValueOutOfRange);
		}

		if (realNumberOfPagesinDoc < lastPage)
			lastPage = realNumberOfPagesinDoc;
	}

	//  NOTE:  We don't have to worry about the number of copies.  The Printing
	//  Manager handles this.  So we just iterate through the document from the
	//  first page to be printed, to the last.
	if (status == noErr)
	{ //  Establish a graphics context for drawing the document's pages.
	  //  Although it's not used in this sample code, PMGetGrafPtr can be called
	  //  get the QD grafport.
	  status = PMSessionBeginDocument(printSession, printSettings, pageFormat);
	  if (status == noErr)
	  { //  Print all the pages in the document.  Note that we spool all pages
			//  and rely upon the Printing Manager to print the correct page range.
			//  In this sample code we assume the total number of pages in the
			//  document is equal to "lastPage".
			pageNumber = 1;
			while ((pageNumber <= lastPage) && (PMSessionError(printSession) == noErr))
			{
				Rect	r;
				
				//  NOTE:  We don't have to deal with the old Printing Manager's
				//  128-page boundary limit anymore.

				//  Set up a page for printing.
				status = PMSessionBeginPage(printSession, pageFormat, &pageRect);
				if (status != noErr)
				  break;

				//  Draw the page.
				r.top = (short)(pageRect.top);
				r.left = (short)(pageRect.left);
				r.bottom = r.top + kHeaderHeight - 1;
				r.right = (short)(pageRect.right);
				_drawHeader( dWin, &r );

				r.top += kHeaderHeight;
				r.bottom = pageRect.bottom - kFooterHeight;
				_drawDump( dWin, &r, addr, endAddr );

				r.top = r.bottom;
				r.bottom += kFooterHeight;
				_drawFooter( dWin, &r, pageNumber, realNumberOfPagesinDoc );

				//  Close the page.
				status = PMSessionEndPage(printSession);
				if (status != noErr)
				  break;

				addr += linesPerPage * kBytesPerLine;
				addr -= ( addr % kBytesPerLine );

				//  And loop.
				pageNumber++;
			}

			// Close the printing port
			(void)PMSessionEndDocument(printSession);
	  }
	}

	//  Only report a printing error once we have completed the print loop. This
	//  ensures that every PMSessionBegin... call is followed by a matching
	//  PMSessionEnd... call, so the Printing Manager can release all temporary
	//  memory and close properly.
	printError = PMSessionError(printSession);
	if ( ( kPMCancel != printError) && (printError != noErr) )
	  PostPrintingErrors(printError);
}

// NS: v1.6.6, event filters for navigation services

#endif	//TARGET_API_MAC_CARBON  -- BB: moved _navEventFilter from Carbon only

/*** PRINT WINDOW ***/
void PrintWindow( EditWindowPtr dWin )
{
#if TARGET_API_MAC_CARBON	// SEL: 1.7 - carbon printing
	// Carbon session based printing variables 
	OSStatus    		status;
	PMPrintSession	printSession;
	PMPrintSettings	printSettings;
	PMPageFormat		pageFormat;
#else
	Boolean			ok;
	Rect				r;
	TPPrPort		printPort;
	TPrStatus		prStatus;
	short		pageNbr, startPage, endPage, nbrPages;
	long		startAddr, endAddr, addr;
	short		linesPerPage;
#endif

	GrafPtr		savePort;

	GetPort( &savePort );

#if TARGET_API_MAC_CARBON	// SEL: 1.7 - implemented Carbon printing

	// make a new printing session
	status = PMCreateSession(&printSession);
	if ( noErr != status )
	{
		PostPrintingErrors(status);
		return;
	}
	if ( kPMNoPageFormat == g.pageFormat )
	{
		status = PMCreatePageFormat(&pageFormat);
		//  Note that PMPageFormat is not session-specific, but calling
		//  PMSessionDefaultPageFormat assigns values specific to the printer
		//  associated with the current printing session.
		if ((status == noErr) && (pageFormat != kPMNoPageFormat))
		{
			status = PMSessionDefaultPageFormat(printSession, pageFormat);
		}
	}
	else
	{ // already have a pageFormat, prolly because the user selected Page Setup
		status = PMCreatePageFormat(&pageFormat);
		status = PMCopyPageFormat(g.pageFormat, pageFormat);
		if ( noErr == status )
		{
			status = PMSessionValidatePageFormat(printSession, pageFormat, kPMDontWantBoolean);
		}
	}
	if ( noErr != status )
	{
		PostPrintingErrors(status);
		(void)PMRelease(printSession);
		return;
	}
	if ( kPMNoPrintSettings != g.printSettings )
	{
		status = PMCreatePrintSettings(&printSettings);
		status = PMCopyPrintSettings(g.printSettings, printSettings);
	}
	else
	{
		printSettings = kPMNoPrintSettings;
	}
	if ( noErr != status )
	{
		PostPrintingErrors(status);
		(void)PMRelease(pageFormat);
		(void)PMRelease(printSession);
		return;
	}
  //  Display the Print dialog.
	status = _doPrintDialog(printSession, pageFormat, &printSettings);
	if  ( kPMCancel != status )
	{ // user did not cancel the print dialog box
		if ( ( noErr == status ) )
		{ //  Execute the print loop.
			_doPrintLoop(printSession, pageFormat, printSettings, dWin);
		}
		else
		{
			PostPrintingErrors(status);
			return;
		}
	}

	//  Release the PageFormat and PrintSettings objects.  PMRelease decrements the
	//  ref count of the allocated objects.  We let the Printing Manager decide when
	//  to release the allocated memory.
	if (pageFormat != kPMNoPageFormat)
	{
		(void)PMRelease(pageFormat);
	}
	if (printSettings != kPMNoPrintSettings)
	{
		(void)PMRelease(printSettings);
	}
	//  Terminate the current printing session.
	(void)PMRelease(printSession);

#else	// non-Carbon printing

	PrOpen();

	PrValidate( g.HPrint );
	ok = PrJobDialog( g.HPrint );
	if( ok )
	{
		if( dWin->startSel == dWin->endSel )
		{
			startAddr = 0;
			endAddr = dWin->fileSize;
		}
		else
		{
			startAddr = dWin->startSel;
			endAddr = dWin->endSel;
		}

		printPort = PrOpenDoc( g.HPrint, NULL, NULL );

		r = printPort->gPort.portRect;
//LR: 1.7 -fix lpp calculation!		linesPerPage = ( r.bottom - TopMargin - ( kHeaderHeight + 1 ) ) / kLineHeight;
			linesPerPage = ((r.bottom - r.top) + (kLineHeight / 3) - (kHeaderHeight + kFooterHeight)) / kLineHeight;
		nbrPages = ((endAddr - startAddr) / kBytesPerLine) / linesPerPage + 1;

		startPage = ( **g.HPrint ).prJob.iFstPage;
		endPage = ( **g.HPrint ).prJob.iLstPage;
		if( startPage > nbrPages )
		{
			PrCloseDoc( printPort );
			ErrorAlert( ES_Caution, errPrintRange, nbrPages );
			goto ErrorExit;
		}
		addr = startAddr;

		if( endPage > nbrPages )
			endPage = nbrPages;

		ctHdl = NULL;	// print in black & white!

		for ( pageNbr = 1; pageNbr <= nbrPages; ++pageNbr )
		{
			SetPort( &printPort->gPort );
			PrOpenPage( printPort, NULL );
	
			if( pageNbr >= startPage && pageNbr <= endPage )
			{
				r = printPort->gPort.portRect;
				r.bottom = r.top + kHeaderHeight - 1;		//LR: 1.7 - don't erase entire page!
				_drawHeader( dWin, &r );
		
				r.top += kHeaderHeight;
				r.bottom = printPort->gPort.portRect.bottom - kFooterHeight;
				_drawDump( dWin, &r, addr, endAddr );
	
				r.top = r.bottom;
				r.bottom += kFooterHeight;
				_drawFooter( dWin, &r, pageNbr, nbrPages );	//SEL: 1.7 - fix Lane's DrawDump usage (what was I thinking? P)
			}

			addr += linesPerPage * kBytesPerLine;
			addr -= ( addr % kBytesPerLine );
			PrClosePage( printPort );
		}
		PrCloseDoc( printPort );
		if( ( **g.HPrint ).prJob.bJDocLoop == bSpoolLoop && PrError() == noErr )
			PrPicFile( g.HPrint, NULL, NULL, NULL, &prStatus );
	}
ErrorExit:
	PrClose();
#endif
	SetPort( savePort );
}

#pragma mark -

/*** COPY FORK ***/
static OSStatus _copyFork( FSSpec *srcSpec, FSSpec *dstSpec, short forkType )
{
	OSStatus error;
	short	sRefNum, dRefNum;
	Ptr		tBuffer;
	long	srcSize=0L, bufSize, count;

	tBuffer = NewPtr( 32000 );

	if( !tBuffer )
	{
		ErrorAlert( ES_Caution, errMemory );
		return 1;
	}
	if( forkType == FT_Resource )
	{
//LR 175		error = HOpenRF( srcSpec->vRefNum, srcSpec->parID, srcSpec->name, fsRdPerm, &sRefNum );
		error = FSpOpenRF( srcSpec, fsRdPerm, &sRefNum );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errSave, error );
			return error;
		}
//LR 175		error = HOpenRF( dstSpec->vRefNum, dstSpec->parID, dstSpec->name, fsWrPerm, &dRefNum );
		error = FSpOpenRF( dstSpec, fsWrPerm, &dRefNum );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errSave, error );
			return error;
		}
	}
	else
	{
//LR 175		error = HOpen( srcSpec->vRefNum, srcSpec->parID, srcSpec->name, fsRdPerm, &sRefNum );
		error = FSpOpenDF( srcSpec, fsRdPerm, &sRefNum );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errSave, error );
			return error;
		}
//LR 175		error = HOpen( dstSpec->vRefNum, dstSpec->parID, dstSpec->name, fsWrPerm, &dRefNum );
		error = FSpOpenDF( dstSpec, fsWrPerm, &dRefNum );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errSave, error );
			return error;
		}
	}
	GetEOF( sRefNum, &srcSize );
	SetEOF( dRefNum, 0L );
	while( srcSize )
	{
		if( srcSize < 32000 )
			bufSize = srcSize;
		else
			bufSize = 32000;
		srcSize -= bufSize;
		count = bufSize;
		error = FSRead( sRefNum, &count, tBuffer );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errRead, error );
			goto ErrorExit;
		}
		error = FSWrite( dRefNum, &count, tBuffer );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errWrite, error );
			goto ErrorExit;
		}
	}
	error = noErr;
ErrorExit:
	FSClose( sRefNum );
	FSClose( dRefNum );
	DisposePtr( tBuffer );
	return error;
}

/*** SAVE CONTENTS ***/
void SaveContents( WindowRef theWin )
{
	short			tRefNum = 0;
	FSSpec			tSpec, bSpec;
	HParamBlockRec	pb;
	EditChunk		**cc;
	long			count;
	OSStatus		error;
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );

	//LR 180 -- first, this is useless on read-only files!
	if( dWin->readOnlyFlag )
	{
		ErrorAlert( ES_Stop, errReadOnly );
		return;
	}

	// if we don't have a file spec it's a new window, do a Save As...
	if( dWin->destSpec.name[0] == 0 )
	{
		SaveAsContents( theWin );
		return;
	}

	// OK, ready to save ... first create our temp work file
	tSpec = dWin->destSpec;

	// If original file exists, write to temp file
	if( dWin->refNum )
	{
		if( tSpec.name[0] < 31 )
		{
			tSpec.name[0]++;
			tSpec.name[tSpec.name[0]] = '^';	//LR 1.72 -- temp files end with ^
		}
		else	tSpec.name[31] ^= 0x10;
	}
//LR 1.73 -- delete temp files!		_ensureNameIsUnique( &tSpec );

//LR 175		HDelete( tSpec.vRefNum, tSpec.parID, tSpec.name );
	FSpDelete( &tSpec );
//LR 175		error = HCreate( tSpec.vRefNum, tSpec.parID, tSpec.name, dWin->creator, dWin->fileType );
	error = FSpCreate( &tSpec, dWin->creator, dWin->fileType, smSystemScript );
	if( error != noErr )
	{
		ErrorAlert( ES_Caution, errCreate, error );
		return;
	}

	// Preserve creation date of orig file if it exists
	if( dWin->creationDate )
	{
		pb.fileParam.ioCompletion = 0l;
		pb.fileParam.ioNamePtr = tSpec.name;
		pb.fileParam.ioVRefNum = tSpec.vRefNum;
		pb.fileParam.ioDirID = tSpec.parID;
		pb.fileParam.ioFDirIndex = 0;
	
		if( ( error = PBHGetFInfo( &pb, false ) ) != noErr )
		{
			ErrorAlert( ES_Caution, errFileInfo, error );
			return;
		}
		// Reset dirID which PBHGeFInfo changes...
		pb.fileParam.ioFlCrDat = dWin->creationDate;
		pb.fileParam.ioDirID = tSpec.parID;
		if( ( error = PBHSetFInfo( &pb, false ) ) != noErr )
		{
			ErrorAlert( ES_Caution, errSetFileInfo, error );
			return;
		}
	}
	// Preserve other fork if it exists (LR 1.73 can't do !fork because fork #s are now 1 & 2!)
	if( dWin->refNum )
		if( _copyFork( &dWin->fsSpec, &tSpec, (dWin->fork == FT_Data) ? FT_Resource : FT_Data ) != noErr )
			return;

	// Open the temp file
	if( dWin->fork == FT_Resource )
	{
//LR 175			error = HOpenRF( tSpec.vRefNum, tSpec.parID, tSpec.name, fsWrPerm, &tRefNum );
		error = FSpOpenRF( &tSpec, fsWrPerm, &tRefNum );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errSave, error );
			return;
		}
	}
	else
	{
//LR 175			error = HOpenDF( tSpec.vRefNum, tSpec.parID, tSpec.name, fsWrPerm, &tRefNum );
		FSpOpenDF( &tSpec, fsWrPerm, &tRefNum );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errSave, error );
			return;
		}
	}
	
	// Save out to temp file
	cc = dWin->firstChunk;
	while( cc )
	{
		LoadChunk( dWin, cc );
		count = ( *cc ) ->size;
		error = FSWrite( tRefNum, &count, *( *cc ) ->data );
		if( error != noErr )
		{
			// !! Error Message - write error
			FSClose( tRefNum );
			if( error == dskFulErr )
			{
				ErrorAlert( ES_Stop, errDiskFull );
				FSpDelete( &tSpec );
//LR 175					HDelete( tSpec.vRefNum, tSpec.parID, tSpec.name );
			}
			else
				ErrorAlert( ES_Caution, errWrite, error );

			return;
		}
		cc = ( *cc ) ->next;
	}

	// Close temp file
	FSClose( tRefNum );

	// If Original File Exists
	if( dWin->refNum )
	{
		// Close original file
		FSClose( dWin->refNum );

		bSpec = dWin->destSpec;

		// If it exists under current name
//LR 175			if( ( error = HOpen( bSpec.vRefNum, bSpec.parID, bSpec.name, fsRdPerm, &dWin->refNum ) ) == noErr )
		if( ( error = FSpOpenDF( &bSpec, fsRdPerm, &dWin->refNum ) ) == noErr )
		{
			FSClose( dWin->refNum );

			if( gPrefs.backupFlag )
			{
				// Delete last backup file, if it exists
				bSpec = dWin->destSpec;
				if( bSpec.name[0] < 31 )	// LR: 000505 -- don't go beyond 31 chars!
					bSpec.name[0]++;
				bSpec.name[bSpec.name[0]] = '~';
				FSpDelete( &bSpec );
//LR 175					HDelete( bSpec.vRefNum, bSpec.parID, bSpec.name );	// backup files end with ~
	
				// Rename original file to backup name
//LR 175					error = HRename( dWin->destSpec.vRefNum, dWin->destSpec.parID, dWin->destSpec.name, bSpec.name );
				FSpRename( &dWin->destSpec, bSpec.name );
				if( error != noErr )
				{
					// Backup is probably open, just delete original
					ErrorAlert( ES_Caution, errBackup, error );
//LR 175						bSpec = dWin->destSpec;
					FSpDelete( &dWin->destSpec );
//LR 175						HDelete( bSpec.vRefNum, bSpec.parID, bSpec.name );
				}
			}
			else
			{
				// Delete Original if backup flag is false
//LR 175					bSpec = dWin->destSpec;
				FSpDelete( &dWin->destSpec );
//LR 175					HDelete( bSpec.vRefNum, bSpec.parID, bSpec.name );
			}
		}

		/* now, for OS X, we need to get the catalog information for later restoration when saving (file permissions) */

#if !defined(__MC68K__) && !defined(__SC__)
		if( FSSetCatalogInfo && dWin->OKToSetCatInfo )	/* not available in all systems (OS 9 and later only it seems) */
		{
			FSRef ref;

			error = FSpMakeFSRef( &tSpec, &ref );
			if( !error )
			{
				error = FSSetCatalogInfo( &ref, kFSCatInfoSettableInfo, &dWin->catinfo );
			}
		}
#endif
		// Rename temp file to correct name
//LR 175			error = HRename( tSpec.vRefNum, tSpec.parID, tSpec.name, dWin->destSpec.name );
		error = FSpRename( &tSpec, dWin->destSpec.name );
		if( error != noErr )
			ErrorAlert( ES_Stop, errRename, error );
	}

	// Open newly saved file read only
	tSpec = dWin->destSpec;
	if( dWin->fork == FT_Resource )
	{
//LR 175			error = HOpenRF( tSpec.vRefNum, tSpec.parID, tSpec.name, fsRdPerm, &dWin->refNum );
		error = FSpOpenRF( &tSpec, fsRdWrPerm, &dWin->refNum );
		if( error != noErr )
			ErrorAlert( ES_Stop, errSave, error );
	}
	else
	{
//LR 175			error = HOpenDF( tSpec.vRefNum, tSpec.parID, tSpec.name, fsRdPerm, &dWin->refNum );
		error = FSpOpenDF( &tSpec, fsRdWrPerm, &dWin->refNum );
		if( error != noErr )
			ErrorAlert( ES_Stop, errSave, error );
	}

	// Reset Work File
	dWin->fsSpec = dWin->destSpec;
//LR: 1.7		SetWTitle( dWin->oWin.theWin, dWin->fsSpec.name );

	dWin->workBytesWritten = 0L;
	SetEOF( dWin->workRefNum, 0L );

	// Flush the volume
	pb.volumeParam.ioCompletion = NULL;
	pb.volumeParam.ioNamePtr = NULL;
	pb.volumeParam.ioVRefNum = tSpec.vRefNum;
	PBFlushVolSync( ( ParmBlkPtr ) &pb );

	// Clear linked list
	UnloadFile( dWin );

	// Rebuilt linked list
	LoadFile( dWin );

	// Clear Dirty Flag
	dWin->dirtyFlag = false;

	return;
}

/*** SAVE AS CONTENTS ***/
void SaveAsContents( WindowRef theWin )
{
	short id;
	Str255 title;
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );

	// LR: 1.7 - must remove item so that if name changes we can add it back w/new name :)
	GetWTitle( theWin, title );
	id = GetWindowMenuItemID( title );
	if( id )
		DeleteMenuItem( GetMenuHandle(kWindowMenu), id );

#if !defined(__MC68K__) && !defined(__SC__)		//LR 1.73 -- not available for 68K (won't even link!)
	if( g.useNavServices ) // BB: replaced #if TARGET_API_MAC_CARBON	// LR: v1.6
	{
		OSStatus error = noErr;
		NavReplyRecord		reply;
		NavDialogOptions	dialogOptions;
		NavEventUPP			eventProc = NewNavEventUPP( _navEventFilter );

		NavGetDefaultDialogOptions( &dialogOptions );
//LR: 1.7 not w/modified window titles!	GetWTitle( theWin, dialogOptions.savedFileName );
		// 05/10/01 - GAB: copy the ENTIRE name, including the last character (hence, the "+ 1" on the count parameter)
		BlockMoveData( dWin->fsSpec.name, dialogOptions.savedFileName, dWin->fsSpec.name[0] + 1 );
		GetIndString( dialogOptions.message, strPrompt, 1 );	//LR 180 -- modify prompt

		error = NavPutFile( NULL, &reply, &dialogOptions, eventProc, kDefaultFileType, kAppCreator, NULL );
		if( reply.validRecord || !error )
		{
			AEKeyword 	keyword;
			DescType 	descType;
			FSSpec		savedSpec;	
			Size 		actualSize;
			
			error = AEGetNthPtr( &(reply.selection), 1, typeFSS, &keyword, &descType, &savedSpec, sizeof(FSSpec), &actualSize );
			if( !error )
			{
				dWin->destSpec = savedSpec;
				dWin->creationDate = 0;
				dWin->readOnlyFlag = false;
				SaveContents( theWin );
				NavCompleteSave( &reply, kNavTranslateInPlace );
			}
			NavDisposeReply( &reply );
		}
		DisposeNavEventUPP( eventProc );
	}
#if !TARGET_API_MAC_CARBON
	else // BB: only used if not in carbon
#endif
#endif	//POWERPC
#if !TARGET_API_MAC_CARBON
	{
		StandardFileReply	reply;
		EditWindowPtr		dWin = (EditWindowPtr) GetWRefCon( theWin );
		Str63				/*fileName,*/ prompt;

//LR: 1.7 not w/modified window titles!	GetWTitle( theWin, fileName );
		GetIndString( prompt, strPrompt, 1 );
		
		StandardPutFile( prompt, dWin->fsSpec.name, &reply );
		if( reply.sfGood )
		{
			dWin->destSpec = reply.sfFile;
			dWin->creationDate = 0;
			dWin->readOnlyFlag = false;
			SaveContents( theWin );
		}
	}
#endif

	_setWindowTitle( dWin );	// LR: 1.7 - set window title, append window to menu
}

/*** REVERT CONTENTS ***/
void RevertContents( WindowRef theWin )
{
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	long			newEOF;

	// Reset Work File
	dWin->workBytesWritten = 0L;
	SetEOF( dWin->workRefNum, 0L );

	// Clear linked list
	UnloadFile( dWin );

	// Reset EOF
	GetEOF( dWin->refNum, &newEOF );

	dWin->fileSize = newEOF;

	// Rebuilt linked list
	LoadFile( dWin );

	// Reset scroll offset, if necessary
	if( dWin->editOffset > dWin->fileSize - kBytesPerLine * dWin->linesPerPage )
		dWin->editOffset = 0;

	dWin->dirtyFlag = false;	//LR 1.72 -- no longer dirty :)

	//LR 1.72 -- release undo if associated with this window
	if( dWin == gUndo.theWin )
	{
		ReleaseEditScrap( dWin, &gUndo.undoScrap );
		gUndo.type = 0;
	}
	if( dWin == gRedo.theWin )
	{
		ReleaseEditScrap( dWin, &gRedo.undoScrap );
		gRedo.type = 0;
	}

//LR 180	DrawPage( dWin );
	UpdateOnscreen( theWin );
}
