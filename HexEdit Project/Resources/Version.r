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
/*
	$"4865 7845 6469 740D 0D41 2066 696C 6520"
	$"6564 6974 696E 6720 7574 696C 6974 7920"
	$"616C 6C6F 7769 6E67 2079 6F75 2074 6F20"
	$"6564 6974 2074 6865 2072 6573 6F75 7263"
	$"6520 616E 6420 6461 7461 2066 6F72 6B73"
	$"206F 6620 616E 7920 6669 6C65 2069 6E20"
	$"6865 782C 2064 6563 696D 616C 206F 7220"
	$"4153 4349 492E 2049 7420 616C 736F 2064"
	$"6973 6173 736D 626C 6573 2050 6F77 6572"
	$"5043 2063 6F64 652E" */
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

