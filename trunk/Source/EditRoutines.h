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

#include "EditWindow.h"

#ifndef _HexEdit_EditRoutines_
#define _HexEdit_EditRoutines_

/*** UNDO RECORD ***/
//LR: 1.66 - get rid of back-ass delcaration method
typedef struct
{
	short			type;				// Type of operation
	short			reserved;
	long			startSel;			// Start of Selection
	long			endSel;				// End of Selection
	long			fileSize;			// File Size for Undo Op
	EditChunk		**undoScrap;
	EditWindowPtr	theWin;
}UndoRecord, *UndoPtr;

// --- Global vars ---

extern UndoRecord gUndo, gRedo;

// --- Prototypes ---

void LoadFile( EditWindowPtr dWin );
void UnloadFile( EditWindowPtr dWin );
EditChunk** NewChunk( long size, long addr, long filePos, short type );
void DisposeChunk( EditWindowPtr dWin, EditChunk **cc );
EditChunk** AppendChunk( EditChunk **list, EditChunk **chunk );
void SetCurrentChunk( EditWindowPtr dWin, long addr );
EditChunk** GetChunkByAddr( EditWindowPtr dWin, long addr );
Byte GetByte( EditWindowPtr dWin, long addr );
void LoadChunk( EditWindowPtr dWin, EditChunk **cc );
void UnloadLeastUsedChunk( EditWindowPtr dWin );
void UnloadChunk( EditWindowPtr dWin, EditChunk	**cc, Boolean writeFlag );
void RewriteAddressChain( EditChunk **fc );
void DeleteSelection( EditWindowPtr dWin );
void InsertCharacter( EditWindowPtr dWin, short charCode );
void ReleaseEditScrap( EditWindowPtr dWin, EditChunk ***scrap );
void CopySelection( EditWindowPtr dWin );
void CopyOperation( EditWindowPtr dWin, EditChunk ***scrapChunk );
void CutSelection( EditWindowPtr dWin );
void MyGetScrap( EditWindowPtr dWin );
void PasteSelection( EditWindowPtr dWin );
Boolean HexConvertScrap( EditWindowPtr dWin, EditChunk **scrapChunk );
void PasteOperation( EditWindowPtr dWin, EditChunk **scrapChunk );
void ClearSelection( EditWindowPtr dWin );
void RememberOperation( EditWindowPtr dWin, short opType, UndoPtr ur );
void UndoOperation( void );

#endif