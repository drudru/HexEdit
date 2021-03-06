HexEdit
Copyright © 1996-2008 Lane Roathe <www.roathe.com>
Based on the original, Copyright © 1993 Jim Bumgardner <www.jbum.com>

HexEdit is a file editor allowing you to view and edit the data contained withn any file. The original (through v1.07) is from Jim Bumgardner, Dave Polaschek released a version 1.1 and Nick Shanks released a version 1.3, and many of his changes are now part of the official HexEdit release. You can reach Nick at <hexedit@nickshanks.com> or on the web at <http://nickshanks.com/hexedit>. I (Lane Roathe) have released "official" versions from v1.1 through the current release, and you can get more information at my site <http://www.roathe.com/hexedit.html>. The official HexEdit page is:

<http://hexedit.sf.net>

I hope you find HexEdit useful! If you have suggestions or bug reports please feel free to contact me (see below). As several people can attest, I do listen :) Please be aware, however, that this is a FREE program and no support of any kind is actually being offered.

HexEdit is distributed under the Mozilla Public License. Please refere to the file "license.txt" for details. Use of these sources is an implied agreement to the terms of the license.

* Notice of NO Warranty *
This program is distributed in the hope that it will be useful, but without any warranty; without even the warranty of merchantability or fitness for any purpose. The entire risk of operation this program, as well as the risk of quality and performance, resides soley with you! In no event with any of the program authors be liable for any damage caused by the operation, or non-operation, of this program!

* USING A HEX EDITOR IS DANGEROUS! *
Be sure to backup your data before operating this program. It is possible to loose the entire  contents of a hard drive (or data on any type of storage device) by using this program!

* New features *
Too many to list! (Really!) Please see the History.txt file in the Documentation folder if you really want to see how things have evolved.

* Compiling the Code *
The latest code and HexEdit development community can be found on <http://hexedit.sourceforget.net>. The latest version of CodeWarrior is the "Official" development environment, including the set of Apple's Universal Headers and so forth that shipped with the latest version and all of it's updates.


Other development environments may be supported, just not "officially" :)

* File Mappings *
IMPORTANT NOTE: With the release of HexEdit v2.20 I think this issue has been solved! If you have ever installed HexEdit before, you will need to delete ALL copies of HexEdit from your computer, empty the trash and then put a clean copy of v2.20 on.
Here is the older information in case it helps:

Several people have noticed that .dmg and other OS X files are being mapped to HexEdit when downloaded. This is a major pain, hopefully this section will be able to tell you why it is happening and impart enough information that you will be able to correct it on your system.

Apple has decided that Microsoft has the correct idea, and all file should be typed by their 3 letter extension, and that the old reliable file types are a bad idea. The result is that all Apple programs now do NOT assign the file types and so if your web browser, ftp client or the system settings do know know the 3 letter extension then it will (in most cases) get mapped to the HexEdit file type. The system settings are changed in File Exchange in OS 9, but you can't set these in OS X...yes, that's correct; in OS X only 3 letter file extensions control file types, but you are not allowed to edit those assignments!

In Internet Explorer, you can set these by going to "Preferences" and the "File Helpers" pane. You will notice that "Untyped binary data" is set to "HexEdit", and that there is not a listing for ".dmg" extensions (you can click on the Extension word to sort by extensions). To fix this, simply create a new entry, describe it as "Disk Image", the the extension to ".dmg" and make sure you set it be a Macintosh file of binary (not text) persuasion.

Anyway, no one is more annoyed with this entire problem than me (Lane), because I get the blame and yet I have no control over the situation! However, there is a program that gives you back some of the control over this situation:
	http://www.rubicode.com/Software/RCDefaultApp/

It's FREE and it works for me :) Thanks to hardboiled for sending me the link!


* Translations *

Japanese translation by:	Hardboiled_egg <boiled@geocities.co.jp>
				<http://www.geocities.co.jp/SiliconValley/5025/>

French translation by:		Jean-Jacques Cortes <jjcortes@wanadoo.fr>
				<http://perso.wanadoo.fr/jjcortes/>

German translation by:		Bruce Gehre <bruce@gehre.org>
				<http://gehre.org/hexedit/>


* From Jim's original Docs *
HexEdit is a hexdump viewer and editor that works similarly to the hex editor provided with Apple's ResEdit.  It allows you to edit either the data fork or the resource fork of a file.

I wrote HexEdit because I needed to be able to insert/delete bytes from the data fork of files I was testing, and tools like FEdit don't have insert/deletion.

ΡΡΡΡΡΡΡΡ
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
    Recovery of damaged files, or at least portions of data therefrom.
	