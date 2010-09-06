#include <Carbon/Carbon.r>


resource 'STR#' (1, "Product") {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"HexEdit",
		/* [2] */
		"HexEdit",
		/* [3] */
		"2010",
		/* [4] */
		"http://hexedit.sourceforge.net",
		/* [5] */
		"http://download.sourceforge.net/hexedit"
	}
};

resource 'STR#' (2, "Vendor") {
	{	/* array StringArray: 11 elements */
		/* [1] */
		"Lane Roathe",
		/* [2] */
		"Ideas From the Deep",
		/* [3] */
		"P.O.Box 472747, Garland, TX 75047-2747"
		/* [4] */
		"972-840-8415",
		/* [5] */
		"",
		/* [6] */
		"",
		/* [7] */
		"",
		/* [8] */
		"",
		/* [9] */
		"lane@ideasfromthedeep.com",
		/* [10] */
		"http://hexedit.sourceforge.net",
		/* [11] */
		"http://download.sourceforge.net/hexedit"
	}
};

data 'TEXT' (1, "Product Description") {
	"HexEdit\n\n"
	"A file editing utility allowing you to edit the resource "
	"and data forks of any file in hex, decimal or ASCII. "
	"It also disassmbles Power PC code."
};

data 'hfdr' (-5696) {
	$"0002 0000 0000 0000 0000 0001 0006 0006"            /* ................ */
	$"0001"                                               /* .. */
};

data 'styl' (1) {
	$"0001 0000 0000 0010 000C 0000 0002 000C"            /* ................ */
	$"0000 0000 0000"                                     /* ...... */
};

resource 'vers' (1) {
	0x2,
	0x22,
	release,
	0x0,
	0,
	"2.2.2",
	"2.2.2 © 1996-2010 Lane Roathe, based on code by Jim Bumgardner. Thanks to Nick Shanks."
};

resource 'vers' (2) {
	0x2,
	0x22,
	release,
	0x0,
	0,
	"2.2.2",
	"www.ideasfromthedeep.com/hexedit"
};

