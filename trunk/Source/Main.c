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

#include <debugging.h>

#include "Main.h"
#include "Menus.h"
#include "Prefs.h"
#include "EditWindow.h"
#include "EditRoutines.h"
#include "EditScrollbar.h"
#include "HexCompare.h"
#include "HexSearch.h"

// set up global & prefs structures here
globals g;
prefs_t prefs;

// LR: --- These are used to setup new UPP stuff ---
AEEventHandlerUPP AEHandlerUPP, AECompareHandlerUPP;
extern ControlActionUPP trackActionUPP;
#if !TARGET_API_MAC_CARBON	// LR: v1.6
	extern DlgHookUPP myGetFileUPP;
#endif

extern SInt16		CompareFlag;
extern WindowRef	CompWind1, CompWind2;
static SInt32		caretTime;
static Boolean		skipOpen = false;

/*** MAIN ***/
void main( void )	// LR: fix warnings
{
	// get system version		// NS 1.7.1
	OSStatus error = Gestalt( gestaltSystemVersion, &g.systemVersion );		// this loads HIToolbox.framework on OS X
	if( error ) return;
	
	// Standard Mac Initialization
	InitToolbox();

#if !TARGET_API_MAC_CARBON	// LR: v1.6
	// Check if Multifinder ( WaitNextEvent ) is implemented
	InitMultifinder();

	// Check if System 7
	CheckEnvironment();
#endif

	InitGlobals();
	InitAppleEvents();
	InitMenubar();

	PrefsLoad();
	InitializeEditor();
	AdjustMenus();

#if !TARGET_API_MAC_CARBON	// LR: v1.6
	if( !g.sys7Flag )
		AskEditWindow();
#endif

	InitCursor();	// NS: v1.6.6, moved here from InitToolbox()
	
#if TARGET_API_MAC_CARBON  // SEL: 1.7 - carbon session based printing
	g.pageFormat = kPMNoPageFormat;
	g.printSettings = kPMNoPrintSettings;
#endif

	// main event loop
	while( !g.quitFlag )
		HandleEvent();

	CleanupEditor();

#ifndef __MC68K__
	if( ICStop )
	{
		ICStop( g.icRef );	// shutdown IC
	}
#endif
}

/*** INIT TOOLBOX ***/
OSStatus InitToolbox( void )
{
#if !TARGET_API_MAC_CARBON	// LR: v1.6
	MaxApplZone();
	InitGraf( &qd.thePort );
	InitFonts();
	FlushEvents( everyEvent, 0 );
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( 0L );
#endif

	memset( &g, 0, sizeof(g) );	// make sure we have a known starting state!

	caretTime = (GetCaretTime()/3)*2;
	return noErr;
}

// Check if WaitNextEvent ( Multifinder ) is implemented on this Macintosh
#if !TARGET_API_MAC_CARBON	// LR: v1.6
OSStatus InitMultifinder( void )
{
	g.WNEImplemented = ( NGetTrapAddress(_WaitNextEvent, ToolTrap) != NGetTrapAddress(_Unimplemented, ToolTrap) );
	return noErr;
}
#endif

/*** INIT GLOBALS ***/
OSStatus InitGlobals( void )
{
	// check for drag manager presence/attributes
	SInt32 result = 0;
	OSStatus error;

	error = Gestalt( gestaltDragMgrAttr, &result );
	g.dragAvailable = (Boolean) (result & (1 << gestaltDragMgrPresent));
	g.translucentDrag = (Boolean) (result & (1 << gestaltDragMgrHasImageSupport));
	
	// check appearance availablilty
	result = nil;
	error = Gestalt( gestaltAppearanceAttr, &result );
	g.appearanceAvailable = (Boolean) (result & (1 << gestaltAppearanceExists));
	if( g.appearanceAvailable )	g.useAppearance = true;
	else						g.useAppearance = false;
	if( g.useAppearance )		RegisterAppearanceClient();
	
	// check nav services availablilty
#ifndef __MC68K__
	g.navAvailable = (Boolean) NavServicesAvailable();
	if( g.navAvailable )		g.useNavServices = true;
	else						g.useNavServices = false;
	if( g.useNavServices )		NavLoad();

	// Startup InternetConfig (should only be done once!)
	if( ICStart )
	{
		error = ICStart( &g.icRef, kAppCreator );			// start IC up
		if( !error )
		{
#if !TARGET_API_MAC_CARBON
			if( ICFindConfigFile )
				error = ICFindConfigFile( g.icRef, 0, NULL );	// configure IC
#endif
		}
	}
#endif

	// application global initalisation
	g.forkMode = FM_Smart;
	g.highChar = 0x7F;
	g.watch = GetCursor( watchCursor );
	g.iBeam = GetCursor( iBeamCursor );
	g.searchBuffer[0] = g.searchText[0] = g.gotoText[0] = 0;
	g.searchDisabled = false;
	g.searchWin = NULL;
	g.gotoWin = NULL;

	//LR: 1.66 - clear undo/redo records
	memset( &gUndo, sizeof(UndoRecord), 0 );
	memset( &gRedo, sizeof(UndoRecord), 0 );

	return( noErr );
}

// The Main Event Dispatcher - this routine should be called repeatedly

/*** HANDLE EVENT ***/
OSStatus HandleEvent( void )
{
	EventRecord	theEvent;
	Boolean		ok;

	// If the more modern WaitNextEvent is implemented, use it 

#if TARGET_API_MAC_CARBON	// LR: v1.6
	ok = WaitNextEvent( everyEvent, &theEvent, caretTime, NULL );
#else
	if( g.WNEImplemented )
		// We don't have to call SystemTask because WaitNextEvent calls it for us
		// Get the next event ( LR 980716: give up lots of time, 'cos we don't need it : )
		ok = WaitNextEvent( everyEvent, &theEvent, caretTime, NULL );
	else
	{
		// we are running in ( Single ) Finder under system 6 or less
		// Give Desk Accessories some processing time
		SystemTask ();

		// Get the next event
		ok = GetNextEvent( everyEvent, &theEvent );
	}
#endif

	if( IsDialogEvent( &theEvent ) )	DoModelessDialogEvent( &theEvent );
	else if( ok )						DoEvent( &theEvent );
	else								IdleObjects( &theEvent );
	return noErr;
}


/*** DO EVENT ***/
OSStatus DoEvent( EventRecord *theEvent )
{
	short 			windowCode;
	WindowRef		theWin;
	ObjectWindowPtr	objectWindow;
	Rect			winRect;
	long			windowKind;

	switch( theEvent->what )
	{
	case mouseDown:
		// Find out where the mouse went down
		windowCode = FindWindow ( theEvent->where, &theWin );

			switch ( windowCode )
			{
#if !TARGET_API_MAC_CARBON	// LR: v1.6
			case inSysWindow:
				SystemClick ( theEvent, theWin );
				break;
#endif
				
			case inMenuBar:
				AdjustMenus();
				HandleMenu( MenuSelect( theEvent->where ) );
				break;

			default:			// Cursor was inside our theWin
				// If the theWin isn't in the front
				if( theWin != FrontNonFloatingWindow() )
				{
					// Make it so...
					SelectWindow( theWin );
					AdjustMenus();
				}
				else
				{
					// Window is already in the front, handle the click
				switch ( windowCode )
				{
					case inContent:
					{
						windowKind = GetWindowKind( theWin );
						objectWindow = (ObjectWindowPtr) GetWRefCon( theWin );
						if( windowKind == kHexEditWindowTag && objectWindow->HandleClick )
							objectWindow->HandleClick( theWin, theEvent->where, theEvent );
						break;
					}

					case inDrag:
					{
						Rect	dragRect;
#if TARGET_API_MAC_CARBON	// LR: v1.6
						BitMap qdScreenBits;

						GetQDGlobalsScreenBits( &qdScreenBits );
						dragRect = qdScreenBits.bounds;
#else
						dragRect = qd.screenBits.bounds;	// LR: qd.
#endif
						// Handle the dragging of the theWin
						DragWindow( theWin, theEvent->where, &dragRect );

						objectWindow = (ObjectWindowPtr)GetWRefCon( theWin );
						if( GetWindowKind( theWin ) == kHexEditWindowTag && !objectWindow->floating )	//LR: 1.66 don't deref non-edit windows!
							SelectWindow( theWin );

						break;
					}

					 case inGoAway:
						if( TrackGoAway( theWin, theEvent->where ) )
						{
							// If mouse is released inside the close box
							// Hide or close the theWin
							windowKind = GetWindowKind( theWin );
							if( windowKind == kHexEditWindowTag )
							{
								CloseEditWindow( theWin );
							}
							else if( g.searchWin && theWin == GetDialogWindow( g.searchWin ) )		//LR: 1.7 -- compares also need GetDialogWindow!
							{
// LR: v1.6.5					DisposeDialog( g.searchWin );
// LR: v1.6.5					g.searchWin = NULL;

								HideWindow( GetDialogWindow( g.searchWin ) );
							}
							else if( g.gotoWin && theWin == GetDialogWindow( g.gotoWin ) )		// LR: v1.6.5
							{
								HideWindow( GetDialogWindow( g.gotoWin ) );
							}

							AdjustMenus();
						}
						break;

					case inGrow:
					{
						long growResult;
						Rect r;

						SelectWindow( theWin );
						SetRect( &r, kHexWindowWidth, 64, kHexWindowWidth, g.maxHeight );

						// Handle the mouse tracking for the resizing
						growResult = GrowWindow( theWin, theEvent->where, &r );

						// Change the size of the theWin
						SizeWindow( theWin, LoWord( growResult ), HiWord( growResult ), true );

						// LR: 1.7 - see if user wants sizing restricted to full lines
						GetWindowPortBounds( theWin, &r );
						if( prefs.constrainSize )
						{
							int h = (r.bottom - r.top);

							r.bottom = r.top + (short)(((h / kLineHeight) - 1) * kLineHeight) + kHeaderHeight;	// force to full line heights
							SizeWindow( theWin, (r.right - r.left), (r.bottom - r.top), true );
						}

						AdjustScrollBars( theWin, true );
#if !TARGET_API_MAC_CARBON	// LR: v1.6
// LR: 1.5					DrawPage( (EditWindowPtr) GetWRefCon( theWin ) );
#endif

						// Redraw the theWin
						SetPortWindowPort( theWin );
						InvalWindowRect( theWin, &r );
						break;
					}

					case inZoomIn:
					case inZoomOut:
						if( TrackBox( theWin, theEvent->where, windowCode ) )
						{
							GetWindowPortBounds( theWin, &winRect );
							SetPortWindowPort( theWin );
							EraseRect( &winRect );

							ZoomWindow( theWin, windowCode, true );
							AdjustScrollBars( theWin, true );

							// Redraw the theWin
							GetWindowPortBounds( theWin, &winRect );
							SetPortWindowPort( theWin );
							InvalWindowRect( theWin, &winRect );
						}
				}
			}
			break;
		}
		break;
		
	// Was a key pressed?
	case keyDown: 
	case autoKey:
		// Was the cmd-key being held down?	If so, process menu bar short cuts.
		if( ( theEvent->modifiers & cmdKey ) != 0 )
		{
			AdjustMenus();
			HandleMenu( MenuKey( (char) (theEvent->message & charCodeMask) ) );
		}
		else
		{
			theWin = FrontNonFloatingWindow();	//LR: 1.66 don't use NULL window!
			if( theWin )
			{
				windowKind = GetWindowKind( theWin );
				objectWindow = (ObjectWindowPtr)GetWRefCon( theWin );
// LR: v1.6.5 WHAT THE???			objectWindow->Dispose( theWin );
				if( windowKind == kHexEditWindowTag && objectWindow->ProcessKey != NULL )
					objectWindow->ProcessKey( theWin, theEvent );
			}
		}
		break;

	// Does a theWin need to be redrawn?
	case updateEvt:
	{
		theWin = (WindowRef) theEvent->message;

		objectWindow = (ObjectWindowPtr) GetWRefCon( theWin );
		if( GetWindowKind( theWin ) == kHexEditWindowTag && objectWindow->Update )
			objectWindow->Update( theWin );
		break;
	}

	// Has a theWin been activated or deactivated?
	case activateEvt:
		theWin = (WindowRef) theEvent->message;

		// Force it to be redrawn
		objectWindow = (ObjectWindowPtr)GetWRefCon( theWin );
		if( GetWindowKind( theWin ) == kHexEditWindowTag && objectWindow->Activate )
			objectWindow->Activate( theWin, (theEvent->modifiers & activeFlag) > 0 );

		break;

	case osEvt:
		// Force it to be redrawn
		switch ( theEvent->message >> 24 )
		{
			case suspendResumeMessage:
				theWin = FrontNonFloatingWindow();
				if( theWin )
				{
					objectWindow = (ObjectWindowPtr) GetWRefCon( theWin );
					windowKind = GetWindowKind( theWin );
					if( windowKind == kHexEditWindowTag && objectWindow->Activate )
						objectWindow->Activate( theWin, (theEvent->message & resumeFlag) > 0 );
				}
				if( theEvent->message & resumeFlag && ( CompWind1 && CompWind2 ) )
					DoComparison();
				break;
		}
		break;		

	case kHighLevelEvent:
#if !TARGET_API_MAC_CARBON	// LR: v1.6
		if( g.sys7Flag )
#endif
			AEProcessAppleEvent( theEvent );
		break;
	}
	return noErr;
}

// Do Idle Time Processing

/*** IDLE OBJECTS ***/
OSStatus IdleObjects( EventRecord *er )
{
	WindowRef theWin = FrontNonFloatingWindow();
	ObjectWindowPtr objectWindow;

	while( theWin )
	{
		objectWindow = (ObjectWindowPtr)GetWRefCon( theWin );	//LR: 1.66 - don't use NULL window, and update for all windows!

		if( GetWindowKind( theWin ) == kHexEditWindowTag && objectWindow->Idle )
			objectWindow->Idle( theWin, er );

		theWin = GetNextWindow( theWin );
	}
	return( noErr );
}

/*** GOT REQUIRED PARAMS ***/
static Boolean GotRequiredParams( const AppleEvent *theEvent )
{
	DescType returnedType;
	Size 	actualSize;
	OSErr	err = AEGetAttributePtr( theEvent, keyMissedKeywordAttr, typeWildCard, &returnedType, NULL, 0, &actualSize );
	return err == errAEDescNotFound;
}

/*** DO OPEN EVENT ***/
static OSStatus DoOpenAppleEvent( const AppleEvent *theEvent, Boolean print )
{
	OSStatus	error;
	Handle		docList = NULL;
	FSSpec		myFSS;
	AEDescList	theList;
	AEKeyword	aeKeyword = keyDirectObject;
	long		itemCount, i;
	DescType	actualType;
	Size		actualSize;
	
	// get event description
	error = AEGetParamDesc( theEvent, keyDirectObject, typeAEList, &theList );
	if( error != noErr )
	{
		DEBUGSTR( "\pAEGetParamDesc" );
		return( error );
	}

	// make sure event description is correct
	if( !GotRequiredParams( theEvent ) )
	{
		DEBUGSTR( "\pGotRequiredParams" );
		return( error );
	}

	// count items to open
	error = AECountItems( &theList, &itemCount );
	if( error != noErr )
	{
		DEBUGSTR( "\pAECountItems" );
		return( error );
	}

	// open all items
	for( i = 1; i <= itemCount; i++ )
	{
		error = AEGetNthPtr( &theList, i, typeFSS, &aeKeyword, &actualType, (Ptr) &myFSS, sizeof( FSSpec ), &actualSize );
		if( error == noErr )
		{
			if( noErr == OpenEditWindow( &myFSS, false ) )
			{
				if( print && kHexEditWindowTag == GetWindowKind( FrontNonFloatingWindow() ) )	// LR: 1.7 -- allow printing documents
				{
					PrintWindow( (EditWindowPtr)GetWRefCon( FrontNonFloatingWindow() ) );
					CloseEditWindow( FrontNonFloatingWindow() );
				}
			}
		}
	}
	AdjustMenus();	//LR: 1.7 -- show change in menus if required

	// event was handled successfully
	AEDisposeDesc( &theList );
	return noErr;
}

/*** CORE EVENT HANDLER ***/
static pascal OSErr CoreEventHandler( const AppleEvent *theEvent, AppleEvent *reply, long refCon )
{
	#pragma unused( reply, refCon )	// LR
	DescType	actualType;
	Size		actualSize;
	DescType	eventID;
	OSErr		error;

	error = AEGetAttributePtr( 	( AppleEvent* ) theEvent, keyEventIDAttr, typeType, &actualType, (Ptr) &eventID, sizeof( eventID ), &actualSize );
	if( error )
		return error;
								
	switch( eventID )
	{
		case kAEOpenApplication:
			if( GotRequiredParams( theEvent ) )
			{
				if( !skipOpen )
					AskEditWindow();	// LR: -- voodoo can't have this
			}
			break;
				
		case kAEOpenDocuments:
			DoOpenAppleEvent( theEvent, false );
			break;
				
		case kAEPrintDocuments:
			DoOpenAppleEvent( theEvent, true );
			break;
			
		case kAEQuitApplication:
			if( GotRequiredParams( theEvent ) )
				g.quitFlag = true;
			break;
	}
	return noErr;
}

// LR --- handles only the VOODOO compare files AE

/*** COMPARE EVENT HANDLER ***/
static pascal OSErr CompareEventHandler( const AppleEvent *theEvent, AppleEvent *reply, long refCon )
{
	#pragma unused( reply, refCon )			// LR
	extern WindowRef CompWind1, CompWind2;	// set in OpenEditWindow

	DescType	actualType;
	Size		actualSize;
	OSErr		error;
	FSSpec	oldSpec, newSpec;
	Boolean result;

	skipOpen = true;

	// close previous file compare windows if open

	if( CompWind1 )	DisposeEditWindow( CompWind1 );
	if( CompWind2 )	DisposeEditWindow( CompWind2 );
	CompWind1 = CompWind2 = NULL;

	error = AEGetParamPtr( theEvent, kAENewFileParam, typeFSS, &actualType, &newSpec, sizeof( newSpec ), &actualSize );
	if( error ) return error;

	error = AEGetParamPtr( theEvent, kAEOldFileParam, typeFSS, &actualType, &oldSpec, sizeof( oldSpec ), &actualSize );
	if( error ) return error;

	CompareFlag = 1;
	error = OpenEditWindow( &newSpec, false );
	if( !error )
	{
		CompareFlag = 2;
		error = OpenEditWindow( &oldSpec, false );
		if( !error )
		{
			if( prefs.searchType == CM_Match )
				result = PerformTextMatchCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );
			else
				result = PerformTextDifferenceCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );

			if( result )
			{
				error = AEPutParamPtr( reply, keyDirectObject, typeBoolean, &result, sizeof( result ) );
				return error;
			}

			if( CompWind2 )
				DisposeEditWindow( CompWind2 );
		}
		if( CompWind1 )
			DisposeEditWindow( CompWind1 );
	}

	CompWind1 = CompWind2 = NULL;
	result = false;
	error = AEPutParamPtr( reply, keyDirectObject, typeBoolean, &result, sizeof( result ) );
	return error;
}

/*** INIT APPLE EVENTS ***/
OSStatus InitAppleEvents( void )
{
#if !TARGET_API_MAC_CARBON	// LR: v1.7 (no need for seperate non-carbon code)
	if( g.sys7Flag )
#endif
	{
		AEHandlerUPP = NewAEEventHandlerUPP( CoreEventHandler );
//LR: 1.7 -- Carbon requires specific handlers!		AEInstallEventHandler( kCoreEventClass, typeWildCard, AEHandlerUPP, 0, false );

		AEInstallEventHandler( kCoreEventClass, kAEOpenApplication, AEHandlerUPP, 0, false );
		AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments, AEHandlerUPP, 0, false );
		AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments, AEHandlerUPP, 0, false );
		AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, AEHandlerUPP, 0, false );

		AECompareHandlerUPP = NewAEEventHandlerUPP( CompareEventHandler );
		AEInstallEventHandler( kCompareEventClass, kAECompareEvent, AECompareHandlerUPP, 0, false );

		trackActionUPP = NewControlActionUPP( MyScrollAction );
	}
	return noErr;
}

#if !TARGET_API_MAC_CARBON	// LR: v1.6

/*** CHECK ENVIRONMENT ***/
OSStatus CheckEnvironment( void )
{
	SysEnvRec	sEnv;
	OSStatus	error;

	error = SysEnvirons( 1, &sEnv );

	g.sys7Flag = sEnv.systemVersion >= 0x0700;
	g.colorQDFlag = sEnv.hasColorQD;
	return error;
}

#endif
