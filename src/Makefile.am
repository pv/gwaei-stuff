## Process this file with automake to produce Makefile.in

PACKAGE = @PACKAGE@
VERSION = @VERSION@
RSYNC = @RSYNC@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gwaei

install-data-local: install-symbolic-link
uninstall-local: uninstall-symbolic-link

install-symbolic-link: install-binPROGRAMS
if ENABLE_WIN32
	rm -f $(DESTDIR)$(bindir)/waei.exe
	cp $(DESTDIR)$(bindir)/gwaei.exe $(DESTDIR)$(bindir)/waei.exe
else
	rm -f $(DESTDIR)$(bindir)/waei
	cp $(DESTDIR)$(bindir)/gwaei $(DESTDIR)$(bindir)/waei
endif
uninstall-symbolic-link:
if ENABLE_WIN32
	rm -f $(DESTDIR)$(bindir)/waei.exe
else
	rm -f $(DESTDIR)$(bindir)/waei
endif

datadir = @datadir@
localedir = $(datadir)/locale
DEPS_DEFS =-DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DBINDIR=\"$(bindir)\" -DRSYNC=\"$(RSYNC)\" -DUNZIP=\"$(UNZIP)\" -DGUNZIP=\"$(GUNZIP)\" -DDATADIR=\"$(datadir)\" -DLIBDIR=\"$(libdir)\"
#AM_CFLAGS = --pedantic -std=c99 -O2 -g


gwaei_SOURCES = main.c dictinfo-object.c dictlist-object.c queryline-object.c engine.c utilities.c io.c regex.c console-main-interface.c searchitem-object.c historylist-object.c resultline-object.c
gwaei_LDADD = $(DEPS_LIBS) @LIBINTL@
INCLUDES = -I$(top_srcdir)/src/include $(DEPS_CFLAGS) $(DEPS_DEFS)

##WINDOWS SETUP
if ENABLE_WIN32
#EMPTY
endif
##LINUX/UNIX/BSD SETUP
if WITH_GNOME
gwaei_SOURCES += gtk-gconf-preferences.c 
gwaei_SOURCES += gtk-main-callbacks.c gtk-main-interface.c gtk-settings-callbacks.c gtk-settings-interface.c gtk-radicals-interface.c gtk-radicals-callbacks.c gtk-printing.c gtk-kanjipad-callbacks.c  gtk-kanjipad-interface.c gtk-kanjipad-interface-candidatearea.c gtk-kanjipad-interface-drawingarea.c gtk-main-interface-tabs.c gtk-settings-interface-install-line.c
gwaei_LDADD += $(GNOME_LIBS)
GNOME_DEFS =-DWITH_GTK
INCLUDES += $(GNOME_CFLAGS) $(GNOME_DEFS)
else
gwaei_SOURCES += none-preferences.c
endif

if WITH_LIBUNIQUE
LIBUNIQUE_DEFS =-DWITH_LIBUNIQUE
gwaei_SOURCES += gtk-main-interface-unique.c
gwaei_LDADD += $(LIBUNIQUE_LIBS)
INCLUDES +=$(LIBUNIQUE_CFLAGS)$(LIBUNIQUE_DEFS)
endif

if WITH_LIBSEXY
LIBSEXY_DEFS =-DWITH_LIBSEXY
gwaei_SOURCES += gtk-main-interface-sexy.c
gwaei_LDADD += $(LIBSEXY_LIBS)
INCLUDES +=$(LIBSEXY_CFLAGS)$(LIBSEXY_DEFS)
endif

if WITH_NCURSES
NCURSES_DEFS =-DWITH_NCURSES
gwaei_SOURCES += ncurses-main-interface.c
gwaei_LDADD += $(NCURSES_LIBS)
INCLUDES +=$(NCURSES_DEFS)
endif

