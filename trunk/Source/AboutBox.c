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
 * Copyright (C) Copyright � 1996-2002.
 * All Rights Reserved.
 *
 * Modified: $Date$
 * Revision: $Id$
 *
 * Contributor(s):
 *		Lane Roathe
 *		Greg Branche
 */

// 05/10/01 - GAB: MPW environment support
#ifdef __MPW__
#include "MPWIncludes.h"
#endif

#include "main.h"
#include "AboutBox.h"
#include "Utility.h"

static TEHandle	hTE;
static short endText;
static long long prevTime;

//LR 1.76 -- rewrite entire text scroll routine to do speed based on pixels per second!
#define SCROLLPIXELSPERSECOND 5.0

//LR 1.73 -- make code more readable
#ifdef __MC68K__
	#define PLATFORM_STRING "\p68K"
#elif TARGET_API_MAC_CARBON
	#define PLATFORM_STRING	"\pCarbon"
#else
	#define PLATFORM_STRING	"\pPPC"
#endif

/*** DRAW TE TEXT ***/
//	draw the TE text in our user item
pascal void DrawTEText( DialogPtr whichDialog, short itemNr )
{
	#pragma unused( whichDialog, itemNr )
	TEUpdate( &(*hTE)->viewRect, hTE );
}

/*** DIALOG FILTER ***/
//	dialog filter for about box (used to scroll TE contents)
pascal Boolean DialogFilter( DialogPtr whichDialog, EventRecord *event, short *itemHit )
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
			register short startOffset = (*hTE)->viewRect.bottom - (*hTE)->viewRect.top;
	
			(*hTE)->destRect.top = (*hTE)->viewRect.top + startOffset;
			(*hTE)->destRect.bottom = (*hTE)->viewRect.bottom + startOffset;
		}
	}

	// let the standard dialog filter handle events
	return StdFilterProc( whichDialog, event, itemHit );
}

/*** HEX EDIT ABOUT BOX ***/
//	code stolen from Lane's ResCon sources
void HexEditAboutBox( void )
{
	#define TEITEM		2

	DialogPtr	theDialog;
	Boolean		done;
	short		item;
	Handle		text;
	StScrpHandle style;
	GrafPtr		savePort;
	StringPtr	verStr;
	Rect		bounds;
	VersRecHndl	vr;
	ModalFilterUPP dlgFilterUPP = NewModalFilterUPP( DialogFilter );
	UserItemUPP userItemUPP = NewUserItemUPP( DrawTEText );

	GetPort( &savePort );

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

	theDialog = GetNewDialog( dlgAbout, NULL, kFirstWindowOfClass );
	SetPortDialogPort( theDialog );

	GetRect( theDialog, TEITEM, &bounds );
	hTE = TEStyleNew( &bounds, &bounds );
	TEScroll( 0, (*hTE)->viewRect.bottom - (*hTE)->viewRect.top, hTE );

	style = (StScrpHandle)GetResource( 'styl', dlgAbout );		// try to use a styled text edit for coolness
	text = GetResource ('TEXT', dlgAbout);
	if( text )
	{
		HLock( text );
		TEStyleInsert (&(**text), GetHandleSize (text), style, hTE);	// Style == 0 creates plain text
		ReleaseResource( text );
	}

	TECalText( hTE );
	endText = (*hTE)->destRect.top - TEGetHeight( (*hTE)->nLines, 1, hTE );	// bottom < this == donet

	SetDraw( theDialog, TEITEM, (Handle) userItemUPP );

	Microseconds( (UnsignedWide *)&prevTime );
	done = false;

	SetDialogDefaultItem( theDialog, ok );
	ShowWindow( GetDialogWindow( theDialog ) );

	InitCursor();

	ModalDialog( dlgFilterUPP, &item );

	// LR: 1.66 check for a URL item and launch if so
#ifndef __MC68K__
	if( itemFirstURL <= item )
	{
		Str255 str;
		GetIndString( str, strURLs, item - (itemFirstURL - 1) );
		LaunchURL( str );
	}
#endif

	TEDispose( hTE );
	DisposeDialog( theDialog );
	DisposeModalFilterUPP( dlgFilterUPP );
	DisposeUserItemUPP( userItemUPP );

	SetPort( savePort );
}
