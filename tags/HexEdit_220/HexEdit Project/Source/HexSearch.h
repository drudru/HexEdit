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
 * Copyright (C) Copyright © 1996-2008.
 * All Rights Reserved.
 *
 * Contributor(s):
 *		Lane Roathe
 *		Nick Shanks
 */

#include "HexEdit.h"
#include "EditWindow.h"

#ifndef _HexEdit_HexSearch_
#define _HexEdit_HexSearch_

enum	// search dialog items
{
	SearchForwardItem		= 1,
	SearchBackwardItem,
	HexModeItem,
	AsciiModeItem,
	SearchTextItem,
	textFind,
	MatchCaseItem,
	textMatch,
	textReplace,
	ReplaceTextItem,
	ReplaceItem,
	ReplaceAllItem,
	WrapItem
};

//LR - 188: text search can now skip UI updates (for replace all)
typedef enum
{
	kSearchUpdateUI = 0,
	kSearchSkipUI = 1

} SearchUIFlag;

void SetSearchButtons( void );
void OpenSearchDialog( void );
Boolean PerformTextSearch( EditWindowPtr dWin, SearchUIFlag uiSkipFlag );
OSStatus OpenGotoAddress( void );
void DoModelessDialogEvent( EventRecord *theEvent );
Boolean StringToSearchBuffer( Boolean matchCase );

#endif