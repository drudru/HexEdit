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
 *      Brian Bergstrand
 */

// 05/10/01 - GAB: MPW environment support
#ifdef __MPW__
#include "MPWIncludes.h"
#endif

#include "EditScrollbar.h"
#include "EditRoutines.h"
#include "HexEdit.h" //BB: bring in globals g;

static ControlActionUPP _trackActionUPP = NULL;		//LR 1.73 -- properly local, must be NULL ast startup!

#define GET_LINE(x) ((x + kBytesPerLine - 1) / kBytesPerLine) 
#define TOTAL_LINES GET_LINE(dWin->fileSize)
#define NON_VIEWABLE_LINES (TOTAL_LINES - dWin->linesPerPage)
#define LIMIT_CALC (NON_VIEWABLE_LINES * kBytesPerLine)

#define S_INT16_MAX	0x7FFF	// maximum value to be used in an SInt16

/*** Calc Scroll Position ***/
//LR 1.73 -- simplify some code
static long _calcScrollPosition( EditWindowPtr dWin )
{
	long	newPos, h;
	Rect	winRect;
	short	vPos = GetControlValue( dWin->vScrollBar );
	float	numer;
	float	denom;
	float	result;

	GetWindowPortBounds( dWin->oWin.theWin, &winRect );
	h = winRect.bottom - winRect.top - (kGrowIconSize - 1) - (kHeaderHeight - 1);

	if (NON_VIEWABLE_LINES < S_INT16_MAX)
		newPos = vPos * kBytesPerLine;
	else
	{	numer = vPos;
		denom = S_INT16_MAX;
		result = numer / denom;
		newPos = result * NON_VIEWABLE_LINES * kBytesPerLine;
	}

	return( newPos );
}


/*** MY SCROLL ACTION ***/
static pascal void _scrollAction( ControlHandle theControl, short thePart )
{
	long			curPos, newPos;
	short			pageWidth;
	EditWindowPtr	dWin;		//LR 1.73 -- need window info for live scrolling
//1.72	Rect		myRect;

	dWin = (EditWindowPtr)GetControlReference( theControl );	//LR 1.73 -- get owning window

	curPos = dWin->editOffset;
	newPos = curPos;

/*LR 1.72 -- not used!
	GetWindowPortBounds( dWin->oWin.theWin, &myRect );

	myRect.right -= kSBarSize-1;
	myRect.bottom -= kSBarSize-1;
*/
	pageWidth = (dWin->linesPerPage - 1) * kBytesPerLine;

	switch( thePart )
	{
		case kControlUpButtonPart:		newPos = curPos - kBytesPerLine;		break;	// LR: -- UH compliant
		case kControlDownButtonPart:	newPos = curPos + kBytesPerLine;		break;
		case kControlPageUpPart:		newPos = curPos - pageWidth;	break;
		case kControlPageDownPart:		newPos = curPos + pageWidth;	break;
		case kControlIndicatorPart:		newPos = _calcScrollPosition( dWin );	break;	//LR 1.73 -- live scrolling
	}

	ScrollToPosition( dWin, newPos );
}

/*** SETUP SCROLL BARS ***/
void SetupScrollBars( EditWindowPtr dWin )
{
	Rect	sRect, r;

	//LR 1.73 -- setup track action proc if not already done
	if( !_trackActionUPP )
		_trackActionUPP = NewControlActionUPP( _scrollAction );

	GetWindowPortBounds( dWin->oWin.theWin, &r );

	sRect.left = r.right - ( kSBarSize - 1 );
	sRect.top = r.top + kHeaderHeight;	// NS: move to below header
	sRect.right = r.right + 1;
	sRect.bottom = r.bottom	- kGrowIconSize;
	// BB: detect Appearance manager, and create a live scroll bar if we do
    dWin->vScrollBar = NewControl( dWin->oWin.theWin, &sRect, "\p", true, 0, 0, sRect.bottom - sRect.top, g.useAppearance ? kControlScrollBarLiveProc : scrollBarProc, 1L );
	AdjustScrollBars( dWin->oWin.theWin, 1 );

	//LR 1.73 -- save window for callback procedure
	SetControlReference( dWin->vScrollBar, (SInt32)dWin );
}

// Adjust scroll bars when they need to be redrawn for some reason.
// resizeFlag is an optimization to avoid extra work when you aren't resizing.

/*** ADJUST SCROLL BARS ***/
void AdjustScrollBars( WindowRef theWin, short resizeFlag )
{
	short			h;
	GrafPtr			savePort;
	long			limit;
	Rect			r;
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( theWin );
	short			maxValue;
	unsigned long	curLine;
	short			curValue;
	float			ratio;
	float			numer;
	float			denom;

	GetWindowPortBounds( theWin, &r );

	GetPort( &savePort );
	
	h = (r.bottom - r.top) - (kGrowIconSize - 1) - (kHeaderHeight - 1);

	if( resizeFlag )
	{
		// Adjust Lines Per Page
//LR: 1.7 -fix lpp calculation!		dWin->linesPerPage = ( ( ( r.bottom - kSBarSize ) - ( kHeaderHeight + 1 ) - TopMargin - BotMargin ) / kLineHeight );
		dWin->linesPerPage = (((r.bottom - r.top - (kLineHeight - 1)) + kLineHeight - kHeaderHeight) / kLineHeight);

		// Move sliders to new position
		// LR: per Shanks, control below theWin's header
		MoveControl( dWin->vScrollBar, r.right - (kSBarSize - 1), r.top + (kHeaderHeight - 1) );	

		// Change their sizes to fit new theWin dimensions
		SizeControl( dWin->vScrollBar, kSBarSize, h );
	}

	// Reposition painting if you have resized or scrolled past the legal
	// bounds	Note: this call will usually be followed by an update
	limit = LIMIT_CALC;
//LR 1.72	limit = ((dWin->fileSize + 15) & 0xFFFFFFF0) - (dWin->linesPerPage / kBytesPerLine);
	if( dWin->editOffset > limit )
		dWin->editOffset = limit;
	if( dWin->editOffset < 0 )
		dWin->editOffset = 0;
		

	// Set the value of the sliders accordingly
	if( limit > 0 )
	{
		curLine = GET_LINE(dWin->editOffset);
		
		if (NON_VIEWABLE_LINES > S_INT16_MAX)
		{
			numer = curLine;
			denom = NON_VIEWABLE_LINES;
			ratio = numer / denom;
			maxValue = (NON_VIEWABLE_LINES > S_INT16_MAX) ? S_INT16_MAX : NON_VIEWABLE_LINES;
			
			curValue = (ratio * maxValue);
		}
		else
		{
			maxValue = NON_VIEWABLE_LINES;
			curValue = curLine;
		}
		
		SetControlMaximum( dWin->vScrollBar, maxValue);
			
#if !defined(__MC68K__) && !defined(__SC__)		//LR 1.73 -- not available for 68K (won't even link!)
		// BB: Set up proportional scroll bar if we can
        if (SetControlViewSize != (void*)kUnresolvedCFragSymbolAddress)
            SetControlViewSize( dWin->vScrollBar, dWin->linesPerPage);
#endif
		SetControlValue( dWin->vScrollBar, curValue);
	}
	else
	{
		SetControlMaximum( dWin->vScrollBar, 0 );
		SetControlValue( dWin->vScrollBar, 0 );
	}
		
	SetPort( savePort );
}

// Intercept Handler for scroll bars
// Returns true if user clicked on scroll bar

/*** HANDLE SCROLL BAR CLICK ***/
Boolean MyHandleControlClick( WindowRef window, Point mouseLoc )
{
	short 			controlPart;
	ControlRef		control;
	EditWindowPtr	dWin = (EditWindowPtr) GetWRefCon( window );

	// NS: v1.6.6, new scrolling code to enable live scrolling on post-Appearance systems
	controlPart = FindControl( mouseLoc, window, &control );
	if( control == nil ) return false;
	
	// scroll the window	-- use old bits for now
/*	if( controlPart == kControlIndicatorPart && !g.useAppearance )	// in thumb (129)
	{
		TrackControl( control, mouseLoc, nil );
		::DrawBody( window );
	}
	else										// in arrows or page up/down
	{
		scrollAction = NewControlActionProc( FileScrollAction );
		TrackControl( control, mouseLoc, scrollAction );
		DisposeRoutineDescriptor( scrollAction );
	}
*/

	// Use default behavior for thumb, program will crash if you don't!!
	if( kControlIndicatorPart == controlPart && !g.useAppearance )
	{
	    // BB: Perform scrollbar tracking
	    controlPart = TrackControl( control, mouseLoc, 0L );
		if( !controlPart )
			return false;

		if( controlPart == kControlIndicatorPart )
		{
			ScrollToPosition( dWin, _calcScrollPosition( dWin ) );
		}
	}
	else
	{
	    // Perform scrollbar tracking
	    controlPart = TrackControl( control, mouseLoc, _trackActionUPP );
	    if( !controlPart )
	    	return false;
	    // BB: all scrolling handled by _scrollAction()
	}						

	return true;
}

/*** SCROLL TO SELECTION ***/
void ScrollToSelection( EditWindowPtr dWin, long pos, Boolean forceUpdate, Boolean centerFlag )
{
	long	curAddr;
	curAddr = dWin->editOffset;

	if( pos >= curAddr && pos < curAddr + (dWin->linesPerPage * kBytesPerLine) )
	{
		if( forceUpdate )
		{
			DrawPage( dWin );
		}
		UpdateOnscreen( dWin->oWin.theWin );
		AdjustScrollBars( dWin->oWin.theWin, false );
		return;
	}

	if( centerFlag )
	{
		curAddr = pos - ( pos % kBytesPerLine );
		curAddr -= kBytesPerLine * ((dWin->linesPerPage / 2) - 1);
		// No need to adjust for limits, will be done by scroll routine
	}
	else
	{
		if( pos < curAddr )
		{
			// Scroll Up
			curAddr = pos;
			curAddr -= (curAddr % kBytesPerLine);
		}
		else
		{
			// Scroll Down
			curAddr = pos - (dWin->linesPerPage - 1) * kBytesPerLine;
			curAddr -= (curAddr % kBytesPerLine);
		}
	}
	ScrollToPosition( dWin, curAddr );
}

/*** SCROLL TO POSITION ***/
void ScrollToPosition( EditWindowPtr dWin, long newPos )
{
	long	limit;

	SetPortWindowPort( dWin->oWin.theWin );

	// Constrain scrolling position to legal limits
		limit = LIMIT_CALC;
//LR 1.72	limit = ((dWin->fileSize + (kBytesPerLine - 1)) & 0xFFFFFFF0) - (dWin->linesPerPage / kBytesPerLine);
	if( newPos > limit )
		newPos = limit;
	if( newPos < 0 )
		newPos = 0;

	// LR: v1.6.5 reduce auto-scroll flicker (Max Horn)
	if( newPos != dWin->editOffset )
	{
		dWin->editOffset = newPos;
		
		// Adjust Scrollbars
		AdjustScrollBars( dWin->oWin.theWin, false );

		// 12/10/93 - Optimize Drawing
		SetCurrentChunk( dWin, dWin->editOffset );

		DrawPage( dWin );
		UpdateOnscreen( dWin->oWin.theWin );
	}
}

/*** AUTO SCROLL ***/
void AutoScroll( EditWindowPtr dWin, Point pos )
{
	short offset;
	if( pos.v < ( kHeaderHeight + 1 ) )
		offset = -kBytesPerLine;
	else if( pos.v >= ( kHeaderHeight + 1 ) + dWin->linesPerPage * kLineHeight )
		offset = kBytesPerLine;
	else
		return;

	ScrollToPosition( dWin, dWin->editOffset+offset );
}