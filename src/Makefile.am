## Process this file with automake to produce Makefile.in

PACKAGE = @PACKAGE@
VERSION = @VERSION@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gwaei

install-data-local: install-symbolic-link
uninstall-local: uninstall-symbolic-link

install-symbolic-link: install-binPROGRAMS
if ENABLE_WIN32
	rm -f $(DESTDIR)$(bindir)/waei.exe
else
	rm -f $(DESTDIR)$(bindir)/waei
	cp $(DESTDIR)$(bindir)/gwaei $(DESTDIR)$(bindir)/waei
endif
uninstall-symbolic-link:
if ENABLE_WIN32
else
	rm -f $(DESTDIR)$(bindir)/waei
endif

datadir = @datadir@
localedir = $(datadir)/locale
DEFINITIONS =-DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DBINDIR=\"$(bindir)\" -DUNZIP=\"$(UNZIP)\" -DGUNZIP=\"$(GUNZIP)\" -DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\" -DGTK_DISABLE_DEPRECATED -DG_SEAL_ENABLE
#AM_CFLAGS = --pedantic -std=c99 -O2 -g


gwaei_SOURCES = main.c dictinfo-object.c dictlist-object.c queryline-object.c engine.c utilities.c io.c regex.c console-main-interface.c searchitem-object.c historylist-object.c resultline-object.c gsettings-preferences.c
gwaei_LDADD =  @LIBINTL@
INCLUDES = -I$(top_srcdir)/src/include  $(DEFINITIONS)

##WINDOWS SETUP
if ENABLE_WIN32
gwaei_LDFLAGS = -Wl,-subsystem,windows 

WIN_PREFIX = /mingw/usr
WIN32_REGEX_LIB = -L$(WIN_PREFIX)/lib -lregex
WIN32_DEFS = -DWITH_GTK -DENABLE_WIN32 -DLC_MESSAGES=\"C\" -DCURL_STATICLIB 
gwaei_LDADD += $(WIN32_LIBS)  $(WIN32_REGEX_LIB)
INCLUDES += $(WIN32_CFLAGS) $(WIN32_DEFS) -I$(WIN_PREFIX)/include
endif

##LINUX/UNIX/BSD SETUP
if WITH_GNOME
gwaei_SOURCES += gtk-gsettings-preferences.c 
gwaei_SOURCES += gtk-main-callbacks.c gtk-main-interface.c gtk-settings-callbacks.c gtk-settings-interface.c gtk-radicals-interface.c gtk-radicals-callbacks.c gtk-printing.c gtk-kanjipad-callbacks.c  gtk-kanjipad-interface.c gtk-kanjipad-interface-candidatearea.c gtk-kanjipad-interface-drawingarea.c gtk-main-interface-tabs.c gtk-dictionary-install-dialog.c gtk-settings-dictionary-manager.c
gwaei_LDADD += $(GNOME_LIBS) 
GNOME_DEFS =-DWITH_GTK
INCLUDES += $(GNOME_CFLAGS) $(GNOME_DEFS)

##Setup with no GUI, base dependancies
else
gwaei_LDADD += $(BASE_LIBS) 
INCLUDES += $(BASE_CFLAGS)
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

EXTRA_DIST = include/gwaei/console-main-interface.h include/gwaei/definitions.h include/gwaei/definitions.h.in include/gwaei/dictinfo-object.h include/gwaei/dictionary-objects.h include/gwaei/dictlist-object.h include/gwaei/engine.h include/gwaei/gettext.h include/gwaei/gtk.h include/gwaei/gtk-kanjipad-callbacks.h include/gwaei/gtk-kanjipad-interface-candidatearea.h include/gwaei/gtk-kanjipad-interface-drawingarea.h include/gwaei/gtk-kanjipad-interface.h include/gwaei/gtk-main-callbacks.h include/gwaei/gtk-main-interface.h include/gwaei/gtk-main-interface-sexy.h include/gwaei/gtk-main-interface-tabs.h include/gwaei/gtk-main-interface-unique.h include/gwaei/gtk-printing.h include/gwaei/gtk-radicals-callbacks.h include/gwaei/gtk-radicals-interface.h include/gwaei/gtk-settings-callbacks.h include/gwaei/gtk-settings-interface.h include/gwaei/historylist-object.h include/gwaei/io.h include/gwaei/main.h include/gwaei/ncurses-main-interface.h include/gwaei/preferences.h include/gwaei/queryline-object.h include/gwaei/regex.h include/gwaei/resultline-object.h include/gwaei/searchitem-object.h include/gwaei/search-objects.h include/gwaei/utilities.h include/gwaei/gsettings-preferences.h include/gwaei/gtk-dictionary-install-dialog.h include/gwaei/gtk-settings-dictionary-manager.h
