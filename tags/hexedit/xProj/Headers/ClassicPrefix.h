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

// include file

#define CALL_NOT_IN_CARBON 1
#define TARGET_API_MAC_OS8 1
#define TARGET_API_MAC_CARBON 0
#define TARGET_API_MAC_OSX 0
#define OPAQUE_UPP_TYPES 0
#define OPAQUE_TOOLBOX_STRUCTS 0
#define ACCESSOR_CALLS_ARE_FUNCTIONS 1

	#include <AEDataModel.h>
	#include <AEObjects.h>
	#include <AEPackObject.h>
	#include <AERegistry.h>
	#include <AEUserTermTypes.h>
	#include <Aliases.h>
	#include <Appearance.h>
	#include <AppleEvents.h>
//	#include <AppleGuide.h>
	#include <AppleScript.h>
//	#include <AppleTalk.h>
	#include <ASDebugging.h>
	#include <ASRegistry.h>
//	#include <Balloons.h>
//	#include <CMApplication.h>
//	#include <CMICCProfile.h>
	#include <CodeFragments.h>
//	#include <Collections.h>
//	#include <ColorPicker.h>
//	#include <Components.h>
//	#include <ConditionalMacros.h>
	#include <ControlDefinitions.h>		/* New for 3.3.1 */
	#include <Controls.h>
//	#include <DateTimeUtils.h>
	#include <Devices.h>
//	#include <Dialogs.h>
//	#include <DiskInit.h>
//	#include <Displays.h>
	#include <Drag.h>
//	#include <DriverFamilyMatching.h>
//	#include <Endian.h>
//	#include <EPPC.h>
	#include <Events.h>
	#include <Files.h>
//	#include <FileTypesAndCreators.h>
//	#include <Finder.h>
//	#include <FinderRegistry.h>
//	#include <FixMath.h>
	#include <Folders.h>
//	#include <Fonts.h>
	#include <Gestalt.h>
//	#include <HFSVolumes.h>
	#include <Icons.h>
//	#include <ImageCompression.h>
	#include <InternetConfig.h>
//	#include <IntlResources.h>
//	#include <Lists.h>
//	#include <LowMem.h>
	#include <MacErrors.h>				/* New for 3.3.1 */
//	#include <MacHelp.h>				/* New for 3.3.1 */
	#include <MacMemory.h>
	#include <MacTypes.h>
	#include <MacWindows.h>
	#include <Menus.h>
//	#include <MixedMode.h>
//	#include <Movies.h>
//	#include <NameRegistry.h>
	#include <Navigation.h>				/* New for 3.2	 */
//	#include <Notification.h>
//	#include <NumberFormatting.h>
//	#include <OSA.h>
//	#include <OSAComp.h>
//	#include <OSAGeneric.h>
//	#include <OSUtils.h>
//	#include <Palettes.h>
//	#include <Patches.h>
//	#include <PPCToolbox.h>
	#include <Printing.h>
//	#include <Processes.h>
	#include <QDOffscreen.h>
	#include <Quickdraw.h>
//	#include <QuickdrawText.h>
	#include <Resources.h>
	#include <Scrap.h>
//	#include <Script.h>
	#include <SegLoad.h>
	#include <Sound.h>
	#include <StandardFile.h>
	#include <StringCompare.h>
//	#include <TextCommon.h>
//	#include <TextEdit.h>
	#include <TextUtils.h>
//	#include <Threads.h>
//	#include <Timer.h>
	#include <ToolUtils.h>
//	#include <Translation.h>
//	#include <TranslationExtensions.h>
	#include <Traps.h>
//	#include <UTCUtils.h>				/* New for 3.3.1 */
//	#include <Video.h>
