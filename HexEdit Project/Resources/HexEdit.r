#include <AppleEvents.r>
#include <AEUserTermTypes.r>
#include <MacTypes.r>
#include <Dialogs.r>
#include <Controls.r>
#include <Menus.r>
#include <Icons.r>
#include <Finder.r>

resource 'ALRT' (10000, "SaveChangesALRT") {
	{40, 40, 136, 392},
	10000,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	},
	alertPositionMainScreen
};

resource 'ALRT' (10001, "ErrorALRT") {
	{62, 29, 206, 389},
	10001,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	},
	alertPositionMainScreen
};

resource 'ALRT' (10002, "NoForkALRT") {
	{65, 40, 241, 440},
	10002,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	},
	alertPositionMainScreen
};

resource 'ALRT' (10003, "RevertALRT") {
	{86, 32, 206, 384},
	10003,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	},
	alertPositionMainScreen
};

resource 'BNDL' (128) {
	'hDmp',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'ICN#',
		{	/* array IDArray: 4 elements */
			/* [1] */
			0, 128,
			/* [2] */
			2, 129,
			/* [3] */
			1, 130,
			/* [4] */
			3, 0
		},
		/* [2] */
		'FREF',
		{	/* array IDArray: 4 elements */
			/* [1] */
			0, 128,
			/* [2] */
			2, 130,
			/* [3] */
			1, 129,
			/* [4] */
			3, 131
		}
	}
};

resource 'CNTL' (128, "Nav Radio Group", purgeable) {
	{10, 16, 50, 366},
	0,
	visible,
	100,
	0,
	416,
	0,
	"Open Fork:"
};

resource 'DITL' (128) {
	{	/* array DITLarray: 13 elements */
		/* [1] */
		{96, 364, 120, 445},
		Button {
			enabled,
			"Find Next"
		},
		/* [2] */
		{96, 244, 120, 349},
		Button {
			enabled,
			"Find Previous"
		},
		/* [3] */
		{66, 106, 84, 160},
		RadioButton {
			enabled,
			"Hex"
		},
		/* [4] */
		{66, 163, 84, 224},
		RadioButton {
			enabled,
			"ASCII"
		},
		/* [5] */
		{8, 112, 24, 442},
		EditText {
			enabled,
			""
		},
		/* [6] */
		{8, 64, 24, 104},
		StaticText {
			disabled,
			"Find:"
		},
		/* [7] */
		{66, 264, 84, 380},
		CheckBox {
			enabled,
			"Case Sensitive"
		},
		/* [8] */
		{66, 34, 82, 98},
		StaticText {
			disabled,
			"Matching:"
		},
		/* [9] */
		{40, 8, 56, 104},
		StaticText {
			disabled,
			"Replace with:"
		},
		/* [10] */
		{40, 112, 56, 442},
		EditText {
			enabled,
			""
		},
		/* [11] */
		{96, 36, 120, 108},
		Button {
			enabled,
			"Replace"
		},
		/* [12] */
		{96, 124, 120, 220},
		Button {
			enabled,
			"Replace All"
		},
		/* [13] */
		{66, 390, 84, 452},
		CheckBox {
			enabled,
			"Wrap"
		}
	}
};

resource 'DITL' (129) {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{64, 112, 86, 177},
		Button {
			enabled,
			"GO"
		},
		/* [2] */
		{40, 25, 56, 69},
		StaticText {
			disabled,
			"Entry:"
		},
		/* [3] */
		{16, 75, 32, 179},
		EditText {
			enabled,
			""
		},
		/* [4] */
		{39, 69, 56, 116},
		RadioButton {
			enabled,
			"Hex"
		},
		/* [5] */
		{39, 116, 56, 192},
		RadioButton {
			enabled,
			"Decimal"
		},
		/* [6] */
		{16, 6, 32, 70},
		StaticText {
			disabled,
			"Address:"
		}
	}
};

resource 'DITL' (130) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{4, 144, 28, 256},
		Button {
			enabled,
			"Find Next"
		},
		/* [2] */
		{4, 368, 28, 432},
		Button {
			enabled,
			"Done"
		},
		/* [3] */
		{4, 8, 28, 136},
		Button {
			enabled,
			"Find Previous"
		},
		/* [4] */
		{4, 272, 28, 360},
		Button {
			enabled,
			"Edit Them"
		}
	}
};

resource 'DITL' (131) {
	{	/* array DITLarray: 10 elements */
		/* [1] */
		{98, 174, 120, 243},
		Button {
			enabled,
			"Done"
		},
		/* [2] */
		{98, 86, 120, 157},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{25, 24, 41, 96},
		RadioButton {
			enabled,
			"Bytes"
		},
		/* [4] */
		{44, 24, 60, 96},
		RadioButton {
			enabled,
			"Words"
		},
		/* [5] */
		{63, 24, 79, 96},
		RadioButton {
			enabled,
			"Longs"
		},
		/* [6] */
		{24, 133, 40, 240},
		RadioButton {
			enabled,
			"Differences"
		},
		/* [7] */
		{44, 133, 60, 240},
		RadioButton {
			enabled,
			"Matches"
		},
		/* [8] */
		{6, 117, 23, 218},
		StaticText {
			disabled,
			"Compare:"
		},
		/* [9] */
		{6, 10, 23, 101},
		StaticText {
			disabled,
			"Compare by:"
		},
		/* [10] */
		{65, 133, 80, 248},
		CheckBox {
			enabled,
			"Case Sensitive"
		}
	}
};

resource 'DITL' (1401, purgeable) {
	{	/* array DITLarray: 14 elements */
		/* [1] */
		{202, 256, 224, 336},
		Button {
			enabled,
			"Open"
		},
		/* [2] */
		{13, 16701, 93, 16719},
		Button {
			enabled,
			"Hidden"
		},
		/* [3] */
		{168, 256, 190, 336},
		Button {
			enabled,
			"Cancel"
		},
		/* [4] */
		{47, 232, 67, 347},
		UserItem {
			disabled
		},
		/* [5] */
		{84, 256, 104, 336},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{117, 256, 136, 336},
		Button {
			enabled,
			"Drive"
		},
		/* [7] */
		{48, 12, 224, 232},
		UserItem {
			enabled
		},
		/* [8] */
		{48, 229, 224, 246},
		UserItem {
			enabled
		},
		/* [9] */
		{152, 252, 153, 340},
		UserItem {
			disabled
		},
		/* [10] */
		{13, 16623, 114, 16719},
		StaticText {
			disabled,
			""
		},
		/* [11] */
		{232, 16, 248, 80},
		RadioButton {
			enabled,
			"Data"
		},
		/* [12] */
		{232, 83, 248, 176},
		RadioButton {
			enabled,
			"Resource"
		},
		/* [13] */
		{232, 177, 248, 248},
		RadioButton {
			enabled,
			"Auto"
		},
		/* [14] */
		{8, 8, 24, 232},
		StaticText {
			disabled,
			"Open File:"
		}
	}
};

resource 'DITL' (10000) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{64, 256, 86, 336},
		Button {
			enabled,
			"Save"
		},
		/* [2] */
		{64, 168, 86, 240},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{64, 64, 86, 144},
		Button {
			enabled,
			"Discard"
		},
		/* [4] */
		{8, 64, 48, 344},
		StaticText {
			disabled,
			"Save changes to ^0?"
		}
	}
};

resource 'DITL' (10001) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{112, 272, 134, 347},
		Button {
			enabled,
			"Bummer!"
		},
		/* [2] */
		{8, 64, 104, 352},
		StaticText {
			disabled,
			"^0"
		}
	}
};

resource 'DITL' (10002) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{144, 320, 166, 386},
		Button {
			enabled,
			"No"
		},
		/* [2] */
		{144, 232, 166, 298},
		Button {
			enabled,
			"Yes"
		},
		/* [3] */
		{8, 72, 72, 392},
		StaticText {
			disabled,
			"^0 has no ^1 fork.  \n\nDo you wish to cre"
			"ate this fork?"
		},
		/* [4] */
		{72, 24, 96, 72},
		StaticText {
			disabled,
			"NOTE:"
		},
		/* [5] */
		{72, 72, 136, 392},
		StaticText {
			disabled,
			"Creating the fork will allow you to add "
			"data to the file, in the fork specified "
			"above, but unless you know why you are c"
			"reating the fork it is best to answer ‘N"
			"o’ here."
		}
	}
};

resource 'DITL' (10003) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{80, 256, 102, 336},
		Button {
			enabled,
			"Revert"
		},
		/* [2] */
		{80, 168, 102, 240},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{8, 64, 72, 344},
		StaticText {
			disabled,
			"Revert “^0”?\n\nAll current changes will b"
			"e lost!"
		}
	}
};

resource 'DITL' (15000) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{0, 0, 40, 350},
		Control {
			enabled,
			128
		},
		/* [2] */
		{20, 12, 36, 80},
		RadioButton {
			enabled,
			"Data"
		},
		/* [3] */
		{20, 128, 36, 225},
		RadioButton {
			enabled,
			"Resource"
		},
		/* [4] */
		{20, 272, 36, 347},
		RadioButton {
			enabled,
			"Auto"
		}
	}
};

data 'DLGX' (128) {
	$"0647 6164 6765 7400 0000 0000 0000 0000"            /* .Gadget......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"000C 0000 0000 0000 0008 0008 0000 0000"            /* ................ */
	$"000C 0002 0000 0000 0000 0000 0000 0002"            /* ................ */
	$"0000 0000 0000 0000 0000 0004 0000 0000"            /* ................ */
	$"0000 0000 0000 0004 0000 0000 0000 0000"            /* ................ */
	$"0000 0007 0000 0000 0000 0000 0000 0006"            /* ................ */
	$"0000 0000 0000 0000 0000 0003 0000 0000"            /* ................ */
	$"0000 0000 0000 0006 0000 0000 0000 0000"            /* ................ */
	$"0000 0006 0000 0000 0000 0000 0000 0007"            /* ................ */
	$"0000 0000 0000 0000 0000 0002 0000 0000"            /* ................ */
	$"0000 0000 0000 0002 0000 0000 0000 0000"            /* ................ */
	$"0000"                                               /* .. */
};

data 'DLGX' (129) {
	$"0647 6164 6765 7400 0000 0000 0000 0000"            /* .Gadget......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"000C 0000 0000 0000 0008 0008 0000 0000"            /* ................ */
	$"0006 0002 0000 0000 0000 0000 0000 0006"            /* ................ */
	$"0000 0000 0000 0000 0000 0007 0000 0000"            /* ................ */
	$"0000 0000 0000 0004 0000 0000 0000 0000"            /* ................ */
	$"0000 0004 0000 0000 0000 0000 0000 0006"            /* ................ */
	$"0000 0000 0000 0000 0000"                           /* .......... */
};

data 'DLGX' (130) {
	$"0647 6164 6765 7400 0000 0000 0000 0000"            /* .Gadget......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"000C 0000 0000 0000 0008 0008 0000 0000"            /* ................ */
	$"0004 0002 0000 0000 0000 0000 0000 0002"            /* ................ */
	$"0000 0000 0000 0000 0000 0002 0000 0000"            /* ................ */
	$"0000 0000 0000 0002 0000 0000 0000 0000"            /* ................ */
	$"0000"                                               /* .. */
};

data 'DLGX' (131) {
	$"0647 6164 6765 7400 0000 0000 0000 0000"            /* .Gadget......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"000C 0000 0000 0000 0004 0004 0000 0000"            /* ................ */
	$"000A 0002 0000 0000 0000 0000 0000 0001"            /* .¬.............. */
	$"0000 0000 0000 0000 0000 0004 0000 0000"            /* ................ */
	$"0000 0000 0000 0004 0000 0000 0000 0000"            /* ................ */
	$"0000 0004 0000 0000 0000 0000 0000 0004"            /* ................ */
	$"0000 0000 0000 0000 0000 0004 0000 0000"            /* ................ */
	$"0000 0000 0000 0006 0000 0000 0000 0000"            /* ................ */
	$"0000 0006 0000 0000 0000 0000 0000 0003"            /* ................ */
	$"0000 0000 0000 0000 0000"                           /* .......... */
};

data 'DLGX' (1401) {
	$"0647 6164 6765 7400 0000 0000 0000 0000"            /* .Gadget......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"000C 0000 0000 0000 0008 0008 0000 0000"            /* ................ */
	$"000E 0002 0000 0000 0000 0000 0000 0002"            /* ................ */
	$"0000 0000 0000 0000 0000 0002 0000 0000"            /* ................ */
	$"0000 0000 0000 000A 0000 0000 0000 0000"            /* .......¬........ */
	$"0000 0002 0000 0000 0000 0000 0000 0002"            /* ................ */
	$"0000 0000 0000 0000 0000 000A 0000 0000"            /* ...........¬.... */
	$"0000 0000 0000 000A 0000 0000 0000 0000"            /* .......¬........ */
	$"0000 000A 0000 0000 0000 0000 0000 0006"            /* ...¬............ */
	$"0000 0000 0000 0000 0000 0004 0000 0000"            /* ................ */
	$"0000 0000 0000 0004 0000 0000 0000 0000"            /* ................ */
	$"0000 0004 0000 0000 0000 0000 0000 0006"            /* ................ */
	$"0000 0000 0000 0000 0000"                           /* .......... */
};

data 'DLGX' (10000) {
	$"0647 6164 6765 7400 0000 0000 0000 0000"            /* .Gadget......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"000C 0000 0000 0002 0008 0008 0000 0000"            /* ................ */
	$"0004 0002 0000 0000 0000 0000 0000 0001"            /* ................ */
	$"0000 0000 0000 0000 0000 0002 0000 0000"            /* ................ */
	$"0000 0000 0000 0006 0000 0000 0000 0000"            /* ................ */
	$"0000"                                               /* .. */
};

data 'DLGX' (10001) {
	$"0647 6164 6765 7400 0000 0000 0000 0000"            /* .Gadget......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"000C 0000 0000 0003 0008 0008 0000 0000"            /* ................ */
	$"0002 0002 0000 0000 0000 0000 0000 0006"            /* ................ */
	$"0000 0000 0000 0000 0000"                           /* .......... */
};

data 'DLGX' (10002) {
	$"0647 6164 6765 7400 0000 0000 0000 0000"            /* .Gadget......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"000C 0000 0000 0003 0008 0008 0000 0000"            /* ................ */
	$"0005 0002 0000 0000 0000 0000 0000 0002"            /* ................ */
	$"0000 0000 0000 0000 0000 0006 0000 0000"            /* ................ */
	$"0000 0000 0000 0006 0000 0000 0000 0000"            /* ................ */
	$"0000 0006 0000 0000 0000 0000 0000"                 /* .............. */
};

data 'DLGX' (10003) {
	$"0647 6164 6765 7400 0000 0000 0000 0000"            /* .Gadget......... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"000C 0000 0000 0002 0008 0008 0000 0000"            /* ................ */
	$"0003 0002 0000 0000 0000 0000 0000 0001"            /* ................ */
	$"0000 0000 0000 0000 0000 0006 0000 0000"            /* ................ */
	$"0000 0000 0000"                                     /* ...... */
};

data 'DLGX' (15000) {
	$"0843 6861 7263 6F61 6C00 0000 0000 0000"            /* .Charcoal....... */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"000C 0000 0000 0000 0004 0004 0000 0000"            /* ................ */
	$"0004 0005 0000 0000 0000 0000 0000 0004"            /* ................ */
	$"0000 0000 0000 0000 0000 0004 0000 0000"            /* ................ */
	$"0000 0000 0000 0004 0000 0000 0000 0000"            /* ................ */
	$"0000"                                               /* .. */
};

resource 'DLOG' (128, "FindDLOG") {
	{70, 71, 198, 531},
	noGrowDocProc,
	invisible,
	goAway,
	0x0,
	128,
	"",
	alertPositionMainScreen
};

resource 'DLOG' (129, "GotoDLOG") {
	{274, 107, 370, 299},
	noGrowDocProc,
	invisible,
	goAway,
	0x0,
	129,
	"Go To Address",
	centerParentWindow
};

resource 'DLOG' (130, "CompareDLOG") {
	{72, 74, 104, 514},
	plainDBox,
	invisible,
	noGoAway,
	0x0,
	130,
	"",
	alertPositionMainScreen
};

resource 'DLOG' (131, "ComparePref") {
	{62, 73, 190, 329},
	movableDBoxProc,
	invisible,
	noGoAway,
	0x0,
	131,
	"Compare Prefs",
	alertPositionMainScreen
};

resource 'DLOG' (1401, "OpenDLOG", purgeable) {
	{35, 27, 291, 379},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	1401,
	"",
	noAutoCenter
};

resource 'DLOG' (15000, purgeable) {
	{584, 621, 624, 973},
	1024,
	visible,
	goAway,
	0x0,
	15000,
	"DITL 15000 from HexEdit.rsrc",
	noAutoCenter
};

resource 'FREF' (128) {
	'APPL',
	0,
	""
};

resource 'FREF' (129) {
	'BINA',
	1,
	""
};

resource 'FREF' (130) {
	'TEXT',
	2,
	""
};

resource 'FREF' (131) {
	'****',
	3,
	""
};

resource 'MBAR' (128, "Classic Menu Bar") {
	{	/* array MenuArray: 7 elements */
		/* [1] */
		128,
		/* [2] */
		129,
		/* [3] */
		130,
		/* [4] */
		131,
		/* [5] */
		132,
		/* [6] */
		133,
		/* [7] */
		134
	}
};

resource 'MBAR' (129, "OS X Menu Bar") {
	{	/* array MenuArray: 7 elements */
		/* [1] */
		128,
		/* [2] */
		135,
		/* [3] */
		130,
		/* [4] */
		131,
		/* [5] */
		132,
		/* [6] */
		133,
		/* [7] */
		134
	}
};

resource 'MENU' (128) {
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{	/* array: 2 elements */
		/* [1] */
		"About HexEdit…", noIcon, "/", noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (129, "Classic File Menu") {
	129,
	textMenuProc,
	0x7FFFB777,
	enabled,
	"File",
	{	/* array: 16 elements */
		/* [1] */
		"New", noIcon, "N", noMark, plain,
		/* [2] */
		"Open…", noIcon, "O", noMark, plain,
		/* [3] */
		"Close", noIcon, "W", noMark, plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"Disassemble PPC Code", noIcon, "D", noMark, plain,
		/* [6] */
		"Edit Other Fork", noIcon, "E", noMark, plain,
		/* [7] */
		"Compare Files…", noIcon, "K", noMark, plain,
		/* [8] */
		"-", noIcon, noKey, noMark, plain,
		/* [9] */
		"Save", noIcon, "S", noMark, plain,
		/* [10] */
		"Save as…", noIcon, "ß", noMark, plain,
		/* [11] */
		"Revert", noIcon, noKey, noMark, plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"Page Setup…", noIcon, noKey, noMark, plain,
		/* [14] */
		"Print…", noIcon, "P", noMark, plain,
		/* [15] */
		"-", noIcon, noKey, noMark, plain,
		/* [16] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'MENU' (130) {
	130,
	textMenuProc,
	0x7FFFFFBD,
	enabled,
	"Edit",
	{	/* array: 8 elements */
		/* [1] */
		"Undo", noIcon, "Z", noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Cut", noIcon, "X", noMark, plain,
		/* [4] */
		"Copy", noIcon, "C", noMark, plain,
		/* [5] */
		"Paste", noIcon, "V", noMark, plain,
		/* [6] */
		"Clear", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Select All", noIcon, "A", noMark, plain
	}
};

resource 'MENU' (131) {
	131,
	textMenuProc,
	0x7FFFFFEF,
	enabled,
	"Find",
	{	/* array: 6 elements */
		/* [1] */
		"Find & Replace…", noIcon, "F", noMark, plain,
		/* [2] */
		"Find Next", noIcon, "G", noMark, plain,
		/* [3] */
		"Find Previous", noIcon, "B", noMark, plain,
		/* [4] */
		"Replace & Find Next", noIcon, "L", noMark, plain,
		/* [5] */
		"-", noIcon, noKey, noMark, plain,
		/* [6] */
		"Go To Address…", noIcon, "J", noMark, plain
	}
};

resource 'MENU' (132) {
	132,
	textMenuProc,
	0x7FFFF6DF,
	enabled,
	"Options",
	{	/* array: 13 elements */
		/* [1] */
		"Show Extended Chars", noIcon, noKey, noMark, plain,
		/* [2] */
		"Use Decimal Addresses", noIcon, noKey, noMark, plain,
		/* [3] */
		"Display Vertical Bars", noIcon, noKey, noMark, plain,
		/* [4] */
		"Display only Full Lines", noIcon, noKey, noMark, plain,
		/* [5] */
		"Use Overwrite Mode", noIcon, "I", noMark, plain,
		/* [6] */
		"Destructive Delete", noIcon, "D", noMark, plain,
		/* [7] */
		"Paging Only Moves Display", noIcon, "M", noMark, plain,
		/* [8] */
		"Copy Hex Data Unformatted", noIcon, "U", noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Make Backups", noIcon, noKey, noMark, plain,
		/* [11] */
		"Open File Dialog at launch", noIcon, noKey, noMark, plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"File Comparison Options…", noIcon, ",", noMark, plain
	}
};

resource 'MENU' (133) {
	133,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	"Color Scheme",
	{	/* array: 2 elements */
		/* [1] */
		"c", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (134) {
	134,
	textMenuProc,
	allEnabled,
	enabled,
	"Window",
	{	/* array: 0 elements */
	}
};

resource 'MENU' (135, "OS X File Menu") {
	129,
	textMenuProc,
	0x7FFFF777,
	enabled,
	"File",
	{	/* array: 14 elements */
		/* [1] */
		"New", noIcon, "N", noMark, plain,
		/* [2] */
		"Open…", noIcon, "O", noMark, plain,
		/* [3] */
		"Close", noIcon, "W", noMark, plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"Disassemble PPC Code", noIcon, "D", noMark, plain,
		/* [6] */
		"Edit Other Fork", noIcon, "E", noMark, plain,
		/* [7] */
		"Compare Files…", noIcon, "K", noMark, plain,
		/* [8] */
		"-", noIcon, noKey, noMark, plain,
		/* [9] */
		"Save", noIcon, "S", noMark, plain,
		/* [10] */
		"Save as…", noIcon, "ß", noMark, plain,
		/* [11] */
		"Revert", noIcon, noKey, noMark, plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"Page Setup…", noIcon, noKey, noMark, plain,
		/* [14] */
		"Print…", noIcon, "P", noMark, plain
	}
};

data 'Mcmd' (1) {
	$"0000"                                               /* .. */
};

data 'Mcmd' (128) {
	$"0000"                                               /* .. */
};

data 'Mcmd' (129) {
	$"0010 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000"                                               /* .. */
};

data 'Mcmd' (130) {
	$"0008 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000"                                               /* .. */
};

data 'Mcmd' (131) {
	$"0006 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000"                           /* .......... */
};

data 'Mcmd' (132) {
	$"000D 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000"                                     /* ...... */
};

data 'Mcmd' (133) {
	$"0000"                                               /* .. */
};

data 'Mcmd' (134) {
	$"0000"                                               /* .. */
};

data 'Mcmd' (135) {
	$"000E 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000"                           /* .......... */
};

resource 'STR#' (128, "Undos", purgeable) {
	{	/* array StringArray: 9 elements */
		/* [1] */
		"Undo ",
		/* [2] */
		"Redo ",
		/* [3] */
		"Typing",
		/* [4] */
		"Paste",
		/* [5] */
		"Insert",
		/* [6] */
		"Overwrite",
		/* [7] */
		"Cut",
		/* [8] */
		"Clear",
		/* [9] */
		"Editing"
	}
};

resource 'STR#' (129, "Printing", purgeable) {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"Print…",
		/* [2] */
		"Print Selection…"
	}
};

resource 'STR#' (130, "Header", purgeable) {
	{	/* array StringArray: 5 elements */
		/* [1] */
		" - Data",
		/* [2] */
		" - Rsrc",
		/* [3] */
		"Len: %9ld | Type/Creator: %4.4s/%4.4s | "
		"Sel: %9ld:%9ld / %8ld",
		/* [4] */
		"Len: $%8.8lX | Type/Creator: %4.4s/%4.4s"
		" | Sel: $%8.8lX:%8.8lX / $%8.8lX",
		/* [5] */
		"File:"
	}
};

resource 'STR#' (131, "Error", purgeable) {
	{	/* array StringArray: 20 elements */
		/* [1] */
		"Out of Memory",
		/* [2] */
		"Unable to seek into file (%d)",
		/* [3] */
		"Unable to read data from file (%d)",
		/* [4] */
		"Unable to set file position (%d)",
		/* [5] */
		"Unable to write to file (%d)",
		/* [6] */
		"Unable to paste, probably not enough mem"
		"ory (%d)",
		/* [7] */
		"FindFolder failed (%d)",
		/* [8] */
		"Unable to create file (%d)",
		/* [9] */
		"Unable to open file (%d)\n\n(The file is l"
		"ikely already open in HexEdit or another"
		" program, especially if you got error -4"
		"9)",
		/* [10] */
		"Unable to get file info (%d)",
		/* [11] */
		"Selected pages are out of range 1-%d",
		/* [12] */
		"Unable to SET file info (%d)",
		/* [13] */
		"Can't backup original - backup file is o"
		"pen (%d)",
		/* [14] */
		"Unable to rename file (%d)",
		/* [15] */
		"Can't save - the disk is full.\tTry freei"
		"ng some space or using another disk.",
		/* [16] */
		"Not a valid value!",
		/* [17] */
		"No default printer has been specified.",
		/* [18] */
		"A printing error has occurred.",
		/* [19] */
		"Unable to save file (%d)\n\nA -43 error in"
		"dicates the file may have been moved or "
		"put in the trash. A -49 error means the "
		"file may have been opened by another pro"
		"gram.",
		/* [20] */
		"This window’s file is read-only, meaning"
		" you can’t make or save changes to the e"
		"xisting file!\n\nTo save changes, select “"
		"Save As…” from the File menu."
	}
};

resource 'STR#' (132, "Color", purgeable) {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"Color Windows",
		/* [2] */
		"B&W Windows"
	}
};

resource 'STR#' (133, "Prompt", purgeable) {
	{	/* array StringArray: 4 elements */
		/* [1] */
		"Save File As...",
		/* [2] */
		"Select the file to open…",
		/* [3] */
		"Select first file to compare...",
		/* [4] */
		"Select file to compare against..."
	}
};

resource 'STR#' (134, "Filenames", purgeable) {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"Preferences",
		/* [2] */
		"HexEdit Preferences",
		/* [3] */
		"Untitled",
		/* [4] */
		"DATA",
		/* [5] */
		"RSRC"
	}
};

resource 'STR#' (135, "Font Face and Size", purgeable) {
	{	/* array StringArray: 2 elements */
		/* [1] */
		"Monaco",
		/* [2] */
		"9"
	}
};

resource 'WIND' (128, "System 7") {
	{46, 15, 412, 331},
	zoomDocProc,
	invisible,
	goAway,
	0x3E8,
	"HexEdit Window",
	staggerParentWindow
};

resource 'WIND' (129, "Appearance") {
	{257, 342, 550, 841},
	1031,
	invisible,
	goAway,
	0x3E8,
	"HexEdit Window",
	staggerParentWindowScreen
};

resource 'actb' (10000, "SaveChangesALRT") {
	{	/* array ColorSpec: 1 elements */
		/* [1] */
		wContentColor, 61166, 61166, 61166
	}
};

resource 'actb' (10001, "ErrorALRT") {
	{	/* array ColorSpec: 1 elements */
		/* [1] */
		wContentColor, 61602, 47434, 49089
	}
};

resource 'actb' (10002, "NoForkALRT") {
	{	/* array ColorSpec: 1 elements */
		/* [1] */
		wContentColor, 61166, 61166, 61166
	}
};

resource 'actb' (10003, "RevertALRT") {
	{	/* array ColorSpec: 1 elements */
		/* [1] */
		wContentColor, 61166, 61166, 61166
	}
};

resource 'alrx' (10000, "SaveChangesALRT", purgeable) {
	versionZero {
		13,
		0,
		kUseThemeWindow,
		"Save Your Changes?"
	}
};

resource 'alrx' (10001, "ErrorALRT", purgeable) {
	versionZero {
		13,
		0,
		kUseThemeWindow,
		"An Error Has Occured!"
	}
};

resource 'alrx' (10002, "NoForkALRT", purgeable) {
	versionZero {
		13,
		0,
		kUseThemeWindow,
		"File does not have requested fork!"
	}
};

resource 'alrx' (10003, "RevertALRT", purgeable) {
	versionZero {
		13,
		0,
		kUseThemeWindow,
		"Overwrite your changes with data in file"
		"?"
	}
};

data 'appl' (0) {
	$"0002 00DB"                                          /* ...€ */
};

resource 'dctb' (128, "FindDLOG") {
	{	/* array ColorSpec: 1 elements */
		/* [1] */
		wContentColor, 61166, 61166, 61166
	}
};

resource 'dctb' (129, "GotoDLOG") {
	{	/* array ColorSpec: 1 elements */
		/* [1] */
		wContentColor, 61166, 61166, 61166
	}
};

resource 'dctb' (130, "CompareDLOG") {
	{	/* array ColorSpec: 1 elements */
		/* [1] */
		wContentColor, 61166, 61166, 61166
	}
};

resource 'dctb' (131, "ComparePref") {
	{	/* array ColorSpec: 1 elements */
		/* [1] */
		wContentColor, 61166, 61166, 61166
	}
};

resource 'dctb' (1401, "OpenDLOG") {
	{	/* array ColorSpec: 0 elements */
	}
};

resource 'dctb' (15000) {
	{	/* array ColorSpec: 5 elements */
		/* [1] */
		wContentColor, 65535, 65535, 65535,
		/* [2] */
		wFrameColor, 0, 0, 0,
		/* [3] */
		wTextColor, 0, 0, 0,
		/* [4] */
		wHiliteColor, 0, 0, 0,
		/* [5] */
		wTitleBarColor, 65535, 65535, 65535
	}
};

resource 'dftb' (128) {
	versionZero {
		{	/* array FontStyle: 13 elements */
			/* [1] */
			skipItem {

			}			,
			/* [2] */
			skipItem {

			}			,
			/* [3] */
			skipItem {

			}			,
			/* [4] */
			skipItem {

			}			,
			/* [5] */
			skipItem {

			}			,
			/* [6] */
			skipItem {

			}			,
			/* [7] */
			skipItem {

			}			,
			/* [8] */
			skipItem {

			}			,
			/* [9] */
			skipItem {

			}			,
			/* [10] */
			skipItem {

			}			,
			/* [11] */
			skipItem {

			}			,
			/* [12] */
			skipItem {

			}			,
			/* [13] */
			skipItem {

			}		}
	}
};

resource 'dlgx' (128, "FindDLOG") {
	versionZero {
		13
	}
};

resource 'dlgx' (129, "GotoDLOG") {
	versionZero {
		13
	}
};

resource 'dlgx' (130, "CompareDLOG") {
	versionZero {
		13
	}
};

resource 'dlgx' (131, "ComparePref") {
	versionZero {
		13
	}
};

resource 'dlgx' (1401, "OpenDLOG") {
	versionZero {
		9
	}
};

resource 'dlgx' (15000) {
	versionZero {
		9
	}
};

data 'hDmp' (0, "Owner resource") {
	$"2CA9 3139 3933 204A 696D 2042 756D 6761"            /* ,©1993 Jim Bumga */
	$"7264 6E65 720D A920 3139 3935 2D32 3030"            /* rdner.© 1995-200 */
	$"3120 4C61 6E65 2052 6F61 7468 65"                   /* 1 Lane Roathe */
};

data 'ictb' (128) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
};

data 'ictb' (129) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000"                                /* ........ */
};

data 'ictb' (130) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
};

data 'ictb' (131) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000"                                /* ........ */
};

data 'ictb' (1401) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000"                                /* ........ */
};

data 'ictb' (10000) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
};

data 'ictb' (10001) {
	$"0000 0000 0000 0000"                                /* ........ */
};

data 'ictb' (10002) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"800F 0014 0028 0100 000C 7FFF 2000 3003"            /* Ä....(.....ˇ .0. */
	$"FFFF FFFF FFFF 0000 0554 696D 6573"                 /* ˇˇˇˇˇˇ...Times */
};

data 'ictb' (10003) {
	$"0000 0000 0000 0000 0000 0000"                      /* ............ */
};

data 'ictb' (15000) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
};

resource 'mctb' (128) {
	{	/* array MCTBArray: 1 elements */
		/* [1] */
		mctbLast, 0,
		{	/* array: 4 elements */
			/* [1] */
			0, 0, 0,
			/* [2] */
			0, 0, 0,
			/* [3] */
			0, 0, 0,
			/* [4] */
			0, 0, 0
		}
	}
};

resource 'mctb' (129) {
	{	/* array MCTBArray: 1 elements */
		/* [1] */
		mctbLast, 0,
		{	/* array: 4 elements */
			/* [1] */
			0, 0, 0,
			/* [2] */
			0, 0, 0,
			/* [3] */
			0, 0, 0,
			/* [4] */
			0, 0, 0
		}
	}
};

resource 'mctb' (131) {
	{	/* array MCTBArray: 1 elements */
		/* [1] */
		mctbLast, 0,
		{	/* array: 4 elements */
			/* [1] */
			0, 0, 0,
			/* [2] */
			0, 0, 0,
			/* [3] */
			0, 0, 0,
			/* [4] */
			0, 0, 0
		}
	}
};

resource 'mctb' (132) {
	{	/* array MCTBArray: 1 elements */
		/* [1] */
		mctbLast, 0,
		{	/* array: 4 elements */
			/* [1] */
			0, 0, 0,
			/* [2] */
			0, 0, 0,
			/* [3] */
			0, 0, 0,
			/* [4] */
			0, 0, 0
		}
	}
};

resource 'mctb' (135) {
	{	/* array MCTBArray: 1 elements */
		/* [1] */
		mctbLast, 0,
		{	/* array: 4 elements */
			/* [1] */
			0, 0, 0,
			/* [2] */
			0, 0, 0,
			/* [3] */
			0, 0, 0,
			/* [4] */
			0, 0, 0
		}
	}
};

resource 'wctb' (128) {
	{	/* array ColorSpec: 5 elements */
		/* [1] */
		wContentColor, 61166, 61166, 61166,
		/* [2] */
		wFrameColor, 0, 0, 0,
		/* [3] */
		wTextColor, 0, 0, 0,
		/* [4] */
		wHiliteColor, 0, 0, 0,
		/* [5] */
		wTitleBarColor, 65535, 65535, 65535
	}
};

