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

#include "Prefs.h"
#include "Menus.h"

extern globals g;
extern prefs_t prefs;

static Str63 prefsFName;

/*** INIT PREFS ***/
OSStatus InitPrefs( void )
{
	// bad version, create default
	memset( &prefs, 0, sizeof( prefs ) );

	prefs.searchCase = false;
	prefs.searchMode = EM_Ascii;
	prefs.searchSize = CM_Byte;
	prefs.searchType = CM_Different;
	prefs.searchForward = true;

	prefs.asciiMode = false;
	prefs.decimalAddr = EM_Hex;
	prefs.overwrite = false;
	prefs.backupFlag = true;
	prefs.vertBars = false;

	prefs.useColor = false;

	prefs.csMenuID = 3;	// default is 1'st color in menu
	prefs.csResID = -1;

	prefs.version = PREFS_VERSION;
	return noErr;
}

/*** CAN USE FIND FOLDER ***/
Boolean CanUseFindFolder( void )
{
	OSStatus	error;
	long		theFeature;
	
	error = Gestalt( gestaltFindFolderAttr, &theFeature );
	if( error != noErr )
		return false;
	
	if( !BitTst( &theFeature, 31 - gestaltFindFolderPresent ) )
		return false;
	else
		return true;
}

/*** GET PREFS FILE PATH ***/
Boolean GetPrefsFPath( long *prefDirID, short *systemVolRef )
{
	OSStatus	error;
	
	error = FindFolder( kOnSystemDisk, kPreferencesFolderType, kCreateFolder, 
		systemVolRef, prefDirID );
	if( error != noErr )
		return false;
	
	return true;
}

/* CREATE PREFS FOLDER ***/
Boolean CreatePrefsFolder( short *systemVolRef )
{
	HFileParam	fileParamBlock;
	Str255		folderName;
	OSStatus		error;
	
	GetIndString( folderName, kPrefsStringsID, kPrefsFolderIndex );
	
	fileParamBlock.ioVRefNum = *systemVolRef;
	fileParamBlock.ioDirID = 0;
	fileParamBlock.ioNamePtr = folderName;
	fileParamBlock.ioCompletion = 0L;
	
	error = PBDirCreate( ( HParmBlkPtr ) &fileParamBlock, false );
	if( error != noErr )
	{
// 		ErrorExit( "\pPrefs Creation Error", error );
		return false;
	}
	return true;
}

#if !TARGET_API_MAC_CARBON

/*** GET PREFS FILE PATH (SYSTEM 6) ***/
Boolean GetPrefsFPath6( short *systemVolRef )
{
	Str255		folderName;
	SysEnvRec	thisWorld;
	CInfoPBRec	catalogInfoPB;
	DirInfo		*directoryInfo = ( DirInfo * ) &catalogInfoPB;
	HFileInfo	*fileInfo = ( HFileInfo * ) &catalogInfoPB;
	WDPBRec		workingDirPB;
	long		prefDirID;
	OSStatus		error;
	
	GetIndString( folderName, kPrefsStringsID, kPrefsFolderIndex );
	
	error = SysEnvirons( 2, &thisWorld );
	if( error != noErr )
		return false;
		
	*systemVolRef = thisWorld.sysVRefNum;
	fileInfo->ioVRefNum = *systemVolRef;
	fileInfo->ioDirID	= 0;
	fileInfo->ioFDirIndex = 0;
	fileInfo->ioNamePtr = folderName;
	fileInfo->ioCompletion = 0L;
	error = PBGetCatInfo( &catalogInfoPB, false );
	if( error != noErr )
	{
		if( error != fnfErr )
		{
// 			ErrorExit( "\pPrefs Filepath Error", error );
			return false;
		}
		if( !CreatePrefsFolder( systemVolRef ) )
			return false;
	
		directoryInfo->ioVRefNum = *systemVolRef;
		directoryInfo->ioFDirIndex = 0;
		directoryInfo->ioNamePtr = folderName;
		error = PBGetCatInfo( &catalogInfoPB, false );
		if( error != noErr )
		{
// 			ErrorExit( "\pPrefs GetCatInfo() Error", error );
			return false;
		}
	}
	prefDirID = directoryInfo->ioDrDirID;
	
	workingDirPB.ioNamePtr = NULL;
	workingDirPB.ioVRefNum = *systemVolRef;
	workingDirPB.ioWDIndex = 0;
	workingDirPB.ioWDProcID = 0;
	workingDirPB.ioWDVRefNum = 0;
	workingDirPB.ioCompletion = 0L;
	error = PBGetWDInfo( &workingDirPB, false );
	if( error != noErr )
	{
// 		ErrorExit( "\pPrefs PBGetWDInfo() Error", error );
		return false;
	}
		
	*systemVolRef = workingDirPB.ioWDVRefNum;
	
	workingDirPB.ioNamePtr = NULL;
	workingDirPB.ioWDDirID = prefDirID;
	workingDirPB.ioVRefNum = *systemVolRef;
	workingDirPB.ioWDProcID = 0;
	workingDirPB.ioCompletion = 0L;
	error = PBOpenWD( &workingDirPB, false );
	if( error != noErr )
	{
// 		ErrorExit( "\pPrefs PBOpenWD() Error", error );
		return false;
	}
	
	*systemVolRef = workingDirPB.ioVRefNum;
	
	return true;
}

#endif

/*** WRITE PREFS ***/
OSStatus WritePrefs( long *prefDirID, short *systemVolRef )
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
	
	byteCount = sizeof( prefs );
	
	error = FSWrite( fileRefNum, &byteCount, (Ptr) &prefs );
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

#if !TARGET_API_MAC_CARBON

/*** WRITE PREFS (SYSTEM 6) ***/
OSStatus WritePrefs6 ( short *systemVolRef )
{
	OSStatus		error;
	short		fileRefNum;
	long		byteCount;
	
	error = FSOpen( prefsFName, *systemVolRef, &fileRefNum );
	if( error != noErr )
	{
		if( error != fnfErr )
		{
// 			ErrorExit( "\pPrefs FSOpen() Error", error );
			return error;
		}
		error = Create( prefsFName, *systemVolRef, kPrefCreatorType, kPrefFileType );
		if( error != noErr )
		{
// 			ErrorExit( "\pPrefs Create() Error", error );
			return error;
		}
		error = FSOpen( prefsFName, *systemVolRef, &fileRefNum );
		if( error != noErr )
		{
// 			ErrorExit( "\pPrefs FSOpen() Error", error );
			return error;
		}
	}
	
	byteCount = sizeof( prefs );
	
	error = FSWrite( fileRefNum, &byteCount, (Ptr) &prefs );
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

#endif

/*** SAVE PREFS ***/
Boolean SavePrefs( void )
{
	long		prefDirID;
	short		systemVolRef;

#if TARGET_API_MAC_CARBON
	if( !GetPrefsFPath( &prefDirID, &systemVolRef ) )
		return false;
#else
	Boolean canUseFSSpecs = CanUseFindFolder();
	if( canUseFSSpecs )
	{
		if( !GetPrefsFPath( &prefDirID, &systemVolRef ) )
			return false;
	}
	else
	{
		if( !GetPrefsFPath6( &systemVolRef ) )
			return false;
	}
#endif

	// LR: v1.6.5 don't save a bad prefs structure!
	if( PREFS_VERSION != prefs.version )
		InitPrefs();

// LR: v1.6.5	prefs.csResID = prefs.csMenuID;	// LR: 1.5, yech!
// LR: v1.6.5	prefs.version = PREFS_VERSION;

#if TARGET_API_MAC_CARBON
	if( noErr != WritePrefs( &prefDirID, &systemVolRef ) )
		return false;
#else
	if( canUseFSSpecs )
	{
		if( noErr != WritePrefs( &prefDirID, &systemVolRef ) )
			return false;
	}
	else
	{
		if( noErr != WritePrefs6( &systemVolRef ) )
			return false;
	}
#endif
	
	return true;
}

/*** READ PREFS ***/
OSStatus ReadPrefs( long *prefDirID, short *systemVolRef )
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
	
	byteCount = sizeof( prefs );
	
	error = FSRead( fileRefNum, &byteCount, (Ptr) &prefs );
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

#if !TARGET_API_MAC_CARBON

/*** READ PREFS (SYSTEM 6) ***/
OSStatus ReadPrefs6( short *systemVolRef )
{
	OSStatus		error;
	short		fileRefNum;
	long		byteCount;
	
	error = FSOpen( prefsFName, *systemVolRef, &fileRefNum );
	if( error != noErr )
	{
// 		if( error == fnfErr )
// 			return error;
// 		else
// 		{
// 			ErrorExit( "\pPrefs FSOpen() Error", error );
// 		}
	}
	
	byteCount = sizeof( prefs );
	
	error = FSRead( fileRefNum, &byteCount, (Ptr) &prefs );
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
	}
	
	return error;
}

#endif

/*** DELETE PREFS ***/
Boolean DeletePrefs( long *dirID, short *volRef )
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

#if !TARGET_API_MAC_CARBON

/*** DELETE PREFS (SYSTEM 6) ***/
Boolean DeletePrefs6( short *volRef )
{
	OSStatus	error;
	
	error = FSDelete( prefsFName, *volRef );
	
	if( error != noErr )
		return false;
	
	return true;
}

#endif

/*** LOAD PREFS ***/
Boolean LoadPrefs( void )
{
	OSStatus	error;
	long		prefDirID;
	short		systemVolRef;
	Boolean		noProblems;
#if !TARGET_API_MAC_CARBON	// LR: v1.6
	Boolean		canUseFSSpecs;
#endif

	GetIndString( prefsFName, kPrefsStringsID, kPrefsFileNameIndex );	// LR: v1.6.5 (always run, so use by Save is OK!)

	prefs.version = 0;	// failure flag

#if !TARGET_API_MAC_CARBON	// LR: v1.6 always in carbon!
	canUseFSSpecs = CanUseFindFolder();
	if( canUseFSSpecs )
#endif
	{
		noProblems = GetPrefsFPath( &prefDirID, &systemVolRef );
		if( noProblems )
		{
			error = ReadPrefs( &prefDirID, &systemVolRef );
			if( error == eofErr )
			{
				noProblems = DeletePrefs( &prefDirID, &systemVolRef );
// LR: v1.6.5			return false;
			}
		}
/* LR -- 1.5 prefs never worked...sigh
		else if( error != noErr )
			return false;
*/
	}
#if !TARGET_API_MAC_CARBON	// LR: v1.6
	else
	{
		noProblems = GetPrefsFPath6( &systemVolRef );
		if( noProblems )
		{
			error = ReadPrefs6( &systemVolRef );	// LR: v1.6.5 never return false
			if( error )
			{
				DeletePrefs6( &systemVolRef );
			}
		}
	}
#endif

	if( prefs.version != PREFS_VERSION )	// check for good version #
		InitPrefs();

	// funky...but menus sorted by name mess me up!
// LR: v1.6.5	prefs.version = prefs.csResID;
	prefs.csResID = GetColorMenuResID( prefs.csMenuID );

	if( prefs.asciiMode )	g.highChar = 0xFF;
	else					g.highChar = 0x7F;
	return true;
}

