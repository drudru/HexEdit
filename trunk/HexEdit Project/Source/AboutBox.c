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
 * The Initial Developer of the Original Code is Lane Roathe
 * Portions created by Lane Roathe are
 * Copyright (C) Copyright © 1996-2008.
 * All Rights Reserved.
 *
 * Contributor(s):
 *		Lane Roathe
 *		Greg Branche
 */

// 05/10/01 - GAB: MPW environment support
#ifdef __MPW__
#include "MPWIncludes.h"
#endif

#include "Main.h"
#include "ObjectWindow.h"
#include "Utility.h"

#include "AboutBox.h"

/* if not carbon, appearance manager might not be available and we need these */
#if !TARGET_API_MAC_CARBON
	static TEHandle	hTE;
	static short endText;
	static long long prevTime;

	#define TEITEM		2

	//LR 1.76 -- rewrite entire text scroll routine to do speed based on pixels per second!
	#define SCROLLPIXELSPERSECOND 10.0	// NOTE: good to keep in even ticks for update cleanness
	#define BLANKPIXELSPACE 190			//LR 181 -- blank spacing in text for autoscroll use
#endif

#define kFirstURLItem 6	// all the rest must follow WITHOUT BREAKS!

//LR 1.73 -- make code more readable
#ifdef __MC68K__
	#define PLATFORM_STRING "\p68K"
#elif TARGET_API_MAC_CARBON
	#define PLATFORM_STRING	"\pCarbon"
#elif TARGET_CPU_PPC
	#define PLATFORM_STRING	"\pPPC"
#else
	#define PLATFORM_STRING "\pUKNOWN!"
#endif

/* --- new dialog method in carbon makes scrolling much better, esp. in X! */

#if !TARGET_API_MAC_CARBON

/*** DRAW TE TEXT ***/
//	draw the TE text in our user item
static pascal void _updateTextBox( DialogPtr whichDialog, short itemNr )
{
	#pragma unused( itemNr )

	GrafPtr savePort;

	// let the standard dialog filter handle events
	GetPort( &savePort );
	SetPortDialogPort( whichDialog );

	TEUpdate( &(*hTE)->viewRect, hTE );

	SetPort( savePort );
}

/*** DIALOG FILTER ***/
//	dialog filter for about box (used to scroll TE contents)
static pascal Boolean _standardFilter( DialogPtr whichDialog, EventRecord *event, short *itemHit )
{
	long long curTime;
	double elapsed;
	int pixels;

	// Get elapsed time in seconds. (makes the constant definition easier to set)
	Microseconds( (UnsignedWide *)&curTime );
	elapsed = (double)(prevTime - curTime);		// get's us a negative, which is what we want!
	elapsed /= 10000.0;

	// how many pixels should we scroll?
	// In Classic this will usually be 0 or 1, in X is normally 5 or more!
	pixels = (int)(elapsed / (60.0 / SCROLLPIXELSPERSECOND));
	if( pixels )
	{
		Microseconds( (UnsignedWide *)&prevTime );	// reset time check

		if( (*hTE)->destRect.bottom >= endText )
		{
			TEScroll( 0, pixels, hTE );	// show text slowly (causes update event)
		}
		else	// end of credits, start from the top!
		{
			register short startOffset = ((*hTE)->viewRect.bottom - (*hTE)->viewRect.top) - BLANKPIXELSPACE;
	
			(*hTE)->destRect.top = (*hTE)->viewRect.top + startOffset;
			(*hTE)->destRect.bottom = (*hTE)->viewRect.bottom + startOffset;
		}
	}

	// let the standard dialog filter handle events
	return StdFilterProc( whichDialog, event, itemHit );
}

#endif //!TARGET_API_MAC_CARBON

static pascal Boolean _appearanceFilter( DialogPtr whichDialog, EventRecord *event, short *itemHit )
{
	Boolean handled = false;

	// let background windows update
	if( updateEvt == event->what )
	{
		ObjectWindowPtr objectWindow;
		WindowRef theWin = (WindowRef) event->message;

		objectWindow = (ObjectWindowPtr) GetWRefCon( theWin );
		if( GetWindowKind( theWin ) == kHexEditWindowTag && objectWindow->Update )
			objectWindow->Update( theWin );
	}
	else
	{
		GrafPtr savePort;

		// let the standard dialog filter handle events
		GetPort( &savePort );
		SetPortDialogPort( whichDialog );
		handled = StdFilterProc( whichDialog, event, itemHit );
		SetPort( savePort );
	}

	return( handled );
}


/*** HEX EDIT ABOUT BOX ***/
//	code stolen from Lane's ResCon sources
void HexEditAboutBox( void )
{
	DialogPtr	theDialog;
	short		item;
	GrafPtr		savePort;
	StringPtr	verStr;
	VersRecHndl	vr;
	short		dialogID;
	ModalFilterUPP dlgFilterUPP;

#if !TARGET_API_MAC_CARBON
	Handle		text;
	StScrpHandle style;
	Rect		bounds;

	UserItemUPP userItemUPP = NewUserItemUPP( _updateTextBox );
#endif

	/* select which filter proc we want to use */
	if( g.useAppearance )
	{
		dialogID = dlgAbout;
		dlgFilterUPP = NewModalFilterUPP( _appearanceFilter );
	}
#if !TARGET_API_MAC_CARBON
	else
	{
		dialogID = dlgAbout + 1;
		dlgFilterUPP = NewModalFilterUPP( _standardFilter );
	}
#endif

	/* First, get our version information to display in dialog */
	if( ( vr = (VersRecHndl)GetResource( 'vers', 1 )) != NULL )
	{
		HLock( (Handle)vr );
		verStr = (StringPtr)(((unsigned long)&(** vr).shortVersion[0]));
	}
	else
		verStr = "\p(unknown version)";

	ParamText( verStr, PLATFORM_STRING, NULL, NULL );

	if( vr )
		ReleaseResource( (Handle)vr );	//LR 1.73 -- just to be safe

	/* Create the dialog */
	GetPort( &savePort );

	theDialog = GetNewDialog( dialogID, NULL, kFirstWindowOfClass );
	SetPortDialogPort( theDialog );

	/* W/O appearance manager we must create scrolling box ourselves! */
#if !TARGET_API_MAC_CARBON
	if( !g.useAppearance )
	{
		/* Get the text to display */
		GetRect( theDialog, TEITEM, &bounds );
		hTE = TEStyleNew( &bounds, &bounds );
		TEScroll( 0, ((*hTE)->viewRect.bottom - (*hTE)->viewRect.top) - BLANKPIXELSPACE, hTE );

		style = (StScrpHandle)GetResource( 'styl', dlgAbout );		// try to use a styled text edit for coolness
		text = GetResource ('TEXT', dlgAbout);
		if( text )
		{
			HLock( text );
			TEStyleInsert (&(**text), GetHandleSize (text), style, hTE);	// Style == 0 creates plain text
			ReleaseResource( text );
		}

		TECalText( hTE );
		endText = (((*hTE)->destRect.top + BLANKPIXELSPACE) - (TEGetHeight( (*hTE)->nLines, 1, hTE ) - BLANKPIXELSPACE));	// bottom < this == done

		SetDraw( theDialog, TEITEM, (Handle) userItemUPP );

		Microseconds( (UnsignedWide *)&prevTime );
	}
#endif //!TARGET_API_MAC_CARBON

	/* Show the dialog and run it */
	SetDialogDefaultItem( theDialog, ok );
	ShowWindow( GetDialogWindow( theDialog ) );

	InitCursor();

	ModalDialog( dlgFilterUPP, &item );

	// LR: 1.66 check for a URL item and launch if so
#ifndef __MC68K__
	if( kFirstURLItem <= item )
	{
		Str255 str;
		GetIndString( str, strURLs, item - (kFirstURLItem - 1) );
		LaunchURL( str );
	}
#endif

#if !TARGET_API_MAC_CARBON
	if( !g.useAppearance )
	{
		TEDispose( hTE );
		DisposeUserItemUPP( userItemUPP );
	}
#endif //!TARGET_API_MAC_CARBON

	DisposeModalFilterUPP( dlgFilterUPP );

	DisposeDialog( theDialog );

	SetPort( savePort );
}
