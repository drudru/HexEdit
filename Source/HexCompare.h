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

#ifndef _HexEdit_HexCompare_
#define _HexEdit_HexCompare_

#include "HexEdit.h"
#include "EditWindow.h"


extern WindowRef CompWind1, CompWind2;

Boolean PerformTextCompare( EditWindowPtr dWin, EditWindowPtr dWin2 );
//181 Boolean PerformTextDifferenceCompare( EditWindowPtr dWin, EditWindowPtr dWin2 );
//181 Boolean PerformTextMatchCompare( EditWindowPtr dWin, EditWindowPtr dWin2 );
void DoComparison( void );
Boolean GetCompareFiles( short modifiers );
void ComparisonPreferences( void );

#endif