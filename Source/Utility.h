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

#ifndef _HexEdit_Utility_
#define _HexEdit_Utility_

// Utility Functions
void SetControl( DialogPtr dialog, short item, short value );
void SetText( DialogPtr dialog, short item, StringPtr text );
void GetText( DialogPtr dialog, short item, StringPtr text );
void GetRect( DialogPtr dialog, short item, Rect *r );
void SetDraw( DialogPtr dialog, short item, Handle proc );
void DisableButton( DialogPtr dialog, short bid );
void EnableButton( DialogPtr dialog, short bid );
void SimulateButtonPress( DialogPtr dialog, short bid );
Boolean CheckForAbort( void );
short ErrorAlert( short severity, short strid, ... );
short MyRandom( short limit );
void MySetCursor( short n );
void CopyPascalStringToC( ConstStr255Param source, char* dest );
void CopyCStringToPascal( const char* source, Str255 dest );
OSStatus LaunchURL( StringPtr url );
unsigned long CStringLength( char *string );
Boolean EqualPStrings( UInt8 *source, UInt8 *dest );

#endif