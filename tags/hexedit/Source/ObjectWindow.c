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

#include "ObjectWindow.h"

/*** INIT OBJECT WINDOW ***/
WindowRef InitObjectWindow( short resID, ObjectWindowPtr objectWindow, Boolean isFloating )
{
	if( !objectWindow )
	{
		objectWindow = (ObjectWindowPtr) NewPtrClear( sizeof(ObjectWindowRecord) );
		if( !objectWindow ) return NULL;
		objectWindow->ownStorage = true;
	}
	else
		objectWindow->ownStorage =false;

	objectWindow->floating = false;
	objectWindow->theWin = GetNewCWindow( resID, NULL, (WindowRef) -1 );
	SetWindowKind( objectWindow->theWin, HexEditWindowID );			// new storage location (not in RefCon)
	SetWRefCon( objectWindow->theWin, (long)objectWindow );	// new storage to stop "(ObjectWindowPtr) theWindowPtr"

	objectWindow->Update = DefaultUpdate;
	objectWindow->Activate = DefaultActivate;
	objectWindow->HandleClick = DefaultHandleClick;
	objectWindow->Dispose = DefaultDispose;
	objectWindow->Draw = DefaultDraw;
	objectWindow->Idle = NULL;
	objectWindow->floating = isFloating;

	return objectWindow->theWin;
}


/*** DEFAULT DISPOSE ***/
void DefaultDispose( WindowRef theWin )
{
	ObjectWindowPtr objectWindow = (ObjectWindowPtr) GetWRefCon( theWin );

	DisposeWindow( theWin );

	if( objectWindow->ownStorage )
		DisposePtr( (Ptr) theWin );
}

/*** DEFAULT UPDATE ***/
void DefaultUpdate( WindowRef theWin )
{
	ObjectWindowPtr objectWindow = (ObjectWindowPtr) GetWRefCon( theWin );
	GrafPtr	savePort;

	GetPort( &savePort );

	SetPortWindowPort( theWin );
	BeginUpdate( theWin );
	objectWindow->Draw( theWin );
	EndUpdate( theWin );

	SetPort( savePort );
}

/*** DEFAULT ACTIVATE ***/
void DefaultActivate( WindowRef theWin, Boolean active )
{
	Rect winRect;
	ObjectWindowPtr objectWindow = (ObjectWindowPtr) GetWRefCon( theWin );
	GrafPtr	savePort;

	GetPort( &savePort );

	GetWindowPortBounds( theWin, &winRect );
	SetPortWindowPort( theWin );
	InvalWindowRect( theWin, &winRect );
	SetPort( savePort );
	objectWindow->active = active;
}

/*** DEFAULT HANDLE CLICK ***/
void DefaultHandleClick( WindowRef theWin, Point where, EventRecord *er )
{
	#pragma unused( theWin, where, er )	// LR
}

/*** DEFAULT DRAW ***/
void DefaultDraw( WindowRef theWin )
{
	#pragma unused( theWin )	// LR
}