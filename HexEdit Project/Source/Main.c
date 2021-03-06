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
 * Copyright (C) Copyright � 1996-2008.
 * All Rights Reserved.
 *
 * Contributor(s):
 *		Lane Roathe (LR)
 *		Nick Shanks (NS)
 *		Scott E. Lasley (SEL) 
 *		Brian Bergstrand (BB) 
 *		Eric Froemling (EF)
 */

// 05/10/01 - GAB: MPW environment support
#ifdef __MPW__
#include "MPWIncludes.h"
#endif

#ifdef __MWERKS__
    #include <debugging.h>
#else
    //#include <FlatCarbon/Debugging.h>
    //#include <CoreServices/CoreServices.h>
#endif

#include "Main.h"
#include "Menus.h"
#include "Prefs.h"
#include "Utility.h"
#include "EditWindow.h"
#include "EditRoutines.h"
#include "EditScrollbar.h"
#include "HexCompare.h"
#include "HexSearch.h"

// set up global & gPrefs structures here
globals g;
prefs_t gPrefs;

// LR: --- These are used to setup new UPP stuff ---
AEEventHandlerUPP AEHandlerUPP, AECompareHandlerUPP;

#if !TARGET_API_MAC_CARBON	// LR: v1.6
	extern DlgHookUPP myGetFileUPP;
#endif

extern WindowRef	CompWind1, CompWind2;
static SInt32		caretTime;
static Boolean		skipOpen = false;

#define kOpenFileAtLaunchDelay 3		//LR 187
static UInt32		checkForOpen = 0;

// 05/10/01 - GAB: qd must be supplied by Interface.o applications
#if !__MWERKS__ && !TARGET_API_MAC_CARBON
QDGlobals	qd;
#endif

// Do Idle Time Processing

/*** IDLE OBJECTS ***/
static OSStatus _idleObjects( EventRecord *er )
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
	else								_idleObjects( &theEvent );

	//LR 187 -- apple events came in after the open app event, so we delay a while now.
	if( checkForOpen )
	{
		if( !--checkForOpen && !skipOpen && gPrefs.dialogAtLaunch )	//LR 192 -- allows skipping at open
			AskEditWindow( kWindowNormal );	// LR: -- voodoo can't have this
	}

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
				HandleMenu( MenuSelect( theEvent->where ), theEvent->modifiers );
				break;

			default:			// Cursor was inside our theWin
			{
				switch ( windowCode )
				{
					case inContent:	//EF 180 -- correctly handle selecting windows
					{
						// If the theWin isn't in the front
						if( theWin != FrontNonFloatingWindow() )
						{
							// Make it so...
							SelectWindow( theWin );
							AdjustMenus();
						}
						else	// Window is already in the front, handle the click
						{
							windowKind = GetWindowKind( theWin );
							objectWindow = (ObjectWindowPtr) GetWRefCon( theWin );

							if( windowKind == kHexEditWindowTag && objectWindow->HandleClick )
								objectWindow->HandleClick( theWin, theEvent->where, theEvent );
						}
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
							else if( g.searchDlg && theWin == GetDialogWindow( g.searchDlg ) )		//LR: 1.7 -- compares also need GetDialogWindow!
							{
// LR: v1.6.5					DisposeDialog( g.searchDlg );
// LR: v1.6.5					g.searchDlg = NULL;

								HideWindow( GetDialogWindow( g.searchDlg ) );
							}
							else if( g.gotoDlg && theWin == GetDialogWindow( g.gotoDlg ) )		// LR: v1.6.5
							{
								HideWindow( GetDialogWindow( g.gotoDlg ) );
							}

							AdjustMenus();
						}
						break;

					case inGrow:
					{
						long growResult;
						Rect r;
						EditWindowPtr dWin = (EditWindowPtr)GetWRefCon(theWin);

						SelectWindow( theWin );
						SetRect( &r, kHexWindowWidth + 1, kHeaderHeight + (10 * kLineHeight), kHexWindowWidth + 1, g.maxHeight );

						// Handle the mouse tracking for the resizing
						growResult = GrowWindow( theWin, theEvent->where, &r );

						// Change the size of the theWin
						SizeWindow( theWin, LoWord( growResult ), HiWord( growResult ), true );

						// LR: 1.7 - see if user wants sizing restricted to full lines
						GetWindowPortBounds( theWin, &r );
						if( gPrefs.constrainSize )
						{
							r.bottom = r.top + (short)((((r.bottom - r.top) / kLineHeight) - 1) * kLineHeight) + kHeaderHeight;	// force to full line heights
						}
						SizeWindow( theWin, (r.right - r.left - 1), (r.bottom - r.top), true ); //LR 190 -- don't grow window by 1 pixel

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
			HandleMenu( MenuKey( (char) (theEvent->message & charCodeMask) ), theEvent->modifiers );
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
		{
			Boolean active = (theEvent->modifiers & activeFlag);
			objectWindow->Activate( theWin, active );

			if( active && g.searchDlg )	//LR 1.72 -- if activating, get text from search dialog
			{
				GetText( g.searchDlg, SearchTextItem, g.searchText );
				if( !StringToSearchBuffer( gPrefs.searchCase ) )
				{
					g.searchText[0] = g.searchBuffer[0] = 0;	//LR 1.73 -- on error, keep from repeating!
					SetText( g.searchDlg, SearchTextItem, g.searchText );
				}
			}
		}
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

#pragma mark -

/*** GOT REQUIRED PARAMS ***/
static Boolean _gotRequiredParams( const AppleEvent *theEvent )
{
	DescType returnedType;
	Size 	actualSize;
	OSErr	err = AEGetAttributePtr( theEvent, keyMissedKeywordAttr, typeWildCard, &returnedType, NULL, 0, &actualSize );
	return err == errAEDescNotFound;
}

/*** DO OPEN EVENT ***/
static OSStatus _doOpenAppleEvent( const AppleEvent *theEvent, Boolean print )
{
	OSStatus	error;
//LR 175	Handle		docList = NULL;
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
// 05/10/01 - GAB: DEBUGSTR not defined in non-Carbon builds
#if !TARGET_API_MAC_CARBON
		DEBUGSTR( "\pAEGetParamDesc" );
#endif
		return( error );
	}

	// make sure event description is correct
	if( !_gotRequiredParams( theEvent ) )
	{
// 05/10/01 - GAB: DEBUGSTR not defined for non-Carbon builds
#if !TARGET_API_MAC_CARBON
		DEBUGSTR( "\p_gotRequiredParams" );
#endif
		return( error );
	}

	// count items to open
	error = AECountItems( &theList, &itemCount );
	if( error != noErr )
	{
// 05/10/01 - GAB: DEBUGSTR not defined for non-Carbon builds
#if !TARGET_API_MAC_CARBON
		DEBUGSTR( "\pAECountItems" );
#endif
		return( error );
	}

	// open all items
	for( i = 1; i <= itemCount; i++ )
	{
		error = AEGetNthPtr( &theList, i, typeFSS, &aeKeyword, &actualType, (Ptr) &myFSS, sizeof( FSSpec ), &actualSize );
		if( error == noErr )
		{
			g.forkMode = FM_Smart;	//LR 190 -- open whichever fork we can find!
			if( noErr == OpenEditWindow( &myFSS, kWindowNormal, !print ) )		//LR 181 -- show fork errors, etc. w/drag & drop open (but not print)
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
static pascal OSErr _coreEventHandler( const AppleEvent *theEvent, AppleEvent *reply, long refCon )
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
			if( _gotRequiredParams( theEvent ) )
			{
				checkForOpen = kOpenFileAtLaunchDelay;	// give us time before checking, to allow time to process AE
			}
			break;
				
		case kAEOpenDocuments:
			_doOpenAppleEvent( theEvent, false );
			break;
				
		case kAEPrintDocuments:
			_doOpenAppleEvent( theEvent, true );
			break;
			
		case kAEQuitApplication:
			if( _gotRequiredParams( theEvent ) && CloseAllEditWindows())
				g.quitFlag = true;
			break;
	}
	return noErr;
}

#if (TARGET_API_MAC_CARBON)
//EF 180 -- When we geta mouse wheel event move the croll position
static pascal OSStatus _handleCarbonEvent(EventHandlerCallRef myHandler,EventRef theEvent, void* userData)
{	
	#pragma unused (myHandler,userData)
	
	switch (GetEventClass(theEvent))
	{
		case kEventClassMouse:
		{
			switch (GetEventKind(theEvent))
			{
				case kEventMouseWheelMoved:
				{
					WindowPtr theWindow;
					long curPos,newPos;
					short windowCode;
					EditWindowPtr dWin;
					Point theMousePoint;
					long delta;
					GetEventParameter(theEvent,kEventParamMouseLocation,typeQDPoint,NULL,sizeof(theMousePoint),NULL,&theMousePoint);
					GetEventParameter(theEvent,kEventParamMouseWheelDelta,typeLongInteger,NULL,sizeof(delta),NULL,&delta);
					windowCode = FindWindow ( theMousePoint, &theWindow );
					if (windowCode == inContent)
					{
						if (GetWindowKind(theWindow) == kHexEditWindowTag)
						{
							dWin = (EditWindowPtr)GetWRefCon(theWindow);
							curPos = dWin->editOffset;
							newPos = curPos;
							newPos -= (kBytesPerLine * delta);
							ScrollToPosition( dWin, newPos );	
							return noErr;
						}
					}
					break;
				}
				
			}
		}
	}
	return eventNotHandledErr;
}
#endif //TARGET_API_MAC_CARBON

// LR --- handles only the VOODOO compare files AE

/*** COMPARE EVENT HANDLER ***/
static pascal OSErr _compareEventHandler( const AppleEvent *theEvent, AppleEvent *reply, long refCon )
{
	#pragma unused( reply, refCon )			// LR
//LR 177	extern WindowRef CompWind1, CompWind2;	// set in OpenEditWindow

	DescType	actualType;
	Size		actualSize;
	FSSpec	oldSpec, newSpec;
	OSType	options;	// options are a text field, we get only the first 4 chars to make compares easy!
	Boolean result;
	OSErr		error;
	

	skipOpen = true;

	// close previous file compare windows if open

	if( CompWind1 )	DisposeEditWindow( CompWind1 );
	if( CompWind2 )	DisposeEditWindow( CompWind2 );
	CompWind1 = CompWind2 = NULL;

	// Get the Compare suite events (file list and options)
	error = AEGetParamPtr( theEvent, kAENewFileParam, typeFSS, &actualType, &newSpec, sizeof( newSpec ), &actualSize );
	if( error ) return error;

	error = AEGetParamPtr( theEvent, kAEOldFileParam, typeFSS, &actualType, &oldSpec, sizeof( oldSpec ), &actualSize );
	if( error ) return error;

	//LR 187 -- Options param is optional, and thus no error do we care about
	error = AEGetParamPtr( theEvent, kAECompOptParam, typeChar, &actualType, &options, sizeof( options ), &actualSize );
	if( !error )
	{
		if( 'rsrc' == options || 'RSRC' == options )
			g.forkMode = FM_Rsrc;
		else if( 'data' == options || 'DATA' == options )
			g.forkMode = FM_Data;
		else
			g.forkMode = FM_Smart;		// default is "smart"
	}

	error = OpenEditWindow( &newSpec, kWindowCompareTop, false );
	if( !error )
	{
		error = OpenEditWindow( &oldSpec, kWindowCompareBtm, false );
		if( !error )
		{
/* 185			if( gPrefs.searchType == CM_Match )
				result = PerformTextMatchCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );
			else
				result = PerformTextDifferenceCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );
*/
			//LR 187 -- always return success status if( result )
			//LR 187 -- Check to make sure user didn't try to open the same file twice
			if( CompWind1 && CompWind2 )
			{
				result = PerformTextCompare( (EditWindowPtr) GetWRefCon( CompWind1 ), (EditWindowPtr) GetWRefCon( CompWind2 ) );

				error = AEPutParamPtr( reply, keyDirectObject, typeBoolean, &result, sizeof( result ) );
				return error;
			}
			else
				error = dupFNErr;
		}
	}

	//LR 187 --  cleanup nice on error
	if( CompWind1 )	DisposeEditWindow( CompWind1 );
	if( CompWind2 )	DisposeEditWindow( CompWind2 );

	CompWind1 = CompWind2 = NULL;

//LR 187 -- no result on error!
//	result = false;
//	error = AEPutParamPtr( reply, keyDirectObject, typeBoolean, &result, sizeof( result ) );

	return error;
}

/*** INIT APPLE EVENTS ***/
static OSStatus _initAppleEvents( void )
{
#if !TARGET_API_MAC_CARBON	// LR: v1.7 (no need for seperate non-carbon code)
	if( g.sys7Flag )
#endif
	{
		AEHandlerUPP = NewAEEventHandlerUPP( _coreEventHandler );
//LR: 1.7 -- Carbon requires specific handlers!		AEInstallEventHandler( kCoreEventClass, typeWildCard, AEHandlerUPP, 0, false );

		AEInstallEventHandler( kCoreEventClass, kAEOpenApplication, AEHandlerUPP, 0, false );
		AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments, AEHandlerUPP, 0, false );
		AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments, AEHandlerUPP, 0, false );
		AEInstallEventHandler( kCoreEventClass, kAEQuitApplication, AEHandlerUPP, 0, false );

		AECompareHandlerUPP = NewAEEventHandlerUPP( _compareEventHandler );
		AEInstallEventHandler( kCompareEventClass, kAECompareEvent, AECompareHandlerUPP, 0, false );
	}
	return noErr;
}

#if TARGET_API_MAC_CARBON

/*** INIT CARBON EVENTS ***/
//EF 180 -- handle mouse wheen events via carbon
static OSStatus _initCarbonEvents( void )
{
	long numTypes = 0;
	OSErr err;
	EventTypeSpec  eventType[15];
	EventHandlerUPP theEventHandler = NewEventHandlerUPP(_handleCarbonEvent);
	
	/* specify which events we want to receive */
	eventType[numTypes].eventClass = kEventClassMouse;
	eventType[numTypes].eventKind = kEventMouseWheelMoved;
	numTypes++;
	err = InstallApplicationEventHandler(theEventHandler,numTypes,&eventType[0],NULL,NULL);
	return err;
}
#endif

/*** CHECK ENVIRONMENT ***/
static OSStatus _checkEnvironment( void )
{
	OSStatus	error;

	// get system version	// NS 1.7.1
	error = Gestalt( gestaltSystemVersion, &g.systemVersion );		// this loads HIToolbox.framework on OS X

#if !TARGET_API_MAC_CARBON	// LR: v1.6
	if( !error )
	{
		SysEnvRec	sEnv;

		error = SysEnvirons( 1, &sEnv );
		g.colorQDFlag = sEnv.hasColorQD;
		g.sys7Flag = sEnv.systemVersion >= 0x0700;
	}
#endif

	return error;
}

/*** INIT TOOLBOX ***/
static OSStatus _initToolbox( void )
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
static OSStatus _initMultifinder( void )
{
	g.WNEImplemented = ( NGetTrapAddress(_WaitNextEvent, ToolTrap) != NGetTrapAddress(_Unimplemented, ToolTrap) );
	return noErr;
}
#endif

/*** INIT GLOBALS ***/
static OSStatus _initGlobals( void )
{
	// check for drag manager presence/attributes
	SInt32 result = 0;
	OSStatus error;

	error = Gestalt( gestaltDragMgrAttr, &result );
	g.dragAvailable = (Boolean) (result & (1 << gestaltDragMgrPresent));
	g.translucentDrag = (Boolean) (result & (1 << gestaltDragMgrHasImageSupport));
	
	// check appearance availablilty
	// 05/10/01 - GAB: result is an int, not a pointer
	result = 0;
	error = Gestalt( gestaltAppearanceAttr, &result );
	g.appearanceAvailable = (Boolean) (result & (1 << gestaltAppearanceExists));

	if( g.appearanceAvailable )
	{
		result = 0;
		error = Gestalt( gestaltAppearanceVersion, &result );	//LR 181 -- only use appearance v1.1 or later!
		g.useAppearance = (result >= 0x110);
	}
	else
		g.useAppearance = false;

	if( g.useAppearance )
		RegisterAppearanceClient();
	
	// check nav services availablilty
	// 05/10/01 - GAB: MPW environment support
#if !defined(__MC68K__) && !defined(__SC__)
	g.navAvailable = (Boolean) NavServicesAvailable();
	// BB: check for Navlib version, older versions were very buggy 
	g.useNavServices = ((g.navAvailable) && (NavLibraryVersion() >= 0x0110));

	if( g.useNavServices )
	{
	  if ( NavLoad() != noErr ) // BB: make sure NavLoad returns noErr
	    g.useNavServices = false;
	}    

	// Startup InternetConfig (should only be done once!)
#if !TARGET_API_MAC_CARBON
	if( ICStart )
#endif
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
	g.searchDlg = NULL;
	g.gotoDlg = NULL;
	g.disassemble = false;

	//LR: 1.66 - clear undo/redo records
	memset( &gUndo, sizeof(UndoRecord), 0 );
	memset( &gRedo, sizeof(UndoRecord), 0 );

	return( noErr );
}

/*** MAIN ***/
int main(int argc, char *argv[])	//LR 175
{
#pragma unused(argc,argv)
	// Standard Mac Initialization
	_initToolbox();

#if !TARGET_API_MAC_CARBON	// LR: v1.6
	// Check if Multifinder ( WaitNextEvent ) is implemented
	_initMultifinder();
#endif

	// Check running environment
	_checkEnvironment();

	_initGlobals();
	_initAppleEvents();
	InitMenubar();

	PrefsLoad();
	InitializeEditor();
	AdjustMenus();

#if TARGET_API_MAC_CARBON
	_initCarbonEvents();	//EF 180 -- handle carbon events
#else
	if( !g.sys7Flag )
		AskEditWindow( kWindowNormal );
#endif

	InitCursor();	// NS: v1.6.6, moved here from _initToolbox()
	
#if TARGET_API_MAC_CARBON  // SEL: 1.7 - carbon session based printing
	g.pageFormat = kPMNoPageFormat;
	g.printSettings = kPMNoPrintSettings;
#endif

	// main event loop
	while( !g.quitFlag )
		HandleEvent();

	CleanupEditor();

#if !defined(__MC68K__) && !defined(__SC__)		//LR 1.73 -- not available for 68K (won't even link!)
	// BB: Unload NavServices
	if( g.useNavServices )
		NavUnload();
#endif

// 05/10/01 - GAB: MPW environment support
#if !defined(__MC68K__) && !defined(__SC__)
#if !TARGET_API_MAC_CARBON
	if( ICStop )
#endif
	{
		ICStop( g.icRef );	// shutdown IC
	}
#endif
	return( 0 );
}
