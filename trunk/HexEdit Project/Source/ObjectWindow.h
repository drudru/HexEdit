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

#include "HexEdit.h"

#ifndef _HexEdit_ObjectWindow_
#define _HexEdit_ObjectWindow_

/*** OBJECT WINDOW RECORD ***/
typedef struct
{
	WindowRef		theWin;

	Boolean			ownStorage;
	Boolean			active;
	Boolean			floating;
	Boolean			themeSavvy;		// NS 1.7.1

	void	(*Dispose)		(WindowRef theWin);
	void	(*Update)		(WindowRef theWin);
	void	(*Activate)		(WindowRef theWin, Boolean active);
	void	(*HandleClick)	(WindowRef theWin, Point where, EventRecord *er);
	void	(*Draw)			(WindowRef theWin);
	void	(*Idle)			(WindowRef theWin, EventRecord *er);
	void	(*Save)			(WindowRef theWin);
	void	(*SaveAs)		(WindowRef theWin);
	void	(*Revert)		(WindowRef theWin);
	void	(*ProcessKey)	(WindowRef theWin, EventRecord *theEvent);

}	ObjectWindowRecord, *ObjectWindowPtr;

WindowRef InitObjectWindow( short resID, ObjectWindowPtr theStorage, Boolean isFloating );
void DisposeObjectWindow( WindowRef theWin, Boolean disposeFlag );
void DefaultUpdate( WindowRef theWin );
void DefaultActivate( WindowRef theWin, Boolean active );
void DefaultHandleClick( WindowRef theWin, Point where, EventRecord *er );
void DefaultDispose( WindowRef theWin );
void DefaultDraw( WindowRef theWin );

#endif