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
 * The Initial Developer of the Original Code is Lane Roathe
 * Portions created by Lane Roathe are
 * Copyright (C) Copyright © 1996-2001.
 * All Rights Reserved.
 * 
 * Contributor(s):
 *		Greg Branche
 */

// 05/10/01 - GAB: MPW environment support
#ifdef __MPW__
#include "MPWIncludes.h"
#endif

#include "AboutBox.h"
#include "Utility.h"

static TEHandle	hTE;
static short endText;
static unsigned long prevTicks;

#define SCROLLDELAY 3

/*** DRAW TE TEXT ***/
pascal void DrawTEText( DialogPtr whichDialog, short itemNr )
{
//	draw the TE text in our user item
	#pragma unused( whichDialog, itemNr )
	TEUpdate( &(*hTE)->viewRect, hTE );
}

/*** DIALOG FILTER ***/
pascal Boolean DialogFilter( DialogPtr whichDialog, EventRecord *event, short *itemHit )
{
//	dialog filter for about box (used to scroll TE contents)
	if( TickCount() - prevTicks >= SCROLLDELAY )		// don't scroll too fast!
	{
		prevTicks = TickCount();

		if( (*hTE)->destRect.bottom >= endText )
			TEScroll( 0, -1, hTE );	// show text slowly
		else
		{
			register short startOffset = (*hTE)->viewRect.bottom - (*hTE)->viewRect.top;
	
			(*hTE)->destRect.top = (*hTE)->viewRect.top + startOffset;	// start over
			(*hTE)->destRect.bottom = (*hTE)->viewRect.bottom + startOffset;
		}
	}
	return StdFilterProc( whichDialog, event, itemHit );
}

/*** HEX EDIT ABOUT BOX ***/
void HexEditAboutBox( void )
{
//	code stolen from ResCon sources
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

	if( ( vr = ( VersRecHndl ) GetResource( 'vers', 1 ) ) != NULL )
	{
		HLock( (Handle) vr );
		verStr = (StringPtr) ( ( ( unsigned long ) &( ** vr ).shortVersion[0] ) );
	}
	else verStr = "\p(unknown version)";

	ParamText( verStr,
#ifdef __MC68K__
				"\p68K"
#elif TARGET_API_MAC_CARBON
				"\pCarbon"
#else
				"\pPPC"
#endif
, NULL, NULL );

	theDialog = GetNewDialog( dlgAbout, NULL, (WindowRef)-1L );
#if TARGET_API_MAC_CARBON
	SetPortDialogPort( theDialog );
#else
	SetPort( theDialog );
#endif

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

	prevTicks = 0;
	done = false;

	InitCursor();
	SetDialogDefaultItem( theDialog, ok );

#if TARGET_API_MAC_CARBON
	if( SetDialogTimeout )
		SetDialogTimeout( theDialog, ok, 60 );

	ShowWindow( GetDialogWindow( theDialog ) );
#else
	ShowWindow( theDialog );
#endif
	
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
