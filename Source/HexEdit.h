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
 *		Scott E. Lasley (SEL) 
 */


#if TARGET_API_MAC_CARBON
	#include <PMApplication.h>
#endif

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
//LR -- 1.72 :no longer requires carbon headers to compile for non-carbon targets
#if !TARGET_API_MAC_CARBON
	#if !defined(SetPortDialogPort)
		#define SetPortDialogPort SetPort
	#endif
	#define GetPortBounds( p, rp ) *rp = (p)->portRect
	#define GetWindowPortBounds( w, rp ) *rp = (w)->portRect
	#define GetPortPixMap(p) p->portPixMap
	#define InvalWindowRect(w,r) InvalRect(r)
	#define EnableMenuItem EnableItem
	#define DisableMenuItem DisableItem
	#define FrontNonFloatingWindow FrontWindow
	#define MenuRef MenuHandle
	#if !defined(GetDialogFromWindow)
		#define GetDialogFromWindow
	#endif
#endif

// make some things a bit easier to read

#define kAppCreator		FOUR_CHAR_CODE('hDmp')
#define kDefaultFileType FOUR_CHAR_CODE('TEXT')

#define kMaxFileRAM		32000L
#define kSlushRAM		1000L
#define kAllocIncrement	64L

#define kSBarSize		16
#define	kGrowIconSize	14

#define kBytesPerLine	16

#define kHeaderHeight	16
#define kFooterHeight	16
//LR: 1.7 #define TopMargin		3
//LR: 1.7 #define BotMargin		0
//LR: 1.7 #define AsciiSpacing	6
//LR: 1.7 #define DescendHeight	0

#define kStringHexPos	11
#define kStringTextPos	(kStringHexPos+(kBytesPerLine*3)+1)
#define kBodyStrLen		(kStringTextPos+kBytesPerLine-kStringHexPos)	// LR: 1.7 was 75 - (kStringHexPos -  2) in EditWindow.c

#define kFontFace		"\pMonaco"
#define kFontSize		9
#define kLineHeight		11	//%% make flexible (and char width?)
#define kCharWidth		6
#define kHexWidth		(kCharWidth*3)

#define kBodyDrawPos	0	// LR: 12 - let's not have any undrawn areas, to avoid erasing!
#define kDataDrawPos	(kBodyDrawPos + ((kStringHexPos - 1) * kCharWidth))
#define kTextDrawPos	(kBodyDrawPos + ((kStringTextPos -  1) * kCharWidth))

#define LINENUM(a)		((a) >> 4)
#define COLUMN(a)		((a) & 0x0F)
#define CHARPOS(a)		(((a) << 2) + ((a) << 1))	// Multiply by 6
#define HEXPOS(a)		(((a) << 4) + ((a) << 1))	// Multiply by 18

#define kHexWindowWidth	(((kStringHexPos + kBodyStrLen) * kCharWidth) + kSBarSize)	//LR 1.7 - renamed to show windows are not sizable horizontally! Must be multiple of 2

//LR 1.7 #define MaxWindowHeight	512	// Note - these are NOT reversed!

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
						errRename, errDiskFull, errHexValues, errDefaultPrinter, errGenericPrinting
					};

enum ChunkTypes		{ CT_Original, CT_Work, CT_Unwritten };
enum ForkType 		{ FT_Data = 1, FT_Resource };
enum ForkModes		{ FM_Data = 1, FM_Rsrc, FM_Smart };
enum Headers		{ HD_Decimal = FT_Resource + 1, HD_Hex, HD_Footer };
enum Filenames		{ FN_PrefsFolder = 1, FN_PrefsFile, FN_Untitled, FN_DATA, FN_RSRC };
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
#define	kSystem7Window			128		// NS 1.7.1
#define	kAppearanceWindow		129

#define kHexEditWindowTag		1000

// Mac OS versions
#define kMacOSSevenPointOne	 	0x00000710
#define kMacOSSevenPointFivePointFive 0x00000755
#define kMacOSEight				0x00000800
#define kMacOSEightPointFive	0x00000850
#define kMacOSEightPointSix		0x00000860
#define kMacOSNine				0x00000900
#define kMacOSNinePointOne		0x00000910
#define kMacOSTen				0x00001000

// CarbonLib versions
#define kCarbonLibOnePointOne			0x00000110
#define kCarbonLibOnePointThreePointOne 0x00000131

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
	SInt32		systemVersion;		// NS 1.7.1
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

	short		fontFaceID;
	short		fontSize;

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
#if TARGET_API_MAC_CARBON	// SEL -- 1.7
	PMPrintSettings	printSettings;
	PMPageFormat	pageFormat;
#endif
}	globals;

extern globals g;

#endif