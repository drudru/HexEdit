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
 * Copyright (C) Copyright © 1996-2001.
 * All Rights Reserved.
 * 
 * Contributor(s):
 *		Nick Shanks
 */

#ifndef _HexEdit_Preferences_
#define _HexEdit_Preferences_

#define	kPrefCreatorType	FOUR_CHAR_CODE('hDmp')
#define	kPrefFileType		FOUR_CHAR_CODE('BINA')

#define	kPrefsStringsID		strFiles
#define	kPrefsFolderIndex	FN_PrefsFolder
#define	kPrefsFileNameIndex	FN_PrefsFile

// LR: preferences structure
#define PREFS_V204		0x0204
#define PREFS_VERSION	0x0205

typedef struct
{
	short	csResID;		// resource ID of default color table
	short	csMenuID;		// 1.65 don't double use version!
	
	short	searchMode,
			searchForward,	// false = backward
			searchCase,		// true = match case
			searchSize,		// byte, short or long
			searchType;		// hex or ASCII

	short	asciiMode;		// show high bit ascii chars (or substitute '.')?
	short	gotoMode;		// hex or dec
	short	decimalAddr;	// display numbers in windows as decimal or hex?
	short	overwrite;		// insert or overwrite editing?
	short	backupFlag;		// from original (moved in 1.5)
	short	vertBars;		// use David Emme's vertical bars

	short	useColor;		// true if we are using color windows

	short	version;		// version # of prefs record

	// %% NOTE: below version so that older versions do not require prefs to be inited!
	//			If a prefs update requires older prefs inited, move below above version!

	short	constrainSize;	// resizing allows partial lines?

}	prefs_t, *prefsPtr;

extern prefs_t prefs;

// --- Prototypes ---

Boolean SavePrefs( void );
Boolean LoadPrefs( void );

#endif