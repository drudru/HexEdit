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

#include "EditWindow.h"
#include "EditRoutines.h"
#include "EditScrollbar.h"
#include "Menus.h"
#include "Prefs.h"
#include "Utility.h"

extern globals g;
extern prefs_t prefs;

// Create a new main theWin using a 'WIND' template from the resource fork

SInt16			CompareFlag = 0;
WindowRef		CompWind1, 
				CompWind2;

HEColorTableHandle ctHdl = NULL;	// LR: global to file, for speed
short fontID;

RGBColor black = { 0, 0, 0 };
RGBColor white = { 0xFFFF, 0xFFFF, 0xFFFF };

/* nav services filters */
pascal void NavEventFilter( NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD );
pascal Boolean NavPreviewFilter( NavCBRecPtr callBackParms, void *callBackUD );
pascal Boolean NavFileFilter( AEDesc* theItem, void* info, void *callBackUD, NavFilterModes filterMode );

/*** LOAD PREFERENCES ***/
/* LR -- removed in 1.5
void LoadPreferences( void )
{
	Handle prefHandle;
	prefHandle = GetResource( PrefResType, PrefResID );
	if( !prefHandle || ResError( void ) ) return;
	BlockMove( *prefHandle, &gPrefs, sizeof( Preferences ) );
	ReleaseResource( prefHandle );
	if( prefs.asciiMode )	gHighChar = 0xFF;
	else					gHighChar = 0x7E;
}
*/

/*** SAVE PREFERENCES ***/
/* LR -- removed in 1.5
void SavePreferences( void )
{
	Handle prefHandle;
	while( ( prefHandle = GetResource( PrefResType, PrefResID ) ) != NULL )
	{
		RemoveResource( prefHandle );
		DisposeHandle( prefHandle );
	}
	prefHandle = NewHandle( sizeof( Preferences ) );
	if( !prefHandle )
		ErrorAlert( ES_Caution, errMemory );
	else
	{
		BlockMove( &gPrefs, *prefHandle, sizeof( Preferences ) );
		AddResource( prefHandle, PrefResType, PrefResID, "\pPreferences" );
		WriteResource( prefHandle );
		ReleaseResource( prefHandle );
	}
}
*/

/*** INITIALIZE EDITOR ***/
void InitializeEditor( void )
{
	CursHandle			cursorHandle = NULL;
#if !TARGET_API_MAC_CARBON	// LR: v1.6
	PScrapStuff			ScrapInfo;
#endif

#if TARGET_API_MAC_CARBON	// LR: v1.6.5
	OSStatus anErr = ClearCurrentScrap();
#else
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
	BitMap qdScreenBits;

	GetQDGlobalsScreenBits( &qdScreenBits );
	g.maxHeight = qdScreenBits.bounds.bottom - qdScreenBits.bounds.top - 24;
}
#else
	g.maxHeight = qd.screenBits.bounds.bottom - qd.screenBits.bounds.top - 24;	// LR: add 'qd.'
#endif

	// LR: v1.6.5 round this to a size showing only full lines
	g.maxHeight = (g.maxHeight / LineHeight) * LineHeight;

// LR: 1.5	if( g.maxHeight < 342 )
// 		g.maxHeight = 342;

#if TARGET_API_MAC_CARBON	// LR: v1.6
// bug: carbon printing not written
#else
	PrOpen();
	g.HPrint = (THPrint) NewHandle( sizeof(TPrint) );
	if( !g.HPrint )
		ErrorAlert( ES_Stop, errMemory );

	PrintDefault( g.HPrint );
	PrClose();
#endif
// LR:	LoadPreferences();
}

/*** CLEANUP EDITOR ***/
void CleanupEditor( void )
{
/*	// LR: keep from locking up
#if TARGET_API_MAC_CARBON	// LR: v1.6
	else CloseWindow( GetDialogWindow( g.searchWin ) );
#else
	else CloseWindow( g.searchWin );
#endif
*/
// LR:	SavePreferences();
	SavePrefs();

	// LR: v1.6.5 now need to dispose of these at exit since they never truly "close"
	if( g.searchWin )
	{
		DisposeDialog( g.searchWin );
		g.searchWin = NULL;
	}

	if( g.gotoWin )
	{
		DisposeDialog( g.gotoWin );
		g.gotoWin = NULL;
	}
}

/*** SETUP NEW EDIT WINDOW ***/
OSStatus SetupNewEditWindow( EditWindowPtr dWin, StringPtr title )
{
	WindowRef theWin;
	ObjectWindowPtr objectWindow;
	Rect r;
	int l;

	theWin = InitObjectWindow( MainWIND, (ObjectWindowPtr) dWin, false );
	if( !theWin )
		ErrorAlert( ES_Stop, errMemory );

	// LR:	Hack for comparing two files
	if( CompareFlag == 1 )
	{
		SetRect( &r, 0, 0, MaxWindowWidth /*+ ( SBarSize-1 ) */, g.maxHeight/2 - 64 );
		CompWind1 = theWin;
	}
	else if( CompareFlag == 2 )
	{
		SetRect( &r, 0, 0, MaxWindowWidth /*+ ( SBarSize-1 ) */, g.maxHeight/2 - 64 );
		MoveWindow( theWin, 14, g.maxHeight/2, true );
		CompWind2 = theWin;
	}
	else
		SetRect( &r, 0, 0, MaxWindowWidth /*+ ( SBarSize-1 ) */, g.maxHeight - 64 );

	// Check for best window size
	if( dWin->fileSize && ( dWin->linesPerPage-1 ) * SBarSize > dWin->fileSize )
		r.bottom -= LineHeight * ( ( ( dWin->linesPerPage-1 ) * SBarSize ) - dWin->fileSize ) / SBarSize;
	if( r.bottom < SBarSize + TopMargin + BotMargin + HeaderHeight + ( LineHeight * 3 ) )
		r.bottom = SBarSize + TopMargin + BotMargin + HeaderHeight + ( LineHeight * 3 );

	SizeWindow( theWin, r.right, r.bottom, true );

// LR: 1.5	SetRect( &r, 0, 0, MaxWindowWidth /*+ ( SBarSize-1 ) */, g.maxHeight - 44 );
// LR: 1.5	OffsetRect( &r, 14, 44 );
// LR: 1.5 zoom rect just modified to correct width.
#if TARGET_API_MAC_CARBON	// LR: v1.6
// bug: how to handle zoom init?
#else
	( ( WStateData * ) * ( (WindowPeek)theWin )->dataHandle )->stdState.right = ( ( WStateData * ) * ( (WindowPeek)theWin )->dataHandle )->stdState.left + MaxWindowWidth;
#endif

	// LR: 1.66 make sure the name is good (for instance "icon/r" is bad!)
	l = (int)title[0];
	while( l )
	{
		if( title[l--] < ' ' )	// truncate name at fist bad char
			title[0] = l;
	}

	SetWTitle( theWin, title );
	
	// NS: 1.6.6 add window to window menu
	AppendMenu( GetMenuHandle(kWindowMenu), title );
	
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
	
// LR: 1.5	SetRect( &offRect, 0, 0, MaxWindowWidth, g.maxHeight );

	if( prefs.useColor )
		dWin->csResID = prefs.csResID;	// LR: 1.5 - color selection
	else
		dWin->csResID = -1;	// LR: if created w/o color then offscreen is 1 bit, NO COLOR possible!

	// Make it the current grafport
	SetPortWindowPort( theWin );
	
/* LR: 1.5 -- this was bad to begin with!
	( *dWin->offscreen->portPixMap )->rowBytes = (((MaxWindowWidth -1) / 32) + 1) * 4;
	( *dWin->offscreen->portPixMap )->baseAddr = NewPtrClear( (long) ( *dWin->offscreen->portPixMap )->rowBytes * g.maxHeight );
	if( !( *dWin->offscreen->portPixMap )->baseAddr )
*/
	dWin->offscreen = NewCOffScreen( MaxWindowWidth, g.maxHeight );
	if( !dWin->offscreen )
			ErrorAlert( ES_Stop, errMemory );

// LR: 1/5	( *dWin->offscreen->portPixMap )->bounds = offRect;
	
	// Show the theWin
	ShowWindow( theWin );

	SetupScrollBars( dWin );

	GetWindowPortBounds( theWin, &r );
	dWin->linesPerPage = ( r.bottom - TopMargin - BotMargin - ( HeaderHeight-1 ) ) / LineHeight + 1;
	dWin->startSel = dWin->endSel = 0L;
	dWin->editMode = EM_Hex;

	return noErr;
}

/* NS: v1.6.6, GWorld creation moved to where it is used */

/*
	Handle creation and use of offscreen bitmaps
	=================================================
	22-Sep-00 LR: GWorlds only now (courtesy of Nick Shanks)
	21-Jun-94 LR: attach palette to offscreen pixmap
	24-Apr-94 LR: Shit, it was working, color table was messed up!
	22-Apr-94 LR: ok, no GWorlds, how about CGrafPtrs?
	20-Apr-94 LR: creation ( w/GWorlds )
*/

// LR: used to force non-color offscreens ( 1/8 to 1/32 the size! )
// NS: not used // short PIXELBITS = 1;	// 8, 16, 32

/*** NEW OFFSCREEN GWORLD ***/
GWorldPtr NewCOffScreen( short width, short height )
{
	OSStatus	error = noErr;
	GWorldPtr	theGWorld = NULL;
	Rect		rect;
	
	SetRect( &rect, 0, 0, width, height );
	error = NewGWorld( &theGWorld, prefs.useColor? 0:1, &rect, NULL, NULL, keepLocal );
	if( error != noErr ) return NULL;
	return theGWorld;
}

/*** NEW EDIT WINDOW ***/
void NewEditWindow( void )
{
	EditWindowPtr		dWin;
	OSStatus				error;
	short				refNum = NULL;
	Point				where = { -1, -1 };
	FSSpec				workSpec;
// LR: 1.5	Rect				r, offRect;

	// Get the Template & Create the Window, it is set up in the resource fork
	// to not be initially visible 

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
	BlockMove( "\pUntitledw", workSpec.name, 10 );
	EnsureNameIsUnique( &workSpec );
	HCreate( workSpec.vRefNum, workSpec.parID, workSpec.name, kAppCreator, '????' );
	if( error != noErr )
	{
		ErrorAlert( ES_Caution, errCreate, error );
		return;
	}
	error = HOpen( workSpec.vRefNum, workSpec.parID, workSpec.name, fsRdWrPerm, &refNum );
	if( error != noErr )
	{
		ErrorAlert( ES_Caution, errOpen, error );
		return;
	}

	dWin->workSpec = workSpec;
	dWin->workRefNum = refNum;
	dWin->workBytesWritten = 0L;

	dWin->fileType = kDefaultFileType;
	dWin->creator = kAppCreator;
	dWin->creationDate = 0L;

	SetupNewEditWindow( dWin, "\pUntitled" );	// LR: 1.5 -make mashortenence easier!

	dWin->firstChunk = NewChunk( 0L, 0L, 0L, CT_Unwritten );
	dWin->curChunk = dWin->firstChunk;
}

#define DataItem		11
#define RsrcItem		12
#define SmartItem		13

#if !TARGET_API_MAC_CARBON		// standard file callbacks not applicable with carbon

/*** SOURCE DLOG HOOK ***/
pascal short SourceDLOGHook( short item, DialogPtr theDialog )
{
	switch( item )
	{
		case DataItem:
		case RsrcItem:
		case SmartItem:
			SetControl( theDialog, g.forkMode + DataItem, false );
			g.forkMode = item - DataItem;
			SetControl( theDialog, g.forkMode + DataItem, true );
			return sfHookNullEvent;	/* Redraw the List */
		
		case sfHookFirstCall:
			SetControl( theDialog, g.forkMode + DataItem, true );
			return sfHookNullEvent;
	}
	return item;
}

/*** SOURCE DLOG FILTER ***/
pascal Boolean SourceDLOGFilter( DialogPtr dlg, EventRecord *event, short *item )
{
	Str63		prompt;

	if( activateEvt == event->what )
	{
		GetIndString( prompt, strPrompt, CompareFlag+2 );	// LR: v1.6.5 localizable way!
		SetText( dlg, 14, prompt );
	}

	return StdFilterProc( dlg, event, item );
}

#endif

/*** ASK EDIT WINDOW ***/
short AskEditWindow( void )
{
#if TARGET_API_MAC_CARBON	// LR: v1.6
	OSStatus error = noErr;
	NavReplyRecord		reply;
	NavDialogOptions	dialogOptions;
 	NavEventUPP			eventProc = NewNavEventUPP( NavEventFilter );
	NavPreviewUPP		previewProc = NULL;
	NavObjectFilterUPP	filterProc = NULL;
	NavTypeListHandle	openTypeList = NULL;
	
	NavGetDefaultDialogOptions( &dialogOptions );
	dialogOptions.dialogOptionFlags += kNavNoTypePopup;
	error = NavGetFile( NULL, &reply, &dialogOptions, eventProc, previewProc, filterProc, openTypeList, NULL);
	if( reply.validRecord || !error )
	{
		AEKeyword 	keyword;
		DescType 	descType;
		FSSpec		openedSpec;	
		Size 		actualSize;
		
		error = AEGetNthPtr( &(reply.selection), 1, typeFSS, &keyword, &descType, &openedSpec, sizeof(FSSpec), &actualSize );
		if( !error ) OpenEditWindow( &openedSpec, true );
		NavDisposeReply( &reply );
	}
	else error = ioErr;		// user cancelled
	DisposeNavEventUPP( eventProc );
	AdjustMenus();
	return error == noErr? 0 : -1;
#else
	SFReply		macSFReply;
	FSSpec		fSpec;
	long		procID;
	Point		where = { -1, -1 };

	DlgHookUPP myGetFileUPP;		// LR: v1.6.5 limited to this routine
	ModalFilterUPP myFilterUPP;

	myGetFileUPP = NewDlgHookProc( SourceDLOGHook );
	myFilterUPP = NewModalFilterProc( SourceDLOGFilter );

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


	SFPGetFile( where, NULL, NULL, -1, NULL, myGetFileUPP, &macSFReply, dlgGetFile, myFilterUPP );

	DisposeDlgHookUPP( myGetFileUPP );
	DisposeModalFilterUPP( myFilterUPP );

	if( macSFReply.good )
	{
		BlockMove( macSFReply.fName, fSpec.name, macSFReply.fName[0]+1 );
		GetWDInfo( macSFReply.vRefNum, &fSpec.vRefNum, &fSpec.parID, &procID );
		OpenEditWindow( &fSpec, true );
	}
	else return -1;

	AdjustMenus();
	return 0;
#endif
}

/*** OPEN EDIT WINDOW ***/
OSStatus OpenEditWindow( FSSpec *fsSpec, Boolean showerr )
{
// LR: 1.5	WindowRef	theWin;
	EditWindowPtr		dWin;
	OSStatus			error;
	short				refNum=0, redo = false;
	Point				where={-1, -1};
	HParamBlockRec		pb;
	FSSpec				workSpec;
// LR: 1.5 	Rect		r, offRect;
	
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
		return error;
	}

	// fileSize = pb.fileParam.ioFlLgLen;	// Data Fork Length!!!!
	// GetEOF( refNum, &fileSize );
	
	dWin = (EditWindowPtr) NewPtrClear( sizeof(EditWindowRecord) );
	if( !dWin )
	{
		error = MemError();
		FSClose( refNum );
		if( showerr )
			ErrorAlert( ES_Caution, errMemory );
		return error;
	}
	
	if( g.forkMode == FM_Data || (pb.fileParam.ioFlLgLen > 0 && g.forkMode == FM_Smart) )
	{
		// Open Data Fork
		dWin->fork = FT_Data;
		error = HOpenDF( fsSpec->vRefNum, fsSpec->parID, fsSpec->name, fsRdPerm, &refNum );
		if( error == fnfErr )
		{
			if( !showerr ) return error;
			ParamText( fsSpec->name, "\pDATA", NULL, NULL );
			if( CautionAlert( alertNoFork, NULL ) != 2 )
				return error;

			error = HCreate( fsSpec->vRefNum, fsSpec->parID, fsSpec->name, 
						pb.fileParam.ioFlFndrInfo.fdCreator, 
						pb.fileParam.ioFlFndrInfo.fdType );

			if( error != noErr )
			{
				ErrorAlert( ES_Caution, errCreate, error );
				return error;
			}
			error = HOpen( fsSpec->vRefNum, fsSpec->parID, fsSpec->name, fsRdPerm, &refNum );
		}
		if( error != noErr )
		{
			if( showerr )
				ErrorAlert( ES_Caution, errOpen, error );
			return error;
		}
		dWin->fileSize = pb.fileParam.ioFlLgLen;
	}
	else
	{
		// Open Resource Fork
		dWin->fork = FT_Resource;
		error = HOpenRF( fsSpec->vRefNum, fsSpec->parID, fsSpec->name, fsRdPerm, &refNum );
		if( error == fnfErr )
		{
			if( !showerr )
				return error;

			ParamText( fsSpec->name, "\pRSRC", NULL, NULL );
			if( CautionAlert( alertNoFork, NULL ) != 2 )
				return error;

			HCreateResFile( fsSpec->vRefNum, fsSpec->parID, fsSpec->name );
			if( ( error = ResError() ) != noErr )
			{
				ErrorAlert( ES_Caution, errCreate, error );
				return error;
			}
			error = HOpenRF( fsSpec->vRefNum, fsSpec->parID, fsSpec->name, fsRdPerm, &refNum );
		}
		if( error != noErr )
		{
			if( showerr )
				ErrorAlert( ES_Caution, errOpen, error );
			return error;
		}
		dWin->fileSize = pb.fileParam.ioFlRLgLen;
	}

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
		workSpec.name[workSpec.name[0]] = '~';
	}
	else
		workSpec.name[31] ^= 0x10;

	EnsureNameIsUnique( &workSpec );
	error = HCreate( workSpec.vRefNum, workSpec.parID, workSpec.name, kAppCreator, '????' );
	if( error != noErr )
	{
		if( showerr )
			ErrorAlert( ES_Caution, errCreate, error );
		return error;
	}
	redo = false;

	error = HOpen( workSpec.vRefNum, workSpec.parID, workSpec.name, fsRdWrPerm, &refNum );
	if( error != noErr )
	{
		if( showerr )
			ErrorAlert( ES_Caution, errOpen, error );
		return error;
	}

	dWin->workSpec = workSpec;
	dWin->workRefNum = refNum;
	dWin->workBytesWritten = 0L;

	dWin->fileType = pb.fileParam.ioFlFndrInfo.fdType;
	dWin->creator = pb.fileParam.ioFlFndrInfo.fdCreator;
	dWin->creationDate =	pb.fileParam.ioFlCrDat;

	dWin->fsSpec = *fsSpec;
	dWin->destSpec = dWin->fsSpec;

	error = SetupNewEditWindow( dWin, fsSpec->name );	// LR: 1.5 -make maintenence easier!
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
		HDelete( dWin->workSpec.vRefNum, dWin->workSpec.parID, dWin->workSpec.name );
	}
	DisposeGWorld( dWin->offscreen );
	DefaultDispose( theWin );
	AdjustMenus();
}

/*** CLOSE EDIT WINDOW ***/
Boolean	CloseEditWindow( WindowRef theWin )
{
	short			i, n;
	Str63			fileName;
	Str255			windowName, menuItemTitle;
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	MenuRef			windowMenu;

	MySetCursor( C_Arrow );

	if( dWin->dirtyFlag )
	{
		GetWTitle( theWin, fileName );
		ParamText( fileName, NULL, NULL, NULL );
		switch( NoteAlert( alertSave, NULL ) )
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
	
	((ObjectWindowPtr)dWin)->Dispose( theWin );

	return true;
}

/*** CLOSE ALL EDIT WINDOWS ***/
Boolean CloseAllEditWindows( void )
{
	WindowRef next, theWin = FrontWindow();

	while( theWin )
	{
		long windowKind = GetWindowKind( theWin );

		next = GetNextWindow( theWin );

		if( (DialogPtr)theWin == g.searchWin )
		{
			DisposeDialog( g.searchWin );
			g.searchWin = NULL;
		}
		else if( windowKind == HexEditWindowID )
			if( !CloseEditWindow( theWin ) )
				return false;

		theWin = next;
	}

	return true;
}

// Locate Edit Window	( LR 951121 )
// 
// ENTRY:	File's refnum of Edit theWin to find, and fork open ( -1 == ignore fork )
// 	EXIT:	ptr to edit theWin, or NULL if not found

EditWindowPtr LocateEditWindow( FSSpec *fs, short fork )
{
	WindowRef theWin = FrontWindow();

	while( theWin )
	{
		if( HexEditWindowID == GetWindowKind( theWin ) )	// LR: v1.6.5 fix search
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

/*** FIND FIRST EDIT WINDOW ***/
EditWindowPtr FindFirstEditWindow( void )
{
	WindowRef theWin, editWin = NULL;

	// Find and Select Top Window
	//LR: 1.66 total re-write to avoid null window references!

	theWin = FrontWindow();
	if( theWin ) do
	{
		if( GetWindowKind( theWin ) == HexEditWindowID )
			editWin = theWin;

		theWin = GetNextWindow( theWin );

	}while( theWin && !editWin  );

	if( !editWin )
		return( NULL );

	return( (EditWindowPtr)GetWRefCon( editWin ) );
}

/*** INIT COLOUR TABLE ***/
OSStatus InitColorTable( HEColorTablePtr ct )
{
	ct->header.red = ct->header.green = ct->header.blue =	// default light grey scheme
	ct->bar.red = ct->bar.green = ct->bar.blue = 0xCFFF;
	ct->headerLine.red = ct->headerLine.green = ct->headerLine.blue =
	ct->barLine.red = ct->barLine.green = ct->barLine.blue = 0x7FFF;
	ct->headerText = ct->barText = ct->text = black;
	ct->body = white;
	return noErr;
}

/*** GET COLOUR INFO ***/
OSStatus GetColorInfo( EditWindowPtr dWin )
{
	GetFNum( "\pMonaco", &fontID );		// LR: handle using color schemes

	// NS: 1.3, set hilight colour
/*	Handle vars = dWin->oWin.theWin.port.grafVars;
	RGBColor hilightColour = (** (GVarHandle) vars).rgbHiliteColor;
	RGBColor rgbBlack = { nixBlack, nixBlack, nixBlack };
	RGBForeColor( &hilightColour );
*/	
	if( prefs.useColor && dWin->csResID > 0 )
	{
		ctHdl = (HEColorTableHandle) GetResource( 'HEct', dWin->csResID );
		if( !ctHdl )
		{
			ctHdl = (HEColorTableHandle) NewHandle( sizeof(HEColorTable_t) );
			if( ctHdl ) InitColorTable( *ctHdl );
		}
	}
	else ctHdl = NULL;	// LR: all that's needed to be nonQD compatible?
	return noErr;
}

/*** DRAW HEADER ***/
void DrawHeader( EditWindowPtr dWin, Rect *r )
{
	char str[256];

	GetColorInfo( dWin );	// LR: v1.6.5 ensure that updates are valid!

	// LR: if we have color table, fill in the address bar!
	if( ctHdl )
	{
		RGBBackColor( &( *ctHdl )->header );
		RGBForeColor( &( *ctHdl )->headerLine );
	}

	EraseRect( r );

	if( ctHdl )	// LR: v1.6.5 LR -- top line darker, bottom lighter (by default)
	{
		MoveTo( r->left, r->bottom - 1 );
		LineTo( r->right, r->bottom - 1 );

		RGBForeColor( &( *ctHdl )->barLine );
	}

	r->right -= SBarSize;	// LR: v1.6.5 don't overwrite scroll bar icon

	MoveTo( r->left, r->bottom - 0 );
	LineTo( r->right, r->bottom - 0 );

	if( ctHdl )
		RGBForeColor( &( *ctHdl )->headerText );

	TextFont( fontID );
	TextSize( 9 );
	TextFace( normal );	// LR: v1.6.5 LR - can't be bold 'cause then the new stuff doesn fit :0
	TextMode( srcCopy );

	// LR: v1.6.5 LR - more stuff in the header now & strings are from an localizable resource :)

	/* NS note to self:	%8	is length of eight (pads with spaces)
						%08	is length of eight (pads with zeros)
						%l	makes the number a long
						%d	identifies as a number (decimal)
						%X	identifies as a number (hexadecimal)		*/
	
	GetIndString( (StringPtr) str, strHeader, prefs.decimalAddr? 1:2 );
	CopyPascalStringToC( (StringPtr) str, str );
	sprintf( (char *) g.buffer, str, dWin->fileSize, &dWin->fileType, &dWin->creator, (dWin->fork == FT_Data? "data" : "rsrc"), dWin->startSel, dWin->endSel );
	MoveTo( 5, r->top + HeaderHeight - DescendHeight - 5 );
	DrawText( g.buffer, 0, strlen( (char *) g.buffer ) );
	
	if( ctHdl )
	{
		RGBForeColor( &black );
		RGBBackColor( &white );
	}
}

/*** DrawFooter ***/
void DrawFooter( EditWindowPtr dWin, Rect *r, short pageNbr, short nbrPages )
{
//	only used when printing
	unsigned long	tim;
	DateTimeRec		dat;
	Str31			fileName;

	TextFont( fontID );
	TextSize( 9 );
	TextFace( normal );
	TextMode( srcCopy );

	GetDateTime( &tim );
	SecondsToDate( tim, &dat );

	MoveTo( r->left, r->top );
	LineTo( r->right, r->top );

	sprintf( (char *) g.buffer, "%02d/%02d/%02d %02d:%02d", dat.month, dat.day, dat.year-1900, dat.hour, dat.minute );

	MoveTo( 20, r->top+FooterHeight-DescendHeight-2 );
	DrawText( g.buffer, 0, strlen( (char *) g.buffer ) );

	GetWTitle( dWin->oWin.theWin, fileName );
	sprintf( (char *) g.buffer, "File: %.*s", fileName[0], &fileName[1] );
	MoveTo( ( r->left+r->right )/2 - TextWidth( g.buffer, 0, strlen( (char *) g.buffer ) )/2, r->top+FooterHeight-DescendHeight-2 );
	DrawText( g.buffer, 0, strlen( (char *) g.buffer ) );
	
	sprintf( (char *) g.buffer, "%d of %d", pageNbr, nbrPages );
	MoveTo( r->right - TextWidth( g.buffer, 0, strlen( (char *) g.buffer ) ) - 8, r->top+FooterHeight-DescendHeight-2 );
	DrawText( g.buffer, 0, strlen( (char *) g.buffer ) );
}

/*** DRAW DUMP ***/
OSStatus DrawDump( EditWindowPtr dWin, Rect *r, long sAddr, long eAddr )
{
//	draws the actual hex/decimal and ASCII panes in the window
	short	i, j, y, x;
	short	hexPos;
	short	asciiPos;
	register short	ch, ch1, ch2;
	long	addr;
	Rect br = *r;

	TextFont( fontID );
	TextSize( 9 );
	TextFace( normal );
	TextMode( srcCopy );
// 	TextMode( srcOr );	// LR: don't overwrite our cool colors! // bug: could be improved for speed

	x = StringWidth( "\p 000000: " );

	br.top -= SBarSize;
	br.right = br.left + x + 1;

	// Draw left edging?
	if( ctHdl )
	{
		RGBBackColor( &( *ctHdl )->bar );
		RGBForeColor( &( *ctHdl )->barLine );

		EraseRect( &br );

		MoveTo( br.right, br.top );
		LineTo( br.right, br.bottom );
	}

	addr = sAddr - ( sAddr % 16 );

	g.buffer[57] = g.buffer[58] = g.buffer[75] = ' ';

	for( y = r->top, j = 0; y < r->bottom && addr < eAddr; y += LineHeight, j++ )
	{
		if( prefs.decimalAddr )
			sprintf( (char *) g.buffer, "%7ld:", addr );
		else
			sprintf( (char *) g.buffer, "%06lX:", addr );

		if( ctHdl )
		{
			RGBBackColor( &( *ctHdl )->bar );
			RGBForeColor( &( *ctHdl )->barText );
		}

		MoveTo( AddrPos, y );
		DrawText( g.buffer, 0, 8 );

		if( ctHdl )
		{
			if( !( j & 1 ) )
			{
				RGBBackColor( &( *ctHdl )->body );
				RGBForeColor( &( *ctHdl )->text );
			}
		}

		hexPos = HexStart;
		asciiPos = AsciiStart;

		for( i = 16; i; --i, ++addr )
		{
			if( addr >= sAddr && addr < eAddr )
			{
				ch = GetByte( dWin, addr );
				ch1 = ch2 = ch;
				ch1 >>= 4;
				ch2 &= 0x0F;
				g.buffer[hexPos++] = ch1 + (( ch1 < 10 )? '0' : ( 'A'-10 ));
				g.buffer[hexPos++] = ch2 + (( ch2 < 10 )? '0' : ( 'A'-10 ));
				g.buffer[hexPos++] = ' ';
				g.buffer[asciiPos++] = ((ch >= 0x20 && ch <= g.highChar)? ch:'.');
			}
			else
			{
				g.buffer[hexPos++] = ' ';
				g.buffer[hexPos++] = ' ';
				g.buffer[hexPos++] = ' ';
				g.buffer[asciiPos++] = ' ';
			}
		}
		g.buffer[HexStart-1] = ' ';

		MoveTo( AddrPos + x - 6, y );
		DrawText( g.buffer, HexStart-1, 75-( HexStart-2 ) );
	}

	// Draw vertical bars?
	// based on David Emme's vertical bar mod
	if( prefs.vertBars )
	{
		short w = StringWidth( "\p0" );	// get width of a character (for positioning)

		if( ctHdl )
			RGBForeColor( &( *ctHdl )->headerLine );	// LR: v1.6.5 LR - was barText

		MoveTo( 22 * w, br.top );
		LineTo( 22 * w, br.bottom );

		MoveTo( 34 * w, br.top );
		LineTo( 34 * w, br.bottom );

		MoveTo( 46 * w, br.top );
		LineTo( 46 * w, br.bottom );
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
void DrawPage( EditWindowPtr dWin )
{
	GrafPtr			savePort;
	Rect			r;
// LR: 1.5 offscreen fix!	BitMap			realBits;

#if PROFILE
	_profile = 1;
#endif

	GetColorInfo( dWin );	// LR: v1.6.5 multiple routines need this

// LR: offscreen fix!	realBits = ( ( GrafPtr ) dWin )->portBits;
// LR: offscreen fix!	SetPortBits( ( BitMap * ) &dWin->pixMap );	// LR: -- pixmap change

	GetPort( &savePort );
	SetPort( ( GrafPtr )dWin->offscreen );

	GetPortBounds( dWin->offscreen, &r );
	r.right -= ( SBarSize - 1 );

/* LR: 1.5 - no longer possible
	// don't draw outside our offscreen pixMap!
	if( r.right - r.left > ( *dWin->offscreen->portPixMap )->bounds.right - ( *dWin->offscreen->portPixMap )->bounds.left ||
		r.bottom - r.top > ( *dWin->offscreen->portPixMap )->bounds.bottom - ( *dWin->offscreen->portPixMap )->bounds.top )
	{
// 		DebugStr( "\pOy!" );
		return;
	}
*/

// LR: 1.5 now done in theWin update!	DrawHeader( dWin, &r );

	r.top += HeaderHeight;		// NS: don't erase header
// LR: 1.5	r.bottom -= ( SBarSize - 1 );

	if( ctHdl )
		RGBBackColor( &( *ctHdl )->body );
	else
	{
		ForeColor( blackColor );
		BackColor( whiteColor );
	}

	EraseRect( &r );

	r.top += ( TopMargin + LineHeight - DescendHeight );
// NS: no bottom bar anymore	r.bottom -= ( SBarSize + DescendHeight + BotMargin );
// 	r.bottom -= ( DescendHeight + BotMargin );

	DrawDump( dWin, &r, dWin->editOffset, dWin->fileSize );

	if( ctHdl )
	{
		RGBForeColor( &black );
		RGBBackColor( &white );
	}

// LR: offscreen fix!	SetPortBits( &realBits );
	SetPort( savePort );

#if PROFILE
	_profile = 0;
#endif
}

// Respond to an update event - BeginUpdate has already been called.

/*** MY DRAW ***/
void MyDraw( WindowRef theWin )
{
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	DrawControls( theWin );
	DrawGrowIcon( theWin );

	// EraseRect( &theWin->portRect );
	DrawPage( dWin );
	UpdateOnscreen( theWin );
}

/*** UPDATE ONSCREEN ***/
void UpdateOnscreen( WindowRef theWin )
{
	Rect			r1, r2, r3;
	GrafPtr			oldPort;
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	PixMapHandle thePixMap = GetPortPixMap( dWin->offscreen );

	GetPortBounds( dWin->offscreen, &r1 );
	GetWindowPortBounds( theWin, &r2 );

	GetPort( &oldPort );
	SetPortWindowPort( theWin );

	g.cursorFlag = false;

	// LR: Header drawn here due to overlapping vScrollBar
	r2.bottom = r2.top + HeaderHeight - 1;
	DrawHeader( dWin, &r2 );

	// LR: adjust for scrollbar & header
	GetWindowPortBounds( theWin, &r2 );
	r2.right -= SBarSize - 1;
	r2.top += (HeaderHeight);

	SectRect( &r1, &r2, &r3 );

// LR: offscreen fix!	CopyBits( ( BitMap * ) &dWin->pixMap, &theWin->portBits, &r3, &r3, srcCopy, 0L );	// LR: -- PixMap change
#if TARGET_API_MAC_CARBON	// LR: v1.6
	CopyBits( GetPortBitMapForCopyBits( dWin->offscreen ), GetPortBitMapForCopyBits( GetWindowPort( theWin ) ), &r3, &r3, srcCopy, 0L );
#else
	CopyBits( ( BitMap * ) &( dWin->offscreen )->portPixMap, &theWin->portBits, &r3, &r3, srcCopy, 0L );
#endif

	if( dWin->endSel > dWin->startSel &&	dWin->endSel >= dWin->editOffset && dWin->startSel < dWin->editOffset+( dWin->linesPerPage<<4 ) ) 
		InvertSelection( dWin );

	SetPort( oldPort );
}

/*** MY IDLE ***/
void MyIdle( WindowRef theWin, EventRecord *er )
{
	EditWindowPtr	dWin = (EditWindowPtr)GetWRefCon( theWin );

// LR: v1.6.5	long			scrapCount;
	Boolean			frontWindowFlag;
	Point			w;

	frontWindowFlag = (theWin == FrontWindow() && dWin->oWin.active);
	if( frontWindowFlag )
	{
		w = er->where;

		SetPortWindowPort( theWin );

		GlobalToLocal( &w );
		if( w.v >= HeaderHeight+TopMargin && 
			w.v < HeaderHeight+TopMargin+( dWin->linesPerPage*LineHeight ) )
		{
				if( w.h >= AddrPos+CharPos( HexStart ) &&
					w.h < AddrPos+CharPos( HexStart )+( HexWidth<<4 ) ) 
					MySetCursor( C_IBeam );
				else if( w.h >= AddrPos+CharPos( AsciiStart ) &&
						 w.h < AddrPos+CharPos( AsciiStart )+( CharWidth<<4 ) )
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
	if( MyHandleControlClick( theWin, w ) )
		return;
	// Else handle editing chore
	CursorOff( theWin );
	if( w.v >= HeaderHeight+TopMargin && w.v < HeaderHeight+TopMargin+( dWin->linesPerPage*LineHeight ) )
	{
		do
		{
			AutoScroll( dWin, w );

			if( w.h >= AddrPos+CharPos( HexStart ) &&
				w.h < AddrPos+CharPos( HexStart )+( HexWidth<<4 ) ) 
			{

				pos = ( ( w.v - ( HeaderHeight+TopMargin ) )/LineHeight ) * SBarSize +
						( w.h - ( AddrPos+CharPos( HexStart ) )+12 ) / HexWidth;
				dWin->editMode = EM_Hex;
			}
			else if( w.h >= AddrPos+CharPos( AsciiStart ) &&
					 w.h < AddrPos+CharPos( AsciiStart )+( CharWidth<<4 ) )
			{
				pos = ( ( w.v - ( HeaderHeight+TopMargin ) )/LineHeight ) * SBarSize +
						( w.h - ( AddrPos+CharPos( AsciiStart ) )+3 ) / CharWidth;
				dWin->editMode = EM_Ascii;
			}
			else
				goto GetMouseLabel;

			pos += dWin->editOffset;
			if( pos < dWin->editOffset )
				pos = dWin->editOffset;
			if( pos > dWin->editOffset + ( dWin->linesPerPage << 4 ) )
				pos = dWin->editOffset + ( dWin->linesPerPage << 4 );
			if( pos > dWin->fileSize )
				pos = dWin->fileSize;
			if( anchorPos == -1 )
			{
				if( er->modifiers & shiftKey )
					anchorPos = ( pos < dWin->startSel )? dWin->endSel : dWin->startSel;
				else anchorPos = pos;
			}
			sPos = pos < anchorPos? pos : anchorPos;
			ePos = pos > anchorPos? pos : anchorPos;
			if( ePos > dWin->fileSize )
				ePos = dWin->fileSize;

			if( sPos != dWin->startSel || ePos != dWin->endSel )
			{
				dWin->startSel = sPos;
				dWin->endSel = ePos;
				UpdateOnscreen( theWin );
			}

	GetMouseLabel:
			GetMouse( &w );
		} while ( WaitMouseUp() );
	}
}

/*** INVERT SELECTION ***/
void InvertSelection( EditWindowPtr	dWin )
{
	Rect	r;
	long	start, end;
	short	startX, endX;
	Boolean	frontFlag;
	RGBColor invertColor;

	frontFlag = (dWin->oWin.theWin == FrontWindow() && dWin->oWin.active);

	if( dWin->endSel <= dWin->startSel )
		return;

	if( ctHdl )	invertColor = ( *ctHdl )->body;
	else		invertColor = white;
	
	InvertColor( &invertColor );
	
	start = dWin->startSel - dWin->editOffset;
	if( start < 0 )
		start = 0;
	end = ( dWin->endSel-1 ) - dWin->editOffset;
	if( end > ( dWin->linesPerPage<<4 )-1 )
		end = ( dWin->linesPerPage<<4 )-1;
	
	PenMode( patXor );
	
	startX = ColNbr( start );
	endX = ColNbr( end );
	
	if( !frontFlag )
	{
		if( LineNbr( start ) < LineNbr( end ) )
		{
			// Invert Hex
			r.top = HeaderHeight+TopMargin+LineNbr( start ) *LineHeight;
			r.bottom = r.top+LineHeight;
			r.left = AddrPos+CharPos( HexStart )+HexPos( startX )-3;
			r.right = AddrPos+CharPos( HexStart )+HexPos( 16 )-3;

			MoveTo( AddrPos+CharPos( HexStart )-3, r.bottom );

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
			r.left = AddrPos+CharPos( AsciiStart )+CharPos( startX )-1;
			r.right = AddrPos+CharPos( AsciiStart )+CharPos( 16 );
			
			MoveTo( AddrPos+CharPos( AsciiStart ), r.bottom );
			LineTo( r.left, r.bottom );

			LineTo( r.left, r.top );
			if( dWin->startSel >= dWin->editOffset )
			{
				LineTo( r.right, r.top );
			}
			else
				MoveTo( r.right, r.top );
			LineTo( r.right, r.bottom );

			if( LineNbr( start ) < LineNbr( end )-1 )
			{
				r.top = HeaderHeight+TopMargin+LineNbr( start ) *LineHeight+LineHeight;
				r.bottom = HeaderHeight+TopMargin+LineNbr( end ) *LineHeight;
				r.left = AddrPos+CharPos( HexStart )-3;
				r.right = AddrPos+CharPos( HexStart )+HexPos( 16 )-3;
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom );
				MoveTo( r.right, r.top );
				LineTo( r.right, r.bottom );

				r.left = AddrPos+CharPos( AsciiStart )-1;
				r.right = AddrPos+CharPos( AsciiStart )+CharPos( 16 );
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom );
				MoveTo( r.right, r.top );
				LineTo( r.right, r.bottom );
			}
			r.top = HeaderHeight+TopMargin+LineNbr( end ) *LineHeight;
			r.bottom = r.top+LineHeight;
			r.left = AddrPos+CharPos( HexStart )-3;
			r.right = AddrPos+CharPos( HexStart )+HexPos( endX )+HexWidth-3;
			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom );
			if( dWin->endSel < dWin->editOffset+dWin->linesPerPage*16 )
			{
				LineTo( r.right, r.bottom );
			}
			else
				MoveTo( r.right, r.bottom );
			LineTo( r.right, r.top );
			LineTo( AddrPos+CharPos( HexStart )+HexPos( 16 )-3, r.top );

			r.left = AddrPos+CharPos( AsciiStart )-1;
			r.right = AddrPos+CharPos( AsciiStart )+CharPos( endX )+CharWidth-1;
			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom-1 );
			if( dWin->endSel < dWin->editOffset+dWin->linesPerPage*16 )
			{
				LineTo( r.right, r.bottom-1 );
			}
			else
				MoveTo( r.right, r.bottom-1 );
			LineTo( r.right, r.top );
			LineTo( AddrPos+CharPos( AsciiStart )+CharPos( 16 ), r.top );
		}
		else
		{
			r.top = HeaderHeight+TopMargin+LineNbr( start ) *LineHeight;
			r.bottom = r.top+LineHeight;
			r.left = AddrPos+CharPos( HexStart )+HexPos( startX )-3;
			r.right = AddrPos+CharPos( HexStart )+HexPos( endX )+HexWidth-3;
			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom );
			if( dWin->endSel < dWin->editOffset+dWin->linesPerPage*16 )
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

			r.left = AddrPos+CharPos( AsciiStart )+CharPos( startX )-1;
			r.right = AddrPos+CharPos( AsciiStart )+CharPos( endX )+CharWidth-1;

			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom-1 );
			if( dWin->endSel < dWin->editOffset+dWin->linesPerPage*16 )
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
	else
	{
		if( dWin->editMode == EM_Hex )
		{
			if( LineNbr( start ) < LineNbr( end ) )
			{
	
				// Invert Hex
				r.top = HeaderHeight+TopMargin+LineNbr( start ) *LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos( HexStart )+HexPos( startX )-3;
				r.right = AddrPos+CharPos( HexStart )+HexPos( 16 )-3;
	
	HiliteColor( &invertColor );
				InvertRect( &r );
	
	
				// Outline Box around Ascii
				r.left = AddrPos+CharPos( AsciiStart )+CharPos( startX )-1;
				r.right = AddrPos+CharPos( AsciiStart )+CharPos( 16 );
				
				MoveTo( AddrPos+CharPos( AsciiStart ), r.bottom );
				LineTo( r.left, r.bottom );
	
				LineTo( r.left, r.top );
				if( dWin->startSel >= dWin->editOffset )
				{
					LineTo( r.right, r.top );
				}
				else
					MoveTo( r.right, r.top );
				LineTo( r.right, r.bottom );
	
				if( LineNbr( start ) < LineNbr( end )-1 )
				{
					r.top = HeaderHeight+TopMargin+LineNbr( start ) *LineHeight+LineHeight;
					r.bottom = HeaderHeight+TopMargin+LineNbr( end ) *LineHeight;
					r.left = AddrPos+CharPos( HexStart )-3;
					r.right = AddrPos+CharPos( HexStart )+HexPos( 16 )-3;
					InvertRect( &r );
	
					r.left = AddrPos+CharPos( AsciiStart )-1;
					r.right = AddrPos+CharPos( AsciiStart )+CharPos( 16 );
					MoveTo( r.left, r.top );
					LineTo( r.left, r.bottom );
					MoveTo( r.right, r.top );
					LineTo( r.right, r.bottom );
				}
				r.top = HeaderHeight+TopMargin+LineNbr( end ) *LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos( HexStart )-3;
				r.right = AddrPos+CharPos( HexStart )+HexPos( endX )+HexWidth-3;
				InvertRect( &r );
	
				r.left = AddrPos+CharPos( AsciiStart )-1;
				r.right = AddrPos+CharPos( AsciiStart )+CharPos( endX )+CharWidth-1;
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom-1 );
				if( dWin->endSel < dWin->editOffset+dWin->linesPerPage*16 )
				{
					LineTo( r.right, r.bottom-1 );
				}
				else
					MoveTo( r.right, r.bottom-1 );
				LineTo( r.right, r.top );
				LineTo( AddrPos+CharPos( AsciiStart )+CharPos( 16 ), r.top );
			}
			else
			{
				r.top = HeaderHeight+TopMargin+LineNbr( start ) *LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos( HexStart )+HexPos( startX )-3;
				r.right = AddrPos+CharPos( HexStart )+HexPos( endX )+HexWidth-3;

				InvertRect( &r );
	
				r.left = AddrPos+CharPos( AsciiStart )+CharPos( startX )-1;
				r.right = AddrPos+CharPos( AsciiStart )+CharPos( endX )+CharWidth-1;
	
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom-1 );
				if( dWin->endSel < dWin->editOffset+dWin->linesPerPage*16 )
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
		else
		{
			// Ascii Mode!!
			// 
			if( LineNbr( start ) < LineNbr( end ) )
			{
	
				// Invert Hex
				r.top = HeaderHeight+TopMargin+LineNbr( start ) *LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos( HexStart )+HexPos( startX )-3;
				r.right = AddrPos+CharPos( HexStart )+HexPos( 16 )-3;
	
				MoveTo( AddrPos+CharPos( HexStart )-3, r.bottom );
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
				r.left = AddrPos+CharPos( AsciiStart )+CharPos( startX )-1;
				r.right = AddrPos+CharPos( AsciiStart )+CharPos( 16 )-1;
				
				InvertRect( &r );
	
				if( LineNbr( start ) < LineNbr( end )-1 )
				{
					r.top = HeaderHeight+TopMargin+LineNbr( start ) *LineHeight+LineHeight;
					r.bottom = HeaderHeight+TopMargin+LineNbr( end ) *LineHeight;
					r.left = AddrPos+CharPos( HexStart )-3;
					r.right = AddrPos+CharPos( HexStart )+HexPos( 16 )-3;
					MoveTo( r.left, r.top );
					LineTo( r.left, r.bottom );
					MoveTo( r.right, r.top );
					LineTo( r.right, r.bottom );
	
					r.left = AddrPos+CharPos( AsciiStart )-1;
					r.right = AddrPos+CharPos( AsciiStart )+CharPos( 16 )-1;
					InvertRect( &r );
				}
				r.top = HeaderHeight+TopMargin+LineNbr( end ) *LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos( HexStart )-3;
				r.right = AddrPos+CharPos( HexStart )+HexPos( endX )+HexWidth-3;
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom );
				if( dWin->endSel < dWin->editOffset+dWin->linesPerPage*16 )
				{
					LineTo( r.right, r.bottom );
				}
				else
					MoveTo( r.right, r.bottom );
				LineTo( r.right, r.top );
				LineTo( AddrPos+CharPos( HexStart )+HexPos( 16 )-3, r.top );
	
				r.left = AddrPos+CharPos( AsciiStart )-1;
				r.right = AddrPos+CharPos( AsciiStart )+CharPos( endX )+CharWidth-1;
				InvertRect( &r );
			}
			else
			{
				r.top = HeaderHeight+TopMargin+LineNbr( start ) *LineHeight;
				r.bottom = r.top+LineHeight;
				r.left = AddrPos+CharPos( HexStart )+HexPos( startX )-3;
				r.right = AddrPos+CharPos( HexStart )+HexPos( endX )+HexWidth-3;
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom );
				if( dWin->endSel < dWin->editOffset+dWin->linesPerPage*16 )
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
	
				r.left = AddrPos+CharPos( AsciiStart )+CharPos( startX )-1;
				r.right = AddrPos+CharPos( AsciiStart )+CharPos( endX )+CharWidth-1;
				InvertRect( &r );
			}
		}
	}
	PenMode( patCopy );
}

/*** PRINT WINDOW ***/
void PrintWindow( EditWindowPtr dWin )
{
#if TARGET_API_MAC_CARBON	// LR: v1.6
	#pragma unused( dWin )
#else
	TPPrPort	printPort;
	TPrStatus	prStatus;
	Boolean		ok;
	Rect		r;
	short		pageNbr, startPage, endPage, nbrPages;
	long		startAddr, endAddr, addr;
	short		linesPerPage;
#endif

	GrafPtr		savePort;

	GetPort( &savePort );

#if TARGET_API_MAC_CARBON	// LR: v1.6
// bug: Printing!
#else
	PrOpen();

	ok = PrValidate( g.HPrint );
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
		linesPerPage = ( r.bottom - TopMargin - ( HeaderHeight +1 ) ) / LineHeight;
		nbrPages = ( ( endAddr - startAddr ) /16 ) /linesPerPage + 1;

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
				DrawHeader( dWin, &r );
		
				r.top += ( HeaderHeight+TopMargin+LineHeight-DescendHeight );
				r.bottom -= ( FooterHeight + DescendHeight + BotMargin );
		
				DrawDump( dWin, &r, addr, endAddr );
	
				r.top = r.bottom + DescendHeight + BotMargin;
				r.bottom = r.top + FooterHeight;
				DrawDump( dWin, &r, pageNbr, nbrPages );
			}

			addr += linesPerPage*16;
			addr -= ( addr % 16 );
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

/*** OFFSET SELECTION ***/
void OffsetSelection( EditWindowPtr dWin, short offset, Boolean shiftFlag )
{
	long	selWidth;
	Boolean	fullUpdate;

	selWidth = dWin->endSel - dWin->startSel;
	fullUpdate = shiftFlag || selWidth > 1;

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
			ScrollToSelection( dWin, dWin->startSel, fullUpdate, false );
			if( !shiftFlag )
				CursorOn( dWin->oWin.theWin );
		}
		else
			SysBeep( 1 );
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
			ScrollToSelection( dWin, dWin->endSel, fullUpdate, false );
			if( !shiftFlag )
				CursorOn( dWin->oWin.theWin );
		}
		else
			SysBeep( 1 );
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
			dWin->editMode = !dWin->editMode;	// NS: v1.6.6, switch edit mode on tab or return
			UpdateOnscreen( dWin->oWin.theWin );
			break;
		
		// move insertion point
		case kLeftArrowCharCode:
			OffsetSelection( dWin, -1, (er->modifiers & shiftKey) > 0 );
			break;
		case kRightArrowCharCode:
			OffsetSelection( dWin, 1, (er->modifiers & shiftKey) > 0 );
			break;
		case kUpArrowCharCode:
			OffsetSelection( dWin, -16, (er->modifiers & shiftKey) > 0 );
			break;
		case kDownArrowCharCode:
			OffsetSelection( dWin, 16, (er->modifiers & shiftKey) > 0 );
			break;
		
		// scroll document
		case kPageUpCharCode:
			OffsetSelection( dWin, -16 * (dWin->linesPerPage -1), (er->modifiers & shiftKey) > 0 );
			break;
		case kPageDownCharCode:
			OffsetSelection( dWin, 16 * (dWin->linesPerPage -1), (er->modifiers & shiftKey) > 0 );
			break;
		case kHomeCharCode:
			OffsetSelection( dWin, 0xFEED, (er->modifiers & shiftKey) > 0 );
			break;
		case kEndCharCode:
			OffsetSelection( dWin, 0xBED, (er->modifiers & shiftKey) > 0 );
			break;
		
		// delete characters
		case kBackspaceCharCode:	// normal delete
			if( dWin->endSel > dWin->startSel )
				ClearSelection( dWin );
			else if( dWin->startSel > 0L )
			{
				ObscureCursor();
				--dWin->startSel;
				ClearSelection( dWin );
			}
			else SysBeep(0);
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
			else SysBeep(0);
			break;
		
		// insert/overwrite characters
		default:
			// Insert Ascii Text into Area indicated by dWin->startSel - dWin->endSel
			// Delete Current Selection if > 0
			ObscureCursor();

			if( dWin->editMode == EM_Ascii )
			{
				if( (dWin->endSel != dWin->lastTypePos ||
					dWin->startSel != dWin->lastTypePos) && g.undo )
					RememberOperation( dWin, EO_Typing, g.undo );
				if( dWin->endSel > dWin->startSel )
					DeleteSelection( dWin );
				if( prefs.overwrite && dWin->startSel < dWin->fileSize - 1 )
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
					dWin->startSel != dWin->lastTypePos) && g.undo )
				{
					dWin->loByteFlag = false;
					RememberOperation( dWin, EO_Typing, g.undo );
				}
				if( dWin->endSel > dWin->startSel )
					DeleteSelection( dWin );

				if( dWin->loByteFlag )
				{
					--dWin->startSel;
					DeleteSelection( dWin );
					hexVal = hexVal | ( dWin->lastNybble << 4 );
					InsertCharacter( dWin, hexVal );
					dWin->loByteFlag = false;
				}
				else
				{
					if( prefs.overwrite && dWin->startSel < dWin->fileSize - 1 )
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
	Rect			r;

	if( !g.cursorFlag && dWin->startSel >= dWin->editOffset && dWin->startSel < dWin->editOffset + ( dWin->linesPerPage << 4 ) ) 
	{
		g.cursorFlag = true;
		SetPortWindowPort( theWin );

		start = dWin->startSel - dWin->editOffset;

		if( dWin->editMode == EM_Hex )
		{
			r.top = HeaderHeight + TopMargin + LineNbr(start) * LineHeight;
			r.bottom = r.top + LineHeight;
			r.left = AddrPos + CharPos(HexStart) + ColNbr(start) * HexWidth -3;
			r.right = r.left +2;
			InvertRect( &r );
			g.cursRect = r;
		}
		else
		{
			r.top = HeaderHeight + TopMargin + LineNbr(start) * LineHeight;
			r.bottom = r.top + LineHeight;
			r.left = AddrPos + CharPos(AsciiStart) + ColNbr(start) * CharWidth -2;
			r.right = r.left +2;
			InvertRect( &r );
			g.cursRect = r;
		}
	}
}

/*** COPY FORK ***/
OSStatus CopyFork( FSSpec *srcSpec, FSSpec *dstSpec, short forkType )
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
		error = HOpenRF( srcSpec->vRefNum, srcSpec->parID, srcSpec->name, fsRdPerm, &sRefNum );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errOpen, error );
			return error;
		}
		error = HOpenRF( dstSpec->vRefNum, dstSpec->parID, dstSpec->name, fsWrPerm, &dRefNum );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errOpen, error );
			return error;
		}
	}
	else
	{
		error = HOpen( srcSpec->vRefNum, srcSpec->parID, srcSpec->name, fsRdPerm, &sRefNum );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errOpen, error );
			return error;
		}
		error = HOpen( dstSpec->vRefNum, dstSpec->parID, dstSpec->name, fsWrPerm, &dRefNum );
		if( error != noErr )
		{
			ErrorAlert( ES_Caution, errOpen, error );
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

/*** ENSURE NAME IS UNIQUE ***/	
// LR: complete re-write as this function used to create bogus names and overwrite memory
void EnsureNameIsUnique( FSSpec *tSpec )
{
	OSStatus err;
	FInfo fInfo;
	short i = tSpec->name[0];

	// Weird compiler bug, having the OS call in the while loop can caues it to fail w/o error
	do
	{
		err = HGetFInfo( tSpec->vRefNum, tSpec->parID, tSpec->name, &fInfo );
		if( err != fnfErr )
		{
			if( tSpec->name[0] > 31 )
				tSpec->name[0] = 31;

			tSpec->name[i] = MyRandom( 'z' - 'A' ) + 'A';
			i--;
		}
	}while( err != fnfErr && i );
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
	
	if( dWin->destSpec.name[0] == 0 )
		SaveAsContents( theWin );
	else
	{
		// Create temp file
		tSpec = dWin->destSpec;

		// If original file exists, write to temp folder
		if( dWin->refNum )
		{
			if( tSpec.name[0] < 31 )
			{
				tSpec.name[0]++;
				tSpec.name[tSpec.name[0]] = 't';
			}
			else	tSpec.name[31] ^= 0x10;
		}
		EnsureNameIsUnique( &tSpec );

		HDelete( tSpec.vRefNum, tSpec.parID, tSpec.name );
		error = HCreate( tSpec.vRefNum, tSpec.parID, tSpec.name, dWin->creator, dWin->fileType );
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
		// Preserve other fork if it exists
		if( dWin->refNum )
			if( CopyFork( &dWin->fsSpec, &tSpec, !dWin->fork ) != noErr )
				return;

		// Open the temp file
		if( dWin->fork == FT_Resource )
		{
			error = HOpenRF( tSpec.vRefNum, tSpec.parID, tSpec.name, fsWrPerm, &tRefNum );
			if( error != noErr )
			{
				ErrorAlert( ES_Caution, errOpen, error );
				return;
			}
		}
		else
		{
			error = HOpen( tSpec.vRefNum, tSpec.parID, tSpec.name, fsWrPerm, &tRefNum );
			if( error != noErr )
			{
				ErrorAlert( ES_Caution, errOpen, error );
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
					goto DiskFull;
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
			if( ( error = HOpen( bSpec.vRefNum, bSpec.parID, bSpec.name, fsRdPerm, &dWin->refNum ) ) == noErr )
			{
				FSClose( dWin->refNum );

				if( prefs.backupFlag )
				{
					// Delete last backup file, if it exists
					bSpec = dWin->destSpec;
					if( bSpec.name[0] < 31 )	// LR: 000505 -- don't go beyond 31 chars!
						bSpec.name[0]++;
					bSpec.name[bSpec.name[0]] = '~';
					HDelete( bSpec.vRefNum, bSpec.parID, bSpec.name );
		
					// Rename original file to backup name
					error = HRename( dWin->destSpec.vRefNum, dWin->destSpec.parID, dWin->destSpec.name, bSpec.name );
					if( error != noErr )
					{
						// Backup is probably open, just delete original
						ErrorAlert( ES_Caution, errBackup, error );
						bSpec = dWin->destSpec;
						HDelete( bSpec.vRefNum, bSpec.parID, bSpec.name );
					}
				}
				else
				{
					// Delete Original if backup flag is false
					bSpec = dWin->destSpec;
					HDelete( bSpec.vRefNum, bSpec.parID, bSpec.name );
				}
			}

			// Rename temp file to correct name
			error = HRename( tSpec.vRefNum, tSpec.parID, tSpec.name, dWin->destSpec.name );
			if( error != noErr )
				ErrorAlert( ES_Stop, errRename, error );
		}

		// Open newly saved file read only
		tSpec = dWin->destSpec;
		if( dWin->fork == FT_Resource )
		{
			error = HOpenRF( tSpec.vRefNum, tSpec.parID, tSpec.name, fsRdPerm, &dWin->refNum );
			if( error != noErr )
				ErrorAlert( ES_Stop, errOpen, error );
		}
		else
		{
			error = HOpen( tSpec.vRefNum, tSpec.parID, tSpec.name, fsRdPerm, &dWin->refNum );
			if( error != noErr )
				ErrorAlert( ES_Stop, errOpen, error );
		}

		// Reset Work File
		dWin->fsSpec = dWin->destSpec;
		SetWTitle( dWin->oWin.theWin, dWin->fsSpec.name );

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
	}
	return;
DiskFull:
	ErrorAlert( ES_Caution, errDiskFull );
	HDelete( tSpec.vRefNum, tSpec.parID, tSpec.name );
}

/*** SAVE AS CONTENTS ***/
void SaveAsContents( WindowRef theWin )
{
#if TARGET_API_MAC_CARBON	// LR: v1.6
	OSStatus error = noErr;
	NavReplyRecord		reply;
	NavDialogOptions	dialogOptions;
 	NavEventUPP			eventProc = NewNavEventUPP( NavEventFilter );
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	
	NavGetDefaultDialogOptions( &dialogOptions );
	GetWTitle( theWin, dialogOptions.savedFileName );
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
			SaveContents( theWin );
			NavCompleteSave( &reply, kNavTranslateInPlace );
		}
		NavDisposeReply( &reply );
	}
	DisposeNavEventUPP( eventProc );
#else
	StandardFileReply	reply;
	EditWindowPtr		dWin = (EditWindowPtr) GetWRefCon( theWin );
	Str63				fileName, prompt;

	GetWTitle( theWin, fileName );
	GetIndString( prompt, strPrompt, 1 );
	
	StandardPutFile( prompt, fileName, &reply );
	if( reply.sfGood )
	{
		dWin->destSpec = reply.sfFile;
		dWin->creationDate = 0;
		SaveContents( theWin );
	}
#endif
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
	if( dWin->editOffset > dWin->fileSize -16 * dWin->linesPerPage )
		dWin->editOffset = 0;
	DrawPage( dWin );
	UpdateOnscreen( theWin );
}

/*** MY ACTIVATE ***/
void MyActivate( WindowRef theWin, Boolean active )
{
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );

	if( dWin->vScrollBar )
		HiliteControl( dWin->vScrollBar, active? 0 : 255 );
	DefaultActivate( theWin, active );
}

/*** UPDATE EDIT WINDOWS ***/
//LR: 1.66 - avoid NULL window ref, DrawPage with CURRENT dWin (not first!)
void UpdateEditWindows( void )
{
	WindowRef		theWin = FrontWindow();
	EditWindowPtr	dWin;

	while( theWin )
	{
		long windowKind = GetWindowKind( theWin );
		if( windowKind == HexEditWindowID )
		{
			dWin = (EditWindowPtr)GetWRefCon( theWin );
			DrawPage( dWin );
			UpdateOnscreen( theWin );
		}
		theWin = GetNextWindow( theWin );
	}
}

// NS: v1.6.6, event filters for navigation services

/*** NAV SERVICES EVENT FILTER ***/
pascal void NavEventFilter( NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD )
{
	#pragma unused( callBackUD )
//	WindowRef theWindow = (WindowRef) callBackParms->eventData.event->message;
	WindowRef theWindow = (WindowRef) callBackParms->eventData.eventDataParms.event->message;
	switch( callBackSelector )
	{
		case kNavCBEvent:
//			switch( callBackParms->eventData.event->what )
			switch( callBackParms->eventData.eventDataParms.event->what )
			{
				case updateEvt:
/*					RgnHandle updateRgn;
					GetWindowRegion( theWindow, kWindowUpdateRgn, updateRgn );
					updateWindow( theWindow, updateRgn );
*/					break;
			}
			break;
	}
}

/*** NAV SERVICES PREVIEW FILTER ***/
pascal Boolean NavPreviewFilter( NavCBRecPtr callBackParms, void *callBackUD )
{
	#pragma unused( callBackParms, callBackUD )
	return false;
}

/*** NAV SERVICES FILE FILTER ***/
pascal Boolean NavFileFilter( AEDesc* theItem, void* info, void *callBackUD, NavFilterModes filterMode )
{
	#pragma unused( theItem, info, callBackUD, filterMode )
	return true;
}