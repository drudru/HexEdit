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


#include <stdarg.h>
#include <string.h>

#if TARGET_API_MAC_OS8
	#if PROFILE			// 6/15 Optional profiling support
		#include <Console.h>
		#include <Profile.h>
	#endif
#else
	#ifdef PROFILE
		#undef PROFILE
	#endif
	
	//#include <Carbon/Carbon.h>
#endif

#ifndef __MC68K__
	#include <InternetConfig.h>
#endif

#ifndef _HexEdit_
#define _HexEdit_

// Comatibility macros
#if !TARGET_API_MAC_CARBON
	#define SetPortDialogPort SetPort
	#define GetPortBounds( p, rp ) *rp = (p)->portRect
	#define GetWindowPortBounds( w, rp ) *rp = (w)->portRect
	#define GetPortPixMap(p) p->portPixMap
	#define InvalWindowRect(w,r) InvalRect(r)
	#define EnableMenuItem EnableItem
	#define DisableMenuItem DisableItem
	#define GetWindowList FrontWindow
	#define MenuRef MenuHandle
#endif

/*** COLOUR TABLE ***/
typedef struct
{
	RGBColor	header;
	RGBColor	headerLine;
	RGBColor	headerText;

	RGBColor	bar;
	RGBColor	barLine;
	RGBColor	barText;

	RGBColor	body;
	RGBColor	text;

}	HEColorTable_t, *HEColorTablePtr, **HEColorTableHandle;

/*** GLOBAL VARIABLES ***/
//LR: 1.66 -- removed undo ptrs
typedef struct
{
	// system info
	Boolean		quitFlag;
#if !TARGET_API_MAC_CARBON	// LR: v1.6
	Boolean		WNEImplemented;
	Boolean		sys7Flag;
	Boolean		colorQDFlag;
#endif
	Boolean		dragAvailable;
	Boolean		translucentDrag;
	Boolean		navAvailable;
	Boolean		appearanceAvailable;
	
	// debugging prefs
	Boolean		useAppleEvents;
	Boolean		useAppearance;
	Boolean		useNavServices;
	
	// application globals
	Boolean 	cursorFlag;
	Rect		cursRect;
	UInt8		forkMode;
	UInt8		highChar;
	SInt8		buffer[512];
	UInt16		maxHeight;
	UInt8		searchBuffer[256];
	UInt8		searchText[256];
	UInt8		gotoText[256];
	Boolean		searchDisabled;

#ifndef __MC68K__
	ICInstance	icRef;
#endif

	// cursors
	CursHandle	watch;
	CursHandle	iBeam;
	
	// dialogs
	DialogPtr	searchWin;
	DialogPtr	gotoWin;
	
	// printing
#if TARGET_API_MAC_OS8
	THPrint		HPrint;
#endif
}	globals;

extern globals g;

// make some things a bit easier to read
#define kAppCreator		FOUR_CHAR_CODE('hDmp')
#define kDefaultFileType FOUR_CHAR_CODE('TEXT')
#define SBarSize		16
#define	GrowIconSize	14
#define	MainWIND		128
#define MaxFileRAM		32000L
#define SlushRAM		1000L
#define AllocIncr		64L
#define HeaderHeight	20
#define FooterHeight	20
#define TopMargin		3
#define BotMargin		0
#define AddrPos			10	// LR: 12
#define HexStart		9
#define AsciiStart		59
#define AsciiSpacing	6
#define LineHeight		11
#define DescendHeight	3
#define CharWidth		6
#define HexWidth		(CharWidth*3)
#define LineNbr(a)		((a) >> 4)
#define ColNbr(a)		((a) & 0x0F)
#define CharPos(a)		(((a) << 2) + ((a) << 1))	// Multiply by 6
#define HexPos(a)		(((a) << 4) + ((a) << 1))	// Multiply by 18

#define MenuBaseID		128

#define MaxWindowWidth	484	// Must be multiple of 2
#define MaxWindowHeight	512	// Note - these are NOT reversed!

// LR: #define PrefResType	'prf1'
// LR: #define PrefResID	128

/* NS: now uses Apple defined constants from events.h
#define TAB_KEY			0x09
#define RETURN_KEY		0x0D
#define ENTER_KEY		0x03
#define ESC_KEY			27
*/

// LR: 1.6.5 -- better ID defs, and include all references
enum AlertIDs		{ alertSave = 10000, alertError, alertNoFork, alertMessage };
enum DialogIDs		{ dlgSearch = 128, dlgGoto, dlgAbout, dlgCompare, dlgComparePref, dlgGetFile = 1401 };
enum StringIDs		{ strUndo = 128, strPrint, strHeader, strError, strColor, strPrompt, strFiles, strURLs };

enum ErrorIDs		{ errMemory = 1, errSeek, errRead, errSetFPos, errWrite, errPaste, errFindFolder,
						errCreate, errOpen, errFileInfo, errPrintRange, errSetFileInfo, errBackup,
						errRename, errDiskFull, errHexValues
					};

enum ChunkTypes		{ CT_Original, CT_Work, CT_Unwritten };
enum ForkType 		{ FT_Data, FT_Resource };
enum ForkModes		{ FM_Data, FM_Rsrc, FM_Smart };
enum AsciiModes		{ AM_Lo, AM_Hi };
enum EditMode 		{ EM_Hex, EM_Decimal, EM_Ascii };
enum EditOperation	{ EO_Undo = 1, EO_Redo, EO_Typing, EO_Paste, EO_Insert, EO_Overwrite, EO_Cut, EO_Clear, EO_Delete };
enum ErrorSeverity	{ ES_Message, ES_Note, ES_Caution, ES_Stop };
enum CursorNumbers	{ C_Arrow, C_Watch, C_IBeam };
enum CompModeSize 	{ CM_Byte, CM_Word, CM_Long };
enum CompModeType 	{ CM_Different, CM_Match };

// preferences Dialog items
enum CompPref { CP_Done=1,CP_Cancel,CP_Bytes,CP_Words,CP_Longs,CP_Different,CP_Match,
								CP_text1, CP_text2, CP_Case };

// Color table resources MUST start @ 128 & advance by 1
#define CM_StartingResourceID 128

// window ID
#define HexEditWindowID		1000

// Menu Resource IDs
enum	{kAppleMenu = 128, kFileMenu, kEditMenu, kFindMenu, kOptionsMenu, kColorMenu, kWindowMenu};

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

enum	{OM_HiAscii = 1, OM_DecimalAddr, OM_VertBars, OM_Overwrite, OM_Sep1, OM_Backups,
			OM_ComparePref};

enum	{CM_UseColor = 1, CM_Sep1, CM_FirstColor};

#endif