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
 * Portions created by Lane Roathe (LR) are
 * Copyright (C) Copyright © 1996-2000.
 * All Rights Reserved.
 * 
 * Contributor(s):
 *		Nick Shanks (NS)
 */

#include "HexEdit.h"

#ifndef _HexEdit_Menus_
#define _HexEdit_Menus_

// Menu Resource IDs
#define kMenuBaseID		128
#define kMenuXBaseID	129

enum	{kAppleMenu = kMenuBaseID, kFileMenu, kEditMenu, kFindMenu, kOptionsMenu, kColorMenu, kWindowMenu};

// Menu Item Numbers
enum	{AM_About=1};

enum	{FM_New=1,FM_Open,FM_Close,FM_Sep1,
		 FM_OtherFork,FM_CompareFiles,FM_Sep2,	
		 FM_Save, FM_SaveAs, FM_Revert,FM_Sep3,
		 FM_PageSetup, FM_Print,FM_Sep4,
		 FM_Quit};

enum 	{EM_Undo = 1, EM_Sep1, EM_Cut, EM_Copy,
		 EM_Paste, EM_Clear, EM_Sep2, EM_SelectAll};
		 
enum	{SM_Find = 1, SM_FindForward, SM_FindBackward, SM_Sep1, SM_GotoAddress};

enum	{OM_HiAscii = 1, OM_DecimalAddr, OM_VertBars, OM_WinSize, OM_Overwrite, OM_NonDestructive, OM_Unformatted, OM_Sep1,
			 OM_Backups, OM_Sep2, OM_ComparePref};

enum	{CM_UseColor = 1, CM_Sep1, CM_FirstColor};

// *** Prototypes

OSStatus InitMenubar( void );
OSStatus SmartEnableMenuItem( MenuRef menu, short item, short ok );
OSStatus AdjustMenus( void );
short GetColorMenuResID( short menuItem );
short GetWindowMenuItemID( StringPtr title );
OSStatus HandleMenu( long mSelect );

#if TARGET_API_MAC_CARBON
void PostPrintingErrors( OSStatus status );	// SEL: 1.7 - carbon printing
#endif

#endif