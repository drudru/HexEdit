#   File:       Makefile
#   Target:     HexEdit-MPW
#   Created:    Thursday, May 10, 2001 04:49:38 PM


MAKEFILE        = Makefile
¥MondoBuild¥    = {MAKEFILE}  # Make blank to avoid rebuilds when makefile is modified
TargetName		= HexEdit

ObjDir          = :Obj:
Includes        = -i :

SymOpt			= -sym off
Sym-PPC         = {SymOpt}
Sym-68K         = {SymOpt}

CommonCOptions	= -proto strict -opt speed -d __MPW__ {Includes}

PPCCOptions     = {Sym-PPC} {CommonCOptions} -w 21 -d TARGET_API_MAC_CARBON=1
COptions        = {Sym-68K} {CommonCOptions} -model far


### Source Files ###

SrcFiles        =  ¶
				  :Source:AboutBox.c ¶
				  :Source:EditRoutines.c ¶
				  :Source:EditScrollbar.c ¶
				  :Source:EditWindow.c ¶
				  :Source:HexCompare.c ¶
				  :Source:HexSearch.c ¶
				  :Source:Main.c ¶
				  :Source:Menus.c ¶
				  :Source:ObjectWindow.c ¶
				  :Source:Prefs.c ¶
				  :Source:UtilFuncs.c


### Object Files ###

ObjFiles-PPC    =  ¶
				  "{ObjDir}AboutBox.c.x" ¶
				  "{ObjDir}EditRoutines.c.x" ¶
				  "{ObjDir}EditScrollbar.c.x" ¶
				  "{ObjDir}EditWindow.c.x" ¶
				  "{ObjDir}HexCompare.c.x" ¶
				  "{ObjDir}HexSearch.c.x" ¶
				  "{ObjDir}Main.c.x" ¶
				  "{ObjDir}Menus.c.x" ¶
				  "{ObjDir}ObjectWindow.c.x" ¶
				  "{ObjDir}Prefs.c.x" ¶
				  "{ObjDir}UtilFuncs.c.x"

ObjFiles-68K    =  ¶
				  "{ObjDir}AboutBox.c.o" ¶
				  "{ObjDir}EditRoutines.c.o" ¶
				  "{ObjDir}EditScrollbar.c.o" ¶
				  "{ObjDir}EditWindow.c.o" ¶
				  "{ObjDir}HexCompare.c.o" ¶
				  "{ObjDir}HexSearch.c.o" ¶
				  "{ObjDir}Main.c.o" ¶
				  "{ObjDir}Menus.c.o" ¶
				  "{ObjDir}ObjectWindow.c.o" ¶
				  "{ObjDir}Prefs.c.o" ¶
				  "{ObjDir}UtilFuncs.c.o"

RsrcFiles		= ¶
					:Resources:HexEdit.rsrc ¶
					:Resources:Version.rsrc ¶
					:Resources:VOODOO.rsrc ¶
					 ¶
					:Resources:Icons:Cortes.rsrc ¶
					#

### Libraries ###

LibFiles-PPC    =  ¶
				  "{SharedLibraries}CarbonLib" ¶
				  "{SharedLibraries}StdCLib" ¶
				  "{PPCLibraries}StdCRuntime.o" ¶
				  "{PPCLibraries}PPCCRuntime.o" ¶
				  "{PPCLibraries}PPCToolLibs.o"

LibFiles-68K    =  ¶
				  "{Libraries}MathLib.o" ¶
				  "{CLibraries}StdCLib.o" ¶
				  "{Libraries}MacRuntime.o" ¶
				  "{Libraries}IntEnv.o" ¶
				  "{Libraries}ToolLibs.o" ¶
				  "{Libraries}Interface.o"


### Default Rules ###

:Obj:	Ä	:Source:

.c.x  Ä  .c  {¥MondoBuild¥}
	{PPCC} {depDir}{default}.c -o {targDir}{default}.c.x {PPCCOptions}

.c.o  Ä  .c  {¥MondoBuild¥}
	{C} {depDir}{default}.c -o {targDir}{default}.c.o {COptions}


### Build Rules ###

{TargetName}  ÄÄ	 {RsrcFiles}
	for f in {NewerDeps}
		echo "include ¶"{f}¶";"
	end | rez -a -o {Targ}

{TargetName}  ÄÄ  {ObjFiles-PPC} {LibFiles-PPC} {¥MondoBuild¥}
	PPCLink ¶
		-o {Targ} ¶
		{ObjFiles-PPC} ¶
		{LibFiles-PPC} ¶
		{Sym-PPC} ¶
		-mf -d ¶
		-m __appstart ¶
		-t 'APPL' ¶
		-c 'hDmp'


{TargetName}  ÄÄ  {ObjFiles-68K} {LibFiles-68K} {¥MondoBuild¥}
	ILink ¶
		-o {Targ} ¶
		{ObjFiles-68K} ¶
		{LibFiles-68K} ¶
		{Sym-68K} ¶
		-mf -d ¶
		-t 'APPL' ¶
		-c 'hDmp' ¶
		-model far ¶
		-state rewrite ¶
		-compact -pad 0
	If "{Sym-68K}" =~ /-sym Å[nNuU]Å/
		ILinkToSYM {Targ}.NJ -mf -sym 3.2 -c 'sade'
	End


{TargetName}  ÄÄ  {¥MondoBuild¥}
	Echo "Data 'carb' (0) ¶{$¶"00000000¶"¶};" | Rez -o {Targ} -append

{TargetName}  ÄÄ  {¥MondoBuild¥}
	Echo ¶
		"#include ¶"Processes.r¶"¶n ¶
		Resource 'SIZE' (-1) ¶{ ¶
		reserved, ¶
		acceptSuspendResumeEvents, ¶
		reserved, ¶
		canBackground, ¶
		doesActivateOnFGSwitch, ¶
		backgroundAndForeground, ¶
		dontGetFrontClicks, ¶
		ignoreAppDiedEvents, ¶
		is32BitCompatible, ¶
		isHighLevelEventAware, ¶
		onlyLocalHLEvents, ¶
		notStationeryAware, ¶
		dontUseTextEditServices, ¶
		notDisplayManagerAware, ¶
		reserved, ¶
		reserved, ¶
		1024 * 1024, ¶
		1024 * 1024 ¶};" | Rez -o {Targ} -append

Clean	ÄÄ	$OutOfDate
	Delete -i -y ¶
		{ObjFiles-PPC} ¶
		{ObjFiles-68k} ¶
		{TargetName} ¶
		{TargetName}.NJ ¶
		{TargetName}.SYM ¶
		{TargetName}.xcoff ¶
		{TargetName}.makeout ¶
		#

### Optional Dependencies ###
### Build this target to generate "include file" dependencies. ###

Dependencies  Ä  $OutOfDate
	MakeDepend ¶
		-append {MAKEFILE} ¶
		-ignore "{CIncludes}" ¶
		-objdir "{ObjDir}" ¶
		-objext .x ¶
		-objext .o ¶
		{Includes} ¶
		{SrcFiles}


