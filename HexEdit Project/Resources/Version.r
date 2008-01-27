#include <AppleEvents.r>
#include <AEUserTermTypes.r>
#include <MacTypes.r>
#include <Dialogs.r>
#include <Icons.r>

resource 'STR#' (1, "Product") {
	{	/* array StringArray: 5 elements */
		/* [1] */
		"HexEdit",
		/* [2] */
		"HexEdit",
		/* [3] */
		"2008",
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
		"P.O.Box 705 ¥ Little Elm ¥ TX ¥ 75068-07"
		"05",
		/* [4] */
		"214-618-5496",
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
	$"4865 7845 6469 740D 0D41 2066 696C 6520"            /* HexEdit..A file  */
	$"6564 6974 696E 6720 7574 696C 6974 7920"            /* editing utility  */
	$"616C 6C6F 7769 6E67 2079 6F75 2074 6F20"            /* allowing you to  */
	$"6564 6974 2074 6865 2072 6573 6F75 7263"            /* edit the resourc */
	$"6520 616E 6420 6461 7461 2066 6F72 6B73"            /* e and data forks */
	$"206F 6620 616E 7920 6669 6C65 2069 6E20"            /*  of any file in  */
	$"6865 782C 2064 6563 696D 616C 206F 7220"            /* hex, decimal or  */
	$"4153 4349 492E 2049 7420 616C 736F 2064"            /* ASCII. It also d */
	$"6973 6173 736D 626C 6573 2050 6F77 6572"            /* isassmbles Power */
	$"5043 2063 6F64 652E"                                /* PC code. */
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
	0x20,
	release,
	0x0,
	0,
	"2.2",
	"2.2 ©1996-2008 Lane Roathe, based on cod"
	"e by Jim Bumgardner. Thanks to Nick Shan"
	"ks."
};

resource 'vers' (2) {
	0x2,
	0x20,
	release,
	0x0,
	0,
	"2.2",
	"www.ideasfromthedeep.com/hexedit"
};

