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

#include "HexEdit.h"

#ifndef _HexEdit_Preferences_
#define _HexEdit_Preferences_

#define	kPrefCreatorType	FOUR_CHAR_CODE('hDmp')
#define	kPrefFileType		FOUR_CHAR_CODE('BINA')

#define	kPrefsStringsID		strFiles
#define	kPrefsFolderIndex	1
#define	kPrefsFileNameIndex	2

// preferences
OSStatus InitPrefs( void );
Boolean CanUseFindFolder( void );
Boolean GetPrefsFPath( long *prefDirID, short *systemVolRef );
Boolean CreatePrefsFolder( short *systemVolRef );
Boolean GetPrefsFPath6( short *systemVolRef );
OSStatus WritePrefs( long *prefDirID, short *systemVolRef );
OSStatus WritePrefs6 ( short *systemVolRef );
Boolean SavePrefs( void );
OSStatus ReadPrefs( long *prefDirID, short *systemVolRef );
OSStatus ReadPrefs6( short *systemVolRef );
Boolean DeletePrefs( long *dirID, short *volRef );
Boolean DeletePrefs6( short *volRef );
Boolean LoadPrefs( void );

#endif