This is SCANSREADER, a modification of qiv 1.8(*) to browse series of images
of scanned documents (aka scans).
Colas Nahaboo, http://colas.nahaboo.net, 24 June 2003
URL: http://colas.nahaboo.net/scansreader

The difference with QIV:

   * Automatic options: -w <screenwidth> -f -e -N -i -u .
   * Starts in upper left corner, image width zoomed to fill all screen width
   * Mouse wheel, and mouse motion scrolls up/down
   * Space does "the right thing": scrolls, then moves. If the image is zoomed
     to be wider than screen, sspace scrolls left-to-right then down
   * Much bigger scrolling and zooming steps
   * No more deletion/undeletion or other file modification functions
   * Mouse middle shows help & status
   * Numeric keys 1,2,3,..,9,0 go to a position of the image set,
     1 goes to first one, 0 to last one. Thus 3 goes to ~ 30%
     Status bar shows the key to press to get back there:
     e.g., pos 7+4 means: hit "7", then 4 times next image to get back there
   * Does not recurse into dir. Use -u to recurse (now -u do not take 
     an argument), in current dir.
   * no argument: read current dir, with arguments, if they are dirs,
     reads them, and if they are files, read their dirs
   * keep a log of read documents when exiting, that we can get back to
   * cannot modify files in any way: no risk of accidental image changes
   * indicator (small yellow corner) that we are at the end of scrolling
   * preloads next image for less loading time. 
     
SCANSREADER is meant to be used for browsing dirs of images in fullscreen mode.

It needs the debian packages: 
   * woody: imlib-dev imlib gdk-imlib-dev, 
   * sarge: imlib1-dev gdk-imlib1-dev
and the external programs "unzip" and "rar" to be able to open archives.

(*) qiv is at http://www.klografx.net/qiv/
I added the following patch from scansreader-1.9-pre4:
 11.09.02 [pre4]
   [ak] added patch by Geoff Richards <qef@rosies-dumplings.co.uk>
        great patch to reduce flicker while moving in fullscreen :-)
(this was the only scansreader-relevant enhancement of qiv in June 2003)
Quick Image Viewer (qiv) version 1.8  by Adam Kopacz / KLOGRAFX.DE Network
http://www.klografx.net/qiv/ - adam.k@klografx.de - Released on: 16.07.2002


I hacked (butchered would be a more appropriate term) this version quickly
into scansreader rather than adding options as I wanted to make a simple,
no-frills no-options, standalone program.

Log file format: ~/.scansreader.log: one line per record, records are made 
of 3 fields, tab-separated: directory image_name index


TODO:
   * azerty equivalents to 1->0, help keys depends on affectation
   * bookmarks (F1->F12 ?) with list on screen
   * truncate log after N entries (10 000?) 

Installation (Linux)
--------------------

 make
 make install

 Some compile-time options are in the Makefile; You may want to
 read over them before typing "make install"... but the default should be OK.

 if you get errors like this: "/bin/sh: imlib-config: command not
 found" then your Imlib is to old.. this mean you don't have the
 script imlib-config..  you should update Imlib (see "Links" below) or
 replace the `imlib-config` calls in the Makefile:

 `imlib-config --cflags-gdk` -->
 -I/usr/X11R6/include -I/usr/local/lib/glib/include -I/usr/local/include \
 -I/usr/local/include -I/usr/X11R6/include

 `imlib-config --libs-gdk` -->
 -L/usr/local/lib -lgdk_imlib -L/usr/local/lib -L/usr/X11R6/lib -lgtk-1.1 \
 -lgdk-1.1 -rdynamic -lgmodule-1.1 -lglib-1.1 -ldl -lXi -lXext -lX11 -lm \
 -ljpeg -ltiff -lgif -lpng -lz -lm

Other OSes:
----------

 It should work, but it is untested. You will need gdk + imlib + X.

Links:
------

 ImLib   : ftp://ftp.gnome.org/pub/GNOME/stable/sources/imlib/
 gtk/gdk : http://www.gtk.org/

