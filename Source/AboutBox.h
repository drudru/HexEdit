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
 * The Initial Developer of the Original Code is Lane Roathe
 * Portions created by Lane Roathe are
 * Copyright (C) Copyright © 1996-2002.
 * All Rights Reserved.
 *
 * Modified: $Date$
 * Revision: $Id$
 *
 * Contributor(s):
 *		Lane Roathe
 */

#include "HexEdit.h"

#ifndef _HexEdit_AboutBox_
#define _HexEdit_AboutBox_

#define itemFirstURL 11	// all the rest must follow WITHOUT BREAKS!

pascal void DrawTEText( DialogPtr whichDialog, short itemNr );
pascal Boolean DialogFilter( DialogPtr whichDialog, EventRecord *event, short *itemHit );
void HexEditAboutBox( void );

#endif