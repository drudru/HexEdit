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
 * Copyright (C) Copyright � 1996-2008.
 * All Rights Reserved.
 *
 * Contributor(s):
 *		Lane Roathe
 *		Nick Shanks
 */

#ifndef _HexEdit_Preferences_
#define _HexEdit_Preferences_

#include "HexEdit.h"

#define	kPrefCreatorType	FOUR_CHAR_CODE('hDmp')
#define	kPrefFileType		FOUR_CHAR_CODE('BINA')

#define	kPrefsStringsID		strFiles
#define	kPrefsFolderIndex	FN_PrefsFolder
#define	kPrefsFileNameIndex	FN_PrefsFile

// LR: preferences structure
#define PREFS_VERSION	0x0209

typedef struct
{
	short	csResID;		// resource ID of default color table
	short	csMenuID;		// 1.65 don't double use version!
	
	short	searchMode,		// Hex or ASCII searches
			searchForward,	// false = backward
			searchCase,		// true = match case
			searchSize,		// byte, short or long
			searchType;		// Find matches or differences

	short	asciiMode;		// show high bit ascii chars (or substitute '.')?
	short	gotoMode;		// hex or dec
	short	decimalAddr;	// display numbers in windows as decimal or hex?
	short	overwrite;		// insert or overwrite editing?
	short	backupFlag;		// from original (moved in 1.5)
	short	vertBars;		// use David Emme's vertical bars
	short	constrainSize;	// resizing allows partial lines?
	short	formatCopies;	// format copies (spaces, tabs - false == raw data)
	short	nonDestructive;	// in overwrite mode, Detete is non-destructive (## -> 00)
	short	moveOnlyPaging;	// page up/down only move display, not selection point

	short	useColor;		// true if we are using color windows

	short	wrapFinds;		//LR 190 -- true to wrap finds when reaching end/beg of file

 	short	dialogAtLaunch;	//functionality added WD-rpw 12-18-04

	// spare prefs entries so version file can be updated
	// w/o reseting prefs a few times :)
	// NOTE: these default to FALSE in case they ever become used!
	short	spare3;
	short	spare4;

	short	version;		// version # of gPrefs record

}	prefs_t, *prefsPtr;

extern prefs_t gPrefs;

// --- Prototypes ---

Boolean PrefsSave( void );
Boolean PrefsLoad( void );

#endif