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
 * Copyright (C) Copyright © 1996-2002.
 * All Rights Reserved.
 *
 * Modified: $Date$
 * Revision: $Id$
 *
 * Contributor(s):
 *		Lane Roathe
 *		Nick Shanks
 */

// 05/10/01 - GAB: MPW environment support
#ifdef __MPW__
#include "MPWIncludes.h"
#endif

#include <stdio.h>
#if __MWERKS__
	#define DEBUG 1
	#include <debugging.h>	// LR: v1.6.5, via Max Horn - safe on systems w/o debugger
#else
//    #include <CarbonCore/Debugging.h>
#endif

#include "Utility.h"

/*** SET CONTROL ***/
void SetControl( DialogPtr dialog, short item, short value )
{
	Handle handle;
	short type;
	Rect r;

	GetDialogItem( dialog, item, &type, &handle, &r );
	SetControlValue( ( ControlHandle ) handle, value );
}

// Copy a pascal string to a dialog's text item ( static text or edit text )

/*** SET TEXT ***/
void SetText( DialogPtr dialog, short item, StringPtr text )
{
	Handle	handle;
	short	type;
	Rect	r;

	GetDialogItem( dialog, item, &type, &handle, &r );
	SetDialogItemText( handle, text );
}

// Copy a dialog's text item to a pascal string.

/*** GET TEXT ***/
void GetText( DialogPtr dialog, short item, StringPtr text )
{
	Handle	handle;
	short	type;
	Rect	r;

	GetDialogItem( dialog, item, &type, &handle, &r );
	GetDialogItemText( handle, (StringPtr) text );
}

/*** GET RECT ***/
void GetRect( DialogPtr dialog, short item, Rect *r )
{
	Handle	handle;
	short	type;

	GetDialogItem( dialog, item, &type, &handle, r );
}

/*** SET DRAW PROC ***/
void SetDraw( DialogPtr dialog, short item, Handle proc )
{
	Handle	handle;
	short	type;
	Rect	r;

	GetDialogItem( dialog, item, &type, &handle, &r );
	SetDialogItem( dialog, item, type, proc, &r );
}

/*** DISABLE BUTTON ***/
void DisableButton( DialogPtr dialog, short bid )
{
	Handle	h;
	short	t;
	Rect	r;

// LR: v1.6.5,	SetDialogDefaultItem( dialog, bid );
	GetDialogItem( dialog, bid, &t, &h, &r );
	HiliteControl( ( ControlHandle ) h, kControlDisabledPart );
}

/*** ENABLE BUTTON ***/
void EnableButton( DialogPtr dialog, short bid )
{
	Handle	h;
	short	t;
	Rect	r;

// LR: v1.6.5,	SetDialogDefaultItem( dialog, bid );
	GetDialogItem( dialog, bid, &t, &h, &r );
	HiliteControl( ( ControlHandle ) h, kControlNoPart );
}

/*	*** Check for an abort keypress (ESC or Command .) ***

	LR: 1.66 - added to allow any routine to check for an abort keypress.

	EXIT: true == user pressed abort key(s)
*/

Boolean CheckForAbort( void )
{
	EventRecord event;

	if( EventAvail( keyDownMask, &event ) )	// GetNextEvent will NOT work (doesn't see the key press!)
	{
		if( keyDown == event.what )		// should always be true, but ...
		{
			do	//LR 186 -- check avail and all other events for abort press (other no longer worked!)
			{
				char c = (event.message & charCodeMask);

				// We allow both ESC and Cmd-'.' to abort
			if( 27 == c || ('.' == c && (event.modifiers & cmdKey)) )
				{
					FlushEvents( everyEvent, NULL );	// don't leave any events on queue if aborting!
					return( true );
				}
			}while( GetNextEvent( everyEvent, &event ) );
		}
	}
	return( false );
}

// Simulate the user pressing a button.	This is used to give the user some
// visual feedback when they use the keyboard as a shortcut to press a dialog button.

/*** SIMULATE BUTTON PRESS ***/
void SimulateButtonPress( DialogPtr dialog, short bid )
{
	Handle	h;
	short	t;
	Rect	r;
	unsigned long delay;
	GrafPtr	gp;

	GetPort( &gp );
	SetPortDialogPort( dialog );

	GetDialogItem( dialog, bid, &t, &h, &r );
	HiliteControl( ( ControlHandle ) h, kControlButtonPart );
	Delay( 10, &delay );
	HiliteControl( ( ControlHandle ) h, kControlNoPart );
	SetPort( gp );
}

/*** ERROR ALERT ***/
// LR: v1.6.5, modified to accept STR# index for localization
short ErrorAlert( short severity, short strid, ... )
{
	char 	tbuf[256], str[256];
	short	itemHit;
	va_list args;
	AlertType inAlertType;

	GetIndString( (StringPtr)str, strError, strid );
	CopyPascalStringToC( (StringPtr)str, str );

	va_start( args, strid );
	vsprintf( tbuf, str, args );
	va_end( args );

	CopyCStringToPascal( tbuf, (StringPtr)tbuf );

	if (g.appearanceAvailable)
	{
		AlertStdAlertParamRec params;

		switch (severity)
		{
		case ES_Note:
			inAlertType  = kAlertNoteAlert;
			break;

		case ES_Caution:
			inAlertType  = kAlertCautionAlert;
			break;

		case ES_Stop:
		case ES_Fatal:
		default:
			inAlertType  = kAlertStopAlert;
			break;
		}

		//LR 180 -- I want alerts to belong to the error window!
		memset( &params, 0, sizeof(params) );
		params.defaultText = "\pBummer!";
		params.defaultButton = ok;
		params.position = kWindowAlertPositionParentWindow;
		
		StandardAlert( inAlertType, (StringPtr)tbuf, NULL, &params, &itemHit );
	}
	else
	{
		ParamText( (StringPtr) tbuf, NULL, NULL, NULL );
		InitCursor();
		switch( severity )
		{
		case ES_Note:
			itemHit = NoteAlert( alertError, NULL );
			break;
			
		case ES_Caution:
			itemHit = CautionAlert( alertError, NULL );
			break;

		case ES_Stop:
		case ES_Fatal:
			itemHit = StopAlert( alertError, NULL );
			break;
		}
	}

	// LR: v1.6.5, -- No "Debug" buttons, but OPTION on any goes into debugger now!
	{
		KeyMap keys;

		GetKeys( keys );
		if( keys[1] & (1<<2) )

// 05/10/01 - GAB: DEBUGSTR not defined for non-Carbon builds
#if __MWERKS__ && TARGET_API_MAC_CARBON
			debug_string( (StringPtr) tbuf );	//LR -- 1.76 DEBUGSTR is out, this is now in. Sigh.
//			DEBUGSTR( (StringPtr) tbuf );
#else
			DebugStr( (StringPtr) tbuf );
#endif
	}

	if(  severity == ES_Fatal )
		ExitToShell();
	
	return itemHit;
}

/*** OS ERROR ALERT ***/
/* LR: v1.6.5, removed due to redundancy and localization
short OSErrorAlert( short severity, short strid, short error )
{
	Str255 s1, s2;


	GetIndString( s, strError, errOS );
	return ErrorAlert( severity, s, strid, error );
}
*/

/*** MY RANDOM ***/
short MyRandom( short limit )
{
/*
	unsigned long r;
	r = ( unsigned ) Random();
	r = ( r* (long) limit )/65536L;
	return (short) r;
*/
	return Random() % limit;
}

/*** MY SET CURSOR ***/
void MySetCursor( short n )
{
	static short lastN=-1;
	if( n != lastN )
	{
		lastN = n;
		switch( n )
		{
#if TARGET_API_MAC_CARBON
			case C_Arrow:
			{
				Cursor arrow;

				GetQDGlobalsArrow( &arrow );
				SetCursor( &arrow );
				break;
			}
#else
			case C_Arrow:	SetCursor( &qd.arrow );		break;	// LR: qd.
#endif
			// NS: v1.5, changed Curs to CursHandle
			case C_Watch:	SetCursor( *g.watch );		break;
			case C_IBeam:	SetCursor( *g.iBeam );		break;
		}
	}
}

//#if false	// NS: v1.6.6, now in CarbonAccessors.o			!TARGET_API_MAC_CARBON
#if !TARGET_API_MAC_CARBON	//LR -- not if you compile all the projects it's not!!!

/*** COPY PASCAL STRING TO C ***/
void CopyPascalStringToC( ConstStr255Param source, char* dest )
{
	if( source != NULL )
	{
		SInt16 length  = *source++;
		while (length > 0) 
		{
			*dest++ = *(char*) source++;
			--length;
		}
	}
	*dest = '\0';
}

// 	CopyCStringToPascal converts the source C string to a destination
// 	pascal string as it copies. The dest string will
// 	be truncated to fit into an Str255 if necessary.
//  If the C String pointer is NULL,the pascal string's length is set to zero

/*** COPY C STRING TO PASCAL ***/
void CopyCStringToPascal( const char* source, Str255 dest )
{
	SInt16 	length  = 0;
	
	// handle case of overlapping strings
	if( (void*) source == (void*) dest)
	{
		unsigned char*		curdst = &dest[1];
		unsigned char		thisChar;
				
		thisChar = *(const unsigned char*) source++;
		while (thisChar != '\0') 
		{
			unsigned char nextChar;
			
			// use nextChar so we don't overwrite what we are about to read
			nextChar = *(const unsigned char*) source++;
			*curdst++ = thisChar;
			thisChar = nextChar;
			
			if (++length >= 255)
				break;
		}
	}
	else if (source != NULL)
	{
		unsigned char*		curdst = &dest[1];
		SInt16 				overflow = 255;		// count down so test it loop is faster
		register char		temp;
	
		// Can't do the K&R C thing of Òwhile (*s++ = *t++)Ó because it will copy trailing zero
		// which might overrun pascal buffer.  Instead we use a temp variable.
		while( (temp = *source++) != 0 ) 
		{
			*(char*) curdst++ = temp;
				
			if (--overflow <= 0)
				break;
		}
		length = 255 - overflow;
	}
	dest[0] = length;
}

#endif

/*** EQUAL PASCAL STRINGS ***/
Boolean EqualPStrings( UInt8 *source, UInt8 *dest )
{
	UInt8 length = *source, count;
	for( count = 0; count <= length; count++ )
		if( *source++ != *dest++ ) return false;
	return true;
}

#ifndef __MC68K__

/*** LAUNCH WEB BROWSER ***/
OSStatus LaunchURL( StringPtr url )
{
	OSStatus error = noErr;

	if( ICLaunchURL )	//LR weak linked!
	{
		long start = 1, end = 1, length = (long)url[0];

		error = ICLaunchURL( g.icRef, NULL, url+1, length, &start, &end );
	}
	return error;
}

#endif
