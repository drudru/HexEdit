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

#include <stdio.h>

#include "EditWindow.h"
#include "EditRoutines.h"
#include "EditScrollbar.h"
#include "Menus.h"
#include "Prefs.h"
#include "Utility.h"

// Create a new main theWin using a 'WIND' template from the resource fork

SInt16			CompareFlag = 0;
WindowRef		CompWind1, 
				CompWind2;

HEColorTableHandle ctHdl = NULL;	// LR: global to file, for speed

RGBColor black = { 0, 0, 0 };
RGBColor white = { 0xFFFF, 0xFFFF, 0xFFFF };

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
        Calculates a valid page range and prints each page by calling DrawDump.

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
				DrawHeader( dWin, &r );

				r.top += kHeaderHeight;
				r.bottom = pageRect.bottom - kFooterHeight;
				DrawDump( dWin, &r, addr, endAddr );

				r.top = r.bottom;
				r.bottom += kFooterHeight;
				DrawFooter( dWin, &r, pageNumber, realNumberOfPagesinDoc );

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

/*** NAV SERVICES EVENT FILTER ***/
static pascal void _navEventFilter( NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms, NavCallBackUserData callBackUD )
{
	#pragma unused( callBackSelector, callBackParms, callBackUD )
/*
//	WindowRef theWindow = (WindowRef) callBackParms->eventData.event->message;
//	WindowRef theWindow = (WindowRef) callBackParms->eventData.eventDataParms.event->message;
	switch( callBackSelector )
	{
		case kNavCBEvent:
//			switch( callBackParms->eventData.event->what )
			switch( callBackParms->eventData.eventDataParms.event->what )
			{
				case updateEvt:
//					RgnHandle updateRgn;
//					GetWindowRegion( theWindow, kWindowUpdateRgn, updateRgn );
//					updateWindow( theWindow, updateRgn );
//					break;
			}
			break;
	}
*/
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
#endif

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
static GWorldPtr _newCOffScreen( short width, short height )
{
	OSStatus	error = noErr;
	GWorldPtr	theGWorld = NULL;
	Rect		rect;
	
	SetRect( &rect, 0, 0, width, height );
	error = NewGWorld( &theGWorld, prefs.useColor? 0:1, &rect, NULL, NULL, keepLocal );
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

	int i,l;

	// LR: 1.66 make sure the name is good (for instance "icon/r" is bad!)
	l = (int)dWin->fsSpec.name[0];
	for( i = 1; i <= l; i++ )
	{
		if( dWin->fsSpec.name[i] < ' ' )	// truncate name at fist bad char
			break;
		else
			wintitle[i] = dWin->fsSpec.name[i];
	}

	// LR: 1.7 Append fork in use to title
	if( i < 255 - 8 )
	{
		Str31 str2;

		GetIndString( (StringPtr) str2, strHeader, dWin->fork );

		BlockMoveData( &str2[1], &wintitle[i], str2[0] );

		wintitle[0] = i + str2[0] - 1;
	}

	SetWTitle( dWin->oWin.theWin, wintitle );

	// NS: 1.6.6 add window to window menu
	MacAppendMenu( GetMenuHandle(kWindowMenu), wintitle );
}

/*** SETUP NEW EDIT WINDOW ***/
// LR: 1.7 - static, and remove title (always fsSpec->name)

static OSStatus _setupNewEditWindow( EditWindowPtr dWin )
{
	WindowRef theWin;
	ObjectWindowPtr objectWindow;
	Rect r;

	theWin = InitObjectWindow( kMainWIND, (ObjectWindowPtr) dWin, false );
	if( !theWin )
		ErrorAlert( ES_Stop, errMemory );

	// LR:	Hack for comparing two files
	if( CompareFlag == 1 )
	{
		SetRect( &r, 0, 0, kHexWindowWidth, g.maxHeight / 2 - 64 );
		CompWind1 = theWin;
	}
	else if( CompareFlag == 2 )
	{
		SetRect( &r, 0, 0, kHexWindowWidth, g.maxHeight / 2 - 64 );
		MoveWindow( theWin, 14, g.maxHeight/2, true );
		CompWind2 = theWin;
	}
	else
		SetRect( &r, 0, 0, kHexWindowWidth, g.maxHeight - 64 );

	// Check for best window size
	if( (dWin->fileSize / kBytesPerLine) * kLineHeight < g.maxHeight )
		r.bottom = (((dWin->fileSize / kBytesPerLine) + 1) * kLineHeight) + kHeaderHeight;
//LR 1.7		r.bottom = (kLineHeight * (((dWin->linesPerPage - 1) * kBytesPerLine) - dWin->fileSize)) / kLineHeight;

	if( r.bottom < (kHeaderHeight + (kLineHeight * 5)) )
		r.bottom = (kHeaderHeight + (kLineHeight * 5));

	SizeWindow( theWin, r.right, r.bottom, true );

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
	
// LR: 1.5	SetRect( &offRect, 0, 0, kHexWindowWidth, g.maxHeight );

	if( prefs.useColor )
		dWin->csResID = prefs.csResID;	// LR: 1.5 - color selection
	else
		dWin->csResID = -1;	// LR: if created w/o color then offscreen is 1 bit, NO COLOR possible!

	// Make it the current grafport
	SetPortWindowPort( theWin );
	
	dWin->offscreen = _newCOffScreen( kHexWindowWidth - kSBarSize, g.maxHeight - kHeaderHeight );	// LR: 1.7 - areas for scroll bar & header not needed!
	if( !dWin->offscreen )
			ErrorAlert( ES_Stop, errMemory );

	// Show the theWin
	ShowWindow( theWin );

	SetupScrollBars( dWin );

	GetWindowPortBounds( theWin, &r );

//LR: 1.7 -fix lpp calculation!	dWin->linesPerPage = ( r.bottom - TopMargin - BotMargin - ( kHeaderHeight-1 ) ) / kLineHeight + 1;
	dWin->linesPerPage = ((r.bottom - r.top) + (kLineHeight / 3) - kHeaderHeight) / kLineHeight;
	dWin->startSel = dWin->endSel = 0L;
	dWin->editMode = EM_Hex;

	//LR: 1.7 - what was this??? ((WStateData *) *((WindowPeek)theWin)->dataHandle)->stdState.left + kHexWindowWidth;

	LocalToGlobal( (Point *)&r.top );
	LocalToGlobal( (Point *)&r.bottom );

	r.bottom /= 2;		// zoom'd state is 1/2 of normal (actually the reverse of standard zoom!)

	SetWindowStandardState( theWin, &r );

	return noErr;
}


/*** INITIALIZE EDITOR ***/
void InitializeEditor( void )
{
	CursHandle			cursorHandle = NULL;
#if !TARGET_API_MAC_CARBON	// LR: v1.6
	PScrapStuff			ScrapInfo;
#endif

#if !TARGET_API_MAC_CARBON	// LR: v1.6.5
//	OSStatus anErr = ClearCurrentScrap();
//#else
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
	g.maxHeight = (((g.maxHeight - 1) / kLineHeight) * kLineHeight) + kHeaderHeight;

	GetFNum( kFontFace, &g.fontFaceID );		// 1.7 carsten-unhardcoded font name & size
	g.fontSize = kFontSize;

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
	PrefsSave();

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

/*** NEW EDIT WINDOW ***/
void NewEditWindow( void )
{
	EditWindowPtr		dWin;
	OSStatus				error;
	short				refNum = NULL;
	Point				where = { -1, -1 };
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
	HCreate( workSpec.vRefNum, workSpec.parID, workSpec.name, kAppCreator, '????' );
	if( error != noErr )
	{
		ErrorAlert( ES_Caution, errCreate, error );
		return;
	}
	error = HOpenDF( workSpec.vRefNum, workSpec.parID, workSpec.name, fsRdWrPerm, &refNum );
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

	_setupNewEditWindow( dWin );	//LR 1.66 "\pUntitled" );	// LR: 1.5 -make mashortenence easier!

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
 	NavEventUPP			eventProc = NewNavEventUPP( _navEventFilter );
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
	Str31				tempStr;
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
			GetIndString( tempStr, strFiles, FN_DATA );
			ParamText( fsSpec->name, tempStr, NULL, NULL );
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
			error = HOpenDF( fsSpec->vRefNum, fsSpec->parID, fsSpec->name, fsRdPerm, &refNum );
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

			GetIndString( tempStr, strFiles, FN_RSRC );
			ParamText( fsSpec->name, tempStr, NULL, NULL );
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

	_ensureNameIsUnique( &workSpec );
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

	dWin->fsSpec =
	dWin->destSpec = *fsSpec;

	error = _setupNewEditWindow( dWin );	// LR: 1.5 -make maintenence easier!
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

	// LR: v1.7 -- if no edit window available, close find windows
	if( !FindFirstEditWindow() )
	{
		if( g.gotoWin )
			HideWindow( GetDialogWindow( g.gotoWin ) );
		if( g.searchWin )
			HideWindow( GetDialogWindow( g.searchWin ) );
	}

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
		else if( windowKind == kHexEditWindowTag )
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

/*** FIND FIRST EDIT WINDOW ***/
EditWindowPtr FindFirstEditWindow( void )
{
	WindowRef theWin, editWin = NULL;

	// Find and Select Top Window
	//LR: 1.66 total re-write to avoid null window references!

	theWin = FrontWindow();
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
	char str[256];	//LR 1.7 -- no need for fork, replaced with selection length, str2[31];

	GetColorInfo( dWin );	// LR: v1.6.5 ensure that updates are valid!

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
	
	GetIndString( (StringPtr) str, strHeader, prefs.decimalAddr? HD_Decimal : HD_Hex );
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
void DrawFooter( EditWindowPtr dWin, Rect *r, short pageNbr, short nbrPages )
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

OSStatus DrawDump( EditWindowPtr dWin, Rect *r, long sAddr, long eAddr )
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

	// create bounds rectangle
	addrRect.top = r->top;	// we need to erase seperating space
	addrRect.left = r->left;
	addrRect.right = r->left + kBodyDrawPos + StringWidth( "\p 0000000:" );
	addrRect.bottom = r->bottom;

	if( ctHdl )
		RGBBackColor( &( *ctHdl )->bar );
	EraseRect( &addrRect );

	addr = sAddr - (sAddr % kBytesPerLine);

	g.buffer[kStringTextPos - 1] = g.buffer[kStringHexPos - 1] = g.buffer[kStringHexPos + kBodyStrLen] = ' ';

	// draw each line of data
	for( y = r->top + (kLineHeight - 2), j = 0; y < r->bottom && addr < eAddr; y += kLineHeight, j++ )
	{
		if( prefs.decimalAddr )
			sprintf( (char *)&g.buffer[0], "%8ld:", addr );
		else
			sprintf( (char *)&g.buffer[0], " %07lX:", addr );

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

			if( !(j & 1) )
			{
				RGBBackColor( &( *ctHdl )->body );
				RGBForeColor( &( *ctHdl )->text );
			}

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
				g.buffer[hexPos++] = ch1 + (( ch1 < 10 )? '0' : ( 'A'-10 ));
				g.buffer[hexPos++] = ch2 + (( ch2 < 10 )? '0' : ( 'A'-10 ));
				g.buffer[hexPos++] = ' ';
				g.buffer[asciiPos++] = (ch >= 0x20 && ch <= g.highChar && 0x7F != ch) ? ch : '.';	// LR: 1.7 - 0x7F doesn't draw ANYTHING! Ouch!
			}
			else
			{
				g.buffer[hexPos++] = ' ';
				g.buffer[hexPos++] = ' ';
				g.buffer[hexPos++] = ' ';
				g.buffer[asciiPos++] = ' ';
			}
		}

		// %% NOTE: Carsten says to move this into for loop (ie, draw each byte's data) to
		//			prevent font smoothing from messing up the spacing
		MoveTo( kDataDrawPos - kCharWidth, y );
		DrawText( g.buffer, kStringHexPos - 1, kBodyStrLen + 3 );
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
	if( prefs.vertBars )
	{
		if( ctHdl )
			RGBForeColor( &( *ctHdl )->headerLine );	// LR: v1.6.5 LR - was barText

		MoveTo( CHARPOS(22) - (kCharWidth / 2), addrRect.top );
		LineTo( CHARPOS(22) - (kCharWidth / 2), addrRect.bottom );

		MoveTo( CHARPOS(34) - (kCharWidth / 2) - 1, addrRect.top );
		LineTo( CHARPOS(34) - (kCharWidth / 2) - 1, addrRect.bottom );

		MoveTo( CHARPOS(46) - (kCharWidth / 2) - 2, addrRect.top );
		LineTo( CHARPOS(46) - (kCharWidth / 2) - 2, addrRect.bottom );
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
	PixMapHandle	thePixMapH; // sel, for (un)LockPixels

#if PROFILE
	_profile = 1;
#endif

	GetColorInfo( dWin );	// LR: v1.6.5 multiple routines need this

	GetPort( &savePort );
	thePixMapH = GetGWorldPixMap( dWin->offscreen );
	if ( LockPixels( thePixMapH ) )
	{
		SetPort( ( GrafPtr )dWin->offscreen );

		GetPortBounds( dWin->offscreen, &r );

		if( ctHdl )
			RGBBackColor( &( *ctHdl )->body );
		else
		{
			ForeColor( blackColor );
			BackColor( whiteColor );
		}

		// Erase only that part of the buffer that isn't drawn to!
		if( dWin->fileSize / kBytesPerLine < dWin->linesPerPage )
		{
			Rect er = r;

			er.top = (dWin->fileSize / kBytesPerLine) + kHeaderHeight;
			EraseRect( &er );
		}

		DrawDump( dWin, &r, dWin->editOffset, dWin->fileSize );

		if( ctHdl )
		{
			RGBForeColor( &black );
			RGBBackColor( &white );
		}

		UnlockPixels( thePixMapH ); // sel

	// LR: offscreen fix!	SetPortBits( &realBits );
		SetPort( savePort );
	}

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
//	DrawGrowIcon( theWin );

	// EraseRect( &theWin->portRect );
	DrawPage( dWin );
	UpdateOnscreen( theWin );
}

/*** UPDATE ONSCREEN ***/
void UpdateOnscreen( WindowRef theWin )
{
	Rect			r1, r2;//, r3;
	GrafPtr			oldPort;
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	PixMapHandle thePixMapH;

	// Now, draw header to main window & blit offscreen
	thePixMapH = GetPortPixMap( GetWindowPort( theWin ) );
	if ( LockPixels( thePixMapH ) )
	{
		GetPortBounds( dWin->offscreen, &r1 );
		GetWindowPortBounds( theWin, &r2 );

		GetPort( &oldPort );
		SetPortWindowPort( theWin );

		g.cursorFlag = false;

		// LR: Header drawn here due to overlapping vScrollBar
		r2.bottom = r2.top + kHeaderHeight - 1;
		DrawHeader( dWin, &r2 );

		// LR: adjust for scrollbar & header
		GetWindowPortBounds( theWin, &r2 );
// LR: 1.7		r2.right -= kSBarSize - 1;
//		r2.top += kHeaderHeight;
//		SectRect( &r1, &r2, &r3 );

		// restrict draw height to height of window port!
		r2.top = kHeaderHeight;
		r2.right -= kSBarSize;
		r1.bottom = r1.top + (r2.bottom - r2.top);

	// LR: offscreen fix!	CopyBits( ( BitMap * ) &dWin->pixMap, &theWin->portBits, &r3, &r3, srcCopy, 0L );	// LR: -- PixMap change
	#if TARGET_API_MAC_CARBON	// LR: v1.6
		CopyBits( GetPortBitMapForCopyBits( dWin->offscreen ), GetPortBitMapForCopyBits( GetWindowPort( theWin ) ), &r1, &r2, srcCopy, 0L );
	#else
		CopyBits( ( BitMap * ) &( dWin->offscreen )->portPixMap, &theWin->portBits, &r1, &r2, srcCopy, 0L );
	#endif

		//%% LR: 1.7 -- needs to be done offscreen, but then it's  not erased -- this is a new shell todo item :)
		if( dWin->endSel > dWin->startSel && dWin->endSel >= dWin->editOffset && dWin->startSel < dWin->editOffset + (dWin->linesPerPage * kBytesPerLine) )
			InvertSelection( dWin );

		UnlockPixels( thePixMapH );
		SetPort( oldPort );
	}
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
	if( MyHandleControlClick( theWin, w ) )
		return;
	// Else handle editing chore
	CursorOff( theWin );
	if( w.v >= kHeaderHeight && w.v < kHeaderHeight+( dWin->linesPerPage * kLineHeight ) )
	{
		do
		{
			AutoScroll( dWin, w );

			if( w.h >= kDataDrawPos && w.h < kDataDrawPos + (kHexWidth * (kBytesPerLine + 1)) )
			{

				pos = (((w.v - kHeaderHeight) / kLineHeight) * kBytesPerLine) + (w.h - kDataDrawPos + (kHexWidth / 2)) / kHexWidth;
				dWin->editMode = EM_Hex;
			}
			else if( w.h >= kTextDrawPos && w.h < kTextDrawPos + (kCharWidth * (kBytesPerLine + 1)) )
			{
				pos = (((w.v - kHeaderHeight) / kLineHeight) * kBytesPerLine) + (w.h -  kTextDrawPos + (kCharWidth / 2)) / kCharWidth;
				dWin->editMode = EM_Ascii;
			}
			else
				goto GetMouseLabel;

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
	if( end > ( dWin->linesPerPage * kBytesPerLine )-1 )
		end = ( dWin->linesPerPage * kBytesPerLine )-1;
	
	startX = COLUMN( start );
	endX = COLUMN( end );
	
	if( !frontFlag )
	{
		if( LINENUM( start ) < LINENUM( end ) )
		{
			// Outline Hex
			r.top = kHeaderHeight + LINENUM( start ) * kLineHeight;
			r.bottom = r.top+kLineHeight;
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
			{
				LineTo( r.right, r.top );
			}
			else
				MoveTo( r.right, r.top );
			LineTo( r.right, r.bottom );

			if( LINENUM( start ) < LINENUM( end ) - 1 )
			{
				r.top = kHeaderHeight + LINENUM( start ) * kLineHeight + kLineHeight;
				r.bottom = kHeaderHeight + LINENUM( end ) * kLineHeight;
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
			r.top = kHeaderHeight + LINENUM( end ) * kLineHeight;
			r.bottom = r.top + kLineHeight;
			r.left = kDataDrawPos - 3;
			r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;
			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom );
			if( dWin->endSel < dWin->editOffset+dWin->linesPerPage * kBytesPerLine )
			{
				LineTo( r.right, r.bottom );
			}
			else
				MoveTo( r.right, r.bottom );
			LineTo( r.right, r.top );
			LineTo( kDataDrawPos + HEXPOS( kBytesPerLine ) - 3, r.top );

			r.left = kTextDrawPos - 1;
			r.right = kTextDrawPos + CHARPOS( endX ) + kCharWidth - 1;
			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom-1 );
			if( dWin->endSel < dWin->editOffset+dWin->linesPerPage * kBytesPerLine )
			{
				LineTo( r.right, r.bottom-1 );
			}
			else
				MoveTo( r.right, r.bottom-1 );
			LineTo( r.right, r.top );
			LineTo( kTextDrawPos + CHARPOS( kBytesPerLine ), r.top );
		}
		else
		{
			r.top = kHeaderHeight+LINENUM( start ) * kLineHeight;
			r.bottom = r.top+kLineHeight;
			r.left = kDataDrawPos + HEXPOS( startX ) - 3;
			r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;
			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom );
			if( dWin->endSel < dWin->editOffset+dWin->linesPerPage * kBytesPerLine )
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

			r.left = kTextDrawPos + CHARPOS( startX )-1;
			r.right = kTextDrawPos + CHARPOS( endX ) + kCharWidth - 1;

			MoveTo( r.left, r.top );
			LineTo( r.left, r.bottom-1 );
			if( dWin->endSel < dWin->editOffset+dWin->linesPerPage * kBytesPerLine )
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
			if( LINENUM( start ) < LINENUM( end ) )
			{
	
				// Invert Hex
				r.top = kHeaderHeight+LINENUM( start ) * kLineHeight;
				r.bottom = r.top + kLineHeight;
				r.left = kDataDrawPos + HEXPOS( startX ) - 3;
				r.right = kDataDrawPos + HEXPOS( kBytesPerLine ) - 3;
				InvertRect( &r );

				// Outline Box around Ascii
				r.left = kTextDrawPos + CHARPOS( startX ) - 1;
				r.right = kTextDrawPos + CHARPOS( kBytesPerLine );
				
				MoveTo( kTextDrawPos, r.bottom );
				LineTo( r.left, r.bottom );
	
				LineTo( r.left, r.top );
				if( dWin->startSel >= dWin->editOffset )
				{
					LineTo( r.right, r.top );
				}
				else
					MoveTo( r.right, r.top );
				LineTo( r.right, r.bottom );
	
				if( LINENUM( start ) < LINENUM( end )-1 )
				{
					r.top = kHeaderHeight+LINENUM( start ) * kLineHeight + kLineHeight;
					r.bottom = kHeaderHeight+LINENUM( end ) * kLineHeight;
					r.left = kDataDrawPos - 3;
					r.right = kDataDrawPos + HEXPOS( kBytesPerLine ) - 3;
					InvertRect( &r );
	
					r.left = kTextDrawPos - 1;
					r.right = kTextDrawPos + CHARPOS( kBytesPerLine );
					MoveTo( r.left, r.top );
					LineTo( r.left, r.bottom );
					MoveTo( r.right, r.top );
					LineTo( r.right, r.bottom );
				}
				r.top = kHeaderHeight+LINENUM( end ) * kLineHeight;
				r.bottom = r.top+kLineHeight;
				r.left = kDataDrawPos - 3;
				r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;
				InvertRect( &r );
	
				r.left = kTextDrawPos - 1;
				r.right = kTextDrawPos + CHARPOS( endX ) + kCharWidth - 1;
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom-1 );
				if( dWin->endSel < dWin->editOffset+dWin->linesPerPage * kBytesPerLine )
				{
					LineTo( r.right, r.bottom-1 );
				}
				else
					MoveTo( r.right, r.bottom-1 );
				LineTo( r.right, r.top );
				LineTo( kTextDrawPos + CHARPOS( kBytesPerLine ), r.top );
			}
			else
			{
				r.top = kHeaderHeight+LINENUM( start ) * kLineHeight;
				r.bottom = r.top+kLineHeight;
				r.left = kDataDrawPos + HEXPOS( startX ) - 3;
				r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;

				InvertRect( &r );
	
				r.left = kTextDrawPos + CHARPOS( startX )-1;
				r.right = kTextDrawPos + CHARPOS( endX ) + kCharWidth - 1;
	
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom-1 );
				if( dWin->endSel < dWin->editOffset+dWin->linesPerPage * kBytesPerLine )
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
		else	// Ascii Mode!!
		{
			if( LINENUM( start ) < LINENUM( end ) )
			{
				// Outline Hex
				r.top = kHeaderHeight + LINENUM( start ) * kLineHeight;
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
				InvertRect( &r );
	
				if( LINENUM( start ) < LINENUM( end )-1 )
				{
					r.top = kHeaderHeight + LINENUM( start ) * kLineHeight + kLineHeight;
					r.bottom = kHeaderHeight + LINENUM( end ) * kLineHeight;
					r.left = kDataDrawPos - 3;
					r.right = kDataDrawPos + HEXPOS( kBytesPerLine ) - 3;
					MoveTo( r.left, r.top );
					LineTo( r.left, r.bottom );
					MoveTo( r.right, r.top );
					LineTo( r.right, r.bottom );
	
					r.left = kTextDrawPos - 1;
					r.right = kTextDrawPos + CHARPOS( kBytesPerLine );
					InvertRect( &r );
				}
				r.top = kHeaderHeight + LINENUM( end ) * kLineHeight;
				r.bottom = r.top+kLineHeight;
				r.left = kDataDrawPos - 3;
				r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom );
				if( dWin->endSel < dWin->editOffset+dWin->linesPerPage * kBytesPerLine )
				{
					LineTo( r.right, r.bottom );
				}
				else
					MoveTo( r.right, r.bottom );
				LineTo( r.right, r.top );
				LineTo( kDataDrawPos + HEXPOS( kBytesPerLine ) - 3, r.top );
	
				r.left = kTextDrawPos - 1;
				r.right = kTextDrawPos + CHARPOS( endX ) + kCharWidth - 1;
				InvertRect( &r );
			}
			else
			{
				r.top = kHeaderHeight+LINENUM( start ) * kLineHeight;
				r.bottom = r.top+kLineHeight;
				r.left = kDataDrawPos + HEXPOS( startX ) - 3;
				r.right = kDataDrawPos + HEXPOS( endX ) + kHexWidth - 3;
				MoveTo( r.left, r.top );
				LineTo( r.left, r.bottom );
				if( dWin->endSel < dWin->editOffset+dWin->linesPerPage * kBytesPerLine )
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
				InvertRect( &r );
			}
		}
	}
}

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

	ok = PrValidate( g.HPrint );
	if( ok )
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
				DrawHeader( dWin, &r );
		
				r.top += kHeaderHeight;
				r.bottom = printPort->gPort.portRect.bottom - kFooterHeight;
				DrawDump( dWin, &r, addr, endAddr );
	
				r.top = r.bottom;
				r.bottom += kFooterHeight;
				DrawFooter( dWin, &r, pageNbr, nbrPages );	//SEL: 1.7 - fix Lane's DrawDump usage (what was I thinking? P)
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
			OffsetSelection( dWin, -kBytesPerLine, (er->modifiers & shiftKey) > 0 );
			break;
		case kDownArrowCharCode:
			OffsetSelection( dWin, kBytesPerLine, (er->modifiers & shiftKey) > 0 );
			break;
		
		// scroll document
		case kPageUpCharCode:
			OffsetSelection( dWin, -kBytesPerLine * (dWin->linesPerPage - 1), (er->modifiers & shiftKey) > 0 );
			break;
		case kPageDownCharCode:
			OffsetSelection( dWin, kBytesPerLine * (dWin->linesPerPage - 1), (er->modifiers & shiftKey) > 0 );
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
					dWin->startSel != dWin->lastTypePos) )
					RememberOperation( dWin, EO_Typing, &gUndo );
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

		// If original file exists, write to temp file
		if( dWin->refNum )
		{
			if( tSpec.name[0] < 31 )
			{
				tSpec.name[0]++;
				tSpec.name[tSpec.name[0]] = '~';
			}
			else	tSpec.name[31] ^= 0x10;
		}
		_ensureNameIsUnique( &tSpec );

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
			error = HOpenDF( tSpec.vRefNum, tSpec.parID, tSpec.name, fsWrPerm, &tRefNum );
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
			error = HOpenDF( tSpec.vRefNum, tSpec.parID, tSpec.name, fsRdPerm, &dWin->refNum );
			if( error != noErr )
				ErrorAlert( ES_Stop, errOpen, error );
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
	}
	return;
DiskFull:
	ErrorAlert( ES_Caution, errDiskFull );
	HDelete( tSpec.vRefNum, tSpec.parID, tSpec.name );
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

#if TARGET_API_MAC_CARBON	// LR: v1.6
{
	OSStatus error = noErr;
	NavReplyRecord		reply;
	NavDialogOptions	dialogOptions;
	NavEventUPP			eventProc = NewNavEventUPP( _navEventFilter );

	NavGetDefaultDialogOptions( &dialogOptions );
//LR: 1.7 not w/modified window titles!	GetWTitle( theWin, dialogOptions.savedFileName );
	BlockMoveData( dWin->fsSpec.name, dialogOptions.savedFileName, dWin->fsSpec.name[0] );
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
}
#else
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
		if( windowKind == kHexEditWindowTag )
		{
			dWin = (EditWindowPtr)GetWRefCon( theWin );
			DrawPage( dWin );
			UpdateOnscreen( theWin );
		}
		theWin = GetNextWindow( theWin );
	}
}
