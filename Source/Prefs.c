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

// 05/10/01 - GAB: MPW environment support
#ifdef __MPW__
#include "MPWIncludes.h"
#endif

#include "Prefs.h"
#include "Menus.h"

static Str63 prefsFName;

/*** INIT PREFS ***/
static OSStatus _prefsInit( void )
{
	// bad version, create default
	memset( &gPrefs, 0, sizeof( gPrefs ) );

	gPrefs.searchCase = false;
	gPrefs.searchMode = EM_Ascii;
	gPrefs.searchSize = CM_Byte;
	gPrefs.searchType = CM_Different;
	gPrefs.searchForward = true;

	gPrefs.asciiMode = false;
	gPrefs.decimalAddr = EM_Hex;
	gPrefs.overwrite = false;
	gPrefs.nonDestructive = true;
	gPrefs.backupFlag = true;
	gPrefs.vertBars = true;

	gPrefs.useColor = true;

	gPrefs.csMenuID = 3;	// default is 1'st color in menu (" Default")
	gPrefs.csResID = -1;

	gPrefs.constrainSize = true;
	gPrefs.formatCopies = true;

	gPrefs.version = PREFS_VERSION;
	return noErr;
}

/*** GET PREFS FILE PATH ***/
static Boolean _prefsGetFSPath( long *prefDirID, short *systemVolRef )
{
	OSStatus	error;
	
	error = FindFolder( kOnSystemDisk, kPreferencesFolderType, kCreateFolder, 
		systemVolRef, prefDirID );
	if( error != noErr )
		return false;
	
	return true;
}

/*** WRITE PREFS ***/
static OSStatus _prefsWrite( long *prefDirID, short *systemVolRef )
{
	OSStatus		error;
	short		fileRefNum;
	long		byteCount;
	FSSpec		theSpecs;
	
	error = FSMakeFSSpec( *systemVolRef, *prefDirID, prefsFName, &theSpecs );
	if( error != noErr )
	{
		if( error != fnfErr )
		{
// 			ErrorExit( "\pPrefs FSMakeFSSpec() Error", error );
			return error;
		}
		error = FSpCreate( &theSpecs, kPrefCreatorType, kPrefFileType, smSystemScript );
		if( error != noErr )
		{
// 			ErrorExit( "\pPrefs FSpCreate() Error", error );
			return error;
		}
	}
	error = FSpOpenDF( &theSpecs, fsRdWrPerm, &fileRefNum );
	if( error != noErr )
	{
// 		ErrorExit( "\pPrefs FSpOpenDF() Error", error );
		return error;
	}
	
	byteCount = sizeof( gPrefs );
	
	error = FSWrite( fileRefNum, &byteCount, (Ptr) &gPrefs );
	if( error != noErr )
	{
// 		ErrorExit( "\pPrefs FSWrite() Error", error );
		return error;
	}
	
	error = FSClose( fileRefNum );
	if( error != noErr )
	{
// 		ErrorExit( "\pPrefs FSClose() Error", error );
		return error;
	}
	
	return error;
}

/*** READ PREFS ***/
static OSStatus _prefsRead( long *prefDirID, short *systemVolRef )
{
	OSStatus	error;
	short		fileRefNum;
	long		byteCount;
	FSSpec		theSpecs;
	
	error = FSMakeFSSpec( *systemVolRef, *prefDirID, prefsFName, &theSpecs );
	if( error != noErr )
	{
// 		if( error == fnfErr )
			return error;
// 		else
// 		{
// 			ErrorExit( "\pPrefs FSMakeFSSpec() Error", error );
// 		}
	}
	
	error = FSpOpenDF( &theSpecs, fsRdWrPerm, &fileRefNum );
	if( error != noErr )
	{
// 		ErrorExit( "\pPrefs FSpOpenDF() Error", error );
		return error;
	}
	
	byteCount = sizeof( gPrefs );
	
	error = FSRead( fileRefNum, &byteCount, (Ptr) &gPrefs );
	if( error != noErr )
	{
		if( error == eofErr )
			FSClose( fileRefNum );
		else
		{
// 			ErrorExit( "\pPrefs FSRead() Error", error );
		}
		return error;
	}
	
	error = FSClose( fileRefNum );
	if( error != noErr )
	{
// 		ErrorExit( "\pPrefs FSClose() Error", error );
		return error;
	}
	
	return error;
}

/*** DELETE PREFS ***/
static Boolean _prefsDelete( long *dirID, short *volRef )
{
	FSSpec		theSpecs;
	OSStatus	error;
	
	error = FSMakeFSSpec( *volRef, *dirID, prefsFName, &theSpecs );
	if( error != noErr )
		return false;
	else
		error = FSpDelete( &theSpecs );
	
	if( error != noErr )
		return false;
	
	return true;
}

#pragma mark -

/*** SAVE PREFS ***/
Boolean PrefsSave( void )
{
	long		prefDirID;
	short		systemVolRef;

	if( !_prefsGetFSPath( &prefDirID, &systemVolRef ) )
		return false;

	// LR: v1.6.5 don't save a bad gPrefs structure!
	if( PREFS_VERSION != gPrefs.version )
		_prefsInit();

// LR: v1.6.5	gPrefs.csResID = gPrefs.csMenuID;	// LR: 1.5, yech!
// LR: v1.6.5	gPrefs.version = PREFS_VERSION;

	if( noErr != _prefsWrite( &prefDirID, &systemVolRef ) )
		return false;

	return true;
}

/*** LOAD PREFS ***/
Boolean PrefsLoad( void )
{
	OSStatus	error;
	long		prefDirID;
	short		systemVolRef;
	Boolean		noProblems;

	GetIndString( prefsFName, kPrefsStringsID, kPrefsFileNameIndex );	// LR: v1.6.5 (always run, so use by Save is OK!)

	gPrefs.version = 0;	// failure flag

	noProblems = _prefsGetFSPath( &prefDirID, &systemVolRef );
	if( noProblems )
	{
		error = _prefsRead( &prefDirID, &systemVolRef );
		if( error == eofErr )
		{
			noProblems = _prefsDelete( &prefDirID, &systemVolRef );
// LR: v1.6.5			return false;
		}
	}
/* LR -- 1.5 gPrefs never worked...sigh
	else if( error != noErr )
		return false;
*/
	if( PREFS_VERSION != gPrefs.version )	// check for good version #
	{
		_prefsInit();					// !!! start from scratch!
	}

	// funky...but menus sorted by name mess me up!
// LR: v1.6.5	gPrefs.version = gPrefs.csResID;
	gPrefs.csResID = GetColorMenuResID( gPrefs.csMenuID );

	if( gPrefs.asciiMode )	g.highChar = 0xFF;
	else					g.highChar = 0x7F;
	return true;
}

