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

#include "HexEdit.h"

#ifndef _HexEdit_Main_
#define _HexEdit_Main_

#include "AECompareSuite.h"

OSStatus InitToolbox( void );
OSStatus InitMultifinder( void );
OSStatus InitGlobals( void );
OSStatus InitAppleEvents( void );
OSStatus CheckEnvironment( void );
OSStatus HandleEvent( void );
OSStatus DoEvent( EventRecord *theEvent );
OSStatus IdleObjects( EventRecord *er );

#endif