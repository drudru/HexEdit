/*
**	AECompareSuite.h
**
**	VOODOO Apple event interface
**
**	Copyright 1996 by UNI SOFTWARE PLUS GMBH, All rights reserved.
**	http://www.unisoft.co.at
**
**	Hint: Set tab size to 4
**
**	History:
**	98/01/16	S. Kratky	initial version for public release
*/

#ifndef _AECOMPARESUITE_H_
#define _AECOMPARESUITE_H_

#ifdef __MWERKS__
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__MWERKS__) && defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
#pragma import on
#endif

#if defined(__MWERKS__) 
#pragma options align=mac68k
#endif

enum {
	/* events and terms used for comparing files in cooperation with VOODOO */
	kCompareEventClass			= 'Comp',

	/* event "compare" */
	kAECompareEvent 			= 'Comp',
	kAENewFileParam				= 'kNew',	/* is of type 'fss ' (NEEDED) */
	kAEOldFileParam				= 'kOld',	/* is of type 'fss ' (NEEDED) */
	kAECompOptParam				= 'kOpt',	/* is of type 'cOpt' (OPTIONAL) */
	kAEShowDiffParam			= 'kShD',	/* is of type 'bool' (OPTIONAL, default: FALSE) */
	kAEReturnDiffParam			= 'kRtD',	/* is of type 'bool' (OPTIONAL, default: FALSE) */

	/* class "compare options" */
	cCompOptClass				= 'cOpt',
	pCaseInsensitive			= 'pCIn',	/* is of type 'bool' (OPTIONAL, default: FALSE) */
	pIgnoreDiacMarks			= 'pIDM',	/* is of type 'bool' (OPTIONAL, default: FALSE) */
	pIgnoreLeadSpace			= 'pILS',	/* is of type 'bool' (OPTIONAL, default: FALSE) */
	pArguments					= 'pArg',	/* is of type 'TEXT' (OPTIONAL, default: "") */

	/* class "compare result" */
	cCompResClass				= 'cRes',
	pDifferent					= 'pDft',	/* is of type 'bool' */
	pDifferences				= 'pDfc',	/* is of type 'TEXT' */
	pResultShown				= 'pRsS'	/* is of type 'bool' */
};

#if defined(__MWERKS__) 
#pragma options align=reset
#endif

#if defined(__MWERKS__) && defined(__CFM68K__) && !defined(__USING_STATIC_LIBS__)
#pragma import reset
#endif

#ifdef __cplusplus
}
#endif

#endif  /*_AECOMPARESUITE_H_*/
