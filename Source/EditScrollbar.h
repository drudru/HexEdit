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
 * Copyright (C) Copyright � 1996-2000.
 * All Rights Reserved.
 * 
 * Contributor(s):
 *		Nick Shanks
 */

#include "EditWindow.h"

#ifndef _HexEdit_EditScrollbar_
#define _HexEdit_EditScrollbar_

void SetupScrollBars( EditWindowPtr dWin );
void AdjustScrollBars( WindowRef theWin, short resizeFlag );
pascal void MyScrollAction( ControlHandle theControl, short thePart );
Boolean MyHandleControlClick( WindowRef window, Point mouseLoc );
void ScrollToSelection( EditWindowPtr dWin, long pos, Boolean forceUpdate, Boolean centerFlag );
void HEditScrollToPosition( EditWindowPtr dWin, long newPos );
void AutoScroll( EditWindowPtr dWin, Point pos );

#endif