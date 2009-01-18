######################################################################
# Makefile for scansreader
# User Options
######################################################################

# Directory where scansreader will be installed under.
PREFIX = /usr/local

# Font to use for statusbar in fullscreen mode
STATUSBAR_FONT = "-misc-fixed-*-*-normal-*-20-*-*-*-*-*-*-*"

# Cursor to use on scansreader windows - see
# /usr/X11R6/include/X11/cursorfont.h for more choices.
CURSOR = 84

# This sets the file extentions to filter on (other file types will be
# skipped.) It should reflect whatever is compiled into imlib.
EXTNS = GIF TIFF XPM XBM PNG PPM PNM PGM PCX BMP EIM JPEG TGA

# Comment this line out if your system doesn't have getopt_long().
GETOPT_LONG = -DHAVE_GETOPT_LONG

######################################################################

# The following only apply to 'make install-xscreensaver':
# Delay in minutes to start xscreensaver
SS_TIMEOUT = 5

# Delay in minutes to cycle between xscreensaver programs
SS_CYCLE = 5

# Delay in seconds to wait between images
SS_DELAY = 5

# Image files to display
SS_IMAGES = ~/pictures/*.jpg

# Comment out this line to have pictures be dislayed sequentially
SS_RANDOMIZE = -r

######################################################################
# Variables and Rules
# Do not edit below here!
######################################################################

CC        = gcc
CFLAGS    = -O2 -Wall -fomit-frame-pointer -finline-functions \
	    -fcaller-saves -ffast-math -fno-strength-reduce \
	    -fthread-jumps #-march=pentium #-DSTAT_MACROS_BROKEN

INCLUDES  = `imlib-config --cflags-gdk`
LIBS      = `imlib-config --libs-gdk`

PROGRAM   = scansreader
OBJS      = main.o image.o event.o options.o utils.o help.o
HEADERS   = scansreader.h
DEFINES   = $(patsubst %,-DEXTN_%, $(EXTNS)) \
            $(GETOPT_LONG) \
            -DSTATUSBAR_FONT='$(STATUSBAR_FONT)' \
            -DCURSOR=$(CURSOR)

ifndef GETOPT_LONG
OBJS     += getopt.o getopt1.o
OBJS_G   += getopt.g getopt1.g
endif

PROGRAM_G = scansreader-g
OBJS_G    = $(OBJS:.o=.g)
DEFINES_G = $(DEFINES) -DDEBUG

SS_PROG   = $(PREFIX)/ss-scansreader

######################################################################

all: $(PROGRAM) $(PROGRAM)-static

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM) $(LIBS)

$(PROGRAM)-static: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(PROGRAM)-static -static $(LIBS)

$(OBJS): %.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $(DEFINES) $(INCLUDES) $< -o $@

main.o: main.h

######################################################################

debug: $(PROGRAM_G)

$(PROGRAM_G): $(OBJS_G)
	$(CC) -g $(CFLAGS) $(DEFINES_G) $(LIBS) $(OBJS_G) -o $(PROGRAM_G)

$(OBJS_G): %.g: %.c $(HEADERS)
	$(CC) -c -g $(CFLAGS) $(DEFINES_G) $(INCLUDES) $< -o $@

######################################################################

clean :  
	@echo "Cleaning up..."
	rm -f $(PROGRAM) $(PROGRAM_G) $(PROGRAM)-static $(OBJS) $(OBJS_G)

install: $(PROGRAM)
	@echo "Installing..."
	install -s -m 0755 $(PROGRAM) $(PREFIX)/bin
	install -m 0644 $(PROGRAM).1 $(PREFIX)/man/man1

#############################################################################
# development tools, for my own use, you will not need this

ci:
	fshar < FILES > scansreader.shar
	ci -l scansreader.shar </dev/null


DISTDIR=/home/colas/www/lo/web-colas.nahaboo.net/software/scansreader
dist:	all
	rm -f *.o *-g *.g
	tar cfz $(DISTDIR)/files/scansreader-`grep VERSION scansreader.h| sed -e 's/[^"]*"\([^"]*\)".*/\1/'`.tgz `cat FILES` $(PROGRAM) $(PROGRAM)-static
	cp HISTORY.txt README.txt scansreader.html $(DISTDIR)
	strip-mans scansreader.1 >$(DISTDIR)/scansreader-manual.txt

# the end... ;-)
