HexEdit

Copyright © 1996-2000 Lane Roathe <lane@roathe.com>
Based on the original, Copyright © 1993 Jim Bumgardner <www.jbum.com>

HexEdit is a file editor allowing you to view and edit the data contained withn any file. The original (through v1.07) is from Jim Bumgardner, Dave Polaschek released a version 1.1 and Nick Shanks released a version 1.3, and many of his changes are now part of the official HexEdit release. You can reach Nick at <nick.shanks@virgin.net> or on the web at <http://freespace.virgin.net/nick.shanks/mac/>. I (Lane Roathe) have released "official" versions from v1.1 through the current release. The official HexEdit page is:

<http://www.sourceforge.net/projects/hexedit/>

I hope you find HexEdit useful! If you have suggestions or bug reports please feel free to contact me (see below). As several people can attest, I do listen :) Please be aware, however, that this is a FREE program and no support of any kind is actually being offered.

HexEdit is distributed under the Mozilla Public License. Please refere to the file "license.txt" for details. Use of these sources is an implied agreement to the terms of the license.


Notice of NO Warranty
This program is distributed in the hope that it will be useful, but without any warranty; without even the warranty of merchantability or fitness for any purpose. The entire risk of operation this program, as well as the risk of quality and performance, resides soley with you! In no event with any of the program authors be liable for any damage caused by the operation, or non-operation, of this program!


USING A HEX EDITOR IS DANGEROUS! Be sure to backup your data before operating this program. It is possible to loose the entire  contents of a hard drive (or data on any type of storage device) by using this program!


New features

Too many to list! (Really!) Please see the History.txt file in the Documentation folder if you really want to see how things have evolved.

Compiling the Code

The latest code and HexEdit development community can be found on <http://hexedit.sourceforget.net>. The latest version of CodeWarrior is the "Official" development environment, including the set of Apple's Universal Headers and so forth that shipped with the latest version and all of it's updates.

Other development environments may be supported, just not "officially" :)


From Jim's original Docs

HexEdit is a hexdump viewer and editor that works similarly to the hex editor provided with Apple's ResEdit.  It allows you to edit either the data fork or the resource fork of a file.

I wrote HexEdit because I needed to be able to insert/delete bytes from the data fork of files I was testing, and tools like FEdit don't have insert/deletion.

ÑÑÑÑÑÑÑÑ
Examples of what HexEdit has been/can be used for:

	Debugging:
		Examining debugging data (ie, raw ADB output, etc.)
		Examine other programs' binary output for errors (AIFF, MIDI, etc.)

	Spelunking:
		Examine/Edit the contents of a MIDI file.
		Examine binary files received from the Internet to see what format  they are in.
        (MacBinary, Stuffit, etc.)

	Misc.:
		Figure out how Kanji is encoded in a text file.
		Compare MS-Word format to RTF format.
		Examine the data fork of MS-Word Application
        (Owner Name is stored there in easily decrypted format)
    Recovery of damaged files, or at least portions of data therefrom.
	