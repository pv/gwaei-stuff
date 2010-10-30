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
DEPS_DEFS =-DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DBINDIR=\"$(bindir)\" -DRSYNC=\"$(RSYNC)\" -DUNZIP=\"$(UNZIP)\" -DGUNZIP=\"$(GUNZIP)\" -DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\"
#AM_CFLAGS = --pedantic -std=c99 -O2 -g


gwaei_SOURCES = main.c dictinfo-object.c dictlist-object.c queryline-object.c engine.c utilities.c io.c regex.c console-main-interface.c searchitem-object.c historylist-object.c resultline-object.c
#gwaei_LDADD = $(DEPS_LIBS) @LIBINTL@
#INCLUDES = -I$(top_srcdir)/src/include $(DEPS_CFLAGS) $(DEPS_DEFS)
gwaei_LDADD = 
INCLUDES = -I$(top_srcdir)/src/include $(DEPS_DEFS)

##WINDOWS SETUP
if ENABLE_WIN32
WIN32_CURL_INCLUDE = -I$(top_srcdir)/windows/curl/include
WIN32_CURL_LIB = -L$(top_srcdir)/windows/curl/lib -lcurl -lws2_32

WIN32_REGEX_INCLUDE = -I$(top_srcdir)/windows/regex/include
WIN32_REGEX_LIB = -L$(top_srcdir)/windows/regex/lib -lregex

WIN32_GETTEXT_INCLUDE = -I$(top_srcdir)/windows/gettext/include
WIN32_GETTEXT_LIB =  -L$(top_srcdir)/windows/gettext/lib -lintl -lgettextlib

WIN32_CAIRO_INCLUDE = -I$(top_srcdir)/windows/cairo/include/cairo
WIN32_CAIRO_LIB =  -L$(top_srcdir)/windows/cairo/lib -lcairo

WIN32_PANGO_INCLUDE = -I$(top_srcdir)/windows/pango/include/pango-1.0
WIN32_PANGO_LIB =  -L$(top_srcdir)/windows/pango/lib -lpango-1.0 -lpangocairo-1.0

WIN32_GLIB_INCLUDE = -I$(top_srcdir)/windows/glib/include -I$(top_srcdir)/windows/glib/include/glib-2.0 -I$(top_srcdir)/windows/glib/lib/glib-2.0/include
WIN32_GLIB_LIB = -L$(top_srcdir)/windows/glib/lib -lglib-2.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lgio-2.0
WIN32_GTK_INCLUDE = -I$(top_srcdir)/windows/gtk/include/gtk-2.0 -I$(top_srcdir)/windows/gtk/lib/gtk-2.0/include
WIN32_GTK_LIB = -L$(top_srcdir)/windows/gtk/lib -lgtk-win32-2.0 -lgdk-win32-2.0

WIN32_GDK_PIXBUF_INCLUDE = -I$(top_srcdir)/windows/gdk-pixbuf/include/gdk-pixbuf-2.0
WIN32_GDK_PIXBUF_LIB = -L$(top_srcdir)/windows/gdk-pixbuf/lib -lgdk_pixbuf-2.0

WIN32_ATK_INCLUDE = -I$(top_srcdir)/windows/atk/include/atk-1.0
WIN32_ATK_LIB = -L$(top_srcdir)/windows/atk/lib -latk-1.0

WIN32_DEFS = -DWITH_GTK -DENABLE_WIN32 -DLC_MESSAGES=\"C\" -DCURL_STATICLIB $(WIN32_CURL_INCLUDE) $(WIN32_GLIB_INCLUDE) $(WIN32_GTK_INCLUDE) -I/usr/i486-mingw32/include $(WIN32_CAIRO_INCLUDE) $(WIN32_PANGO_INCLUDE) $(WIN32_GDK_PIXBUF_INCLUDE) $(WIN32_ATK_INCLUDE)
WIN32_LIBS = $(WIN32_REGEX_LIB) $(WIN32_CURL_LIB) $(WIN32_GLIB_LIB) $(WIN32_GETTEXT_LIB) $(WIN32_GTK_LIB) -L$(top_srcdir)/windows/rt/lib $(WIN32_CAIRO_LIB) $(WIN32_PANGO_LIB) $(WIN32_GDK_PIXBUF_LIB) $(WIN32_ATK_LIB)

INCLUDES += $(WIN32_REGEX_INCLUDE) $(WIN32_GETTEXT_INCLUDE) $(WIN32_DEFS)
gwaei_LDADD += $(WIN32_LIBS)
gwaei_SOURCES += none-preferences.c
gwaei_SOURCES += gtk-main-callbacks.c gtk-main-interface.c gtk-settings-callbacks.c gtk-settings-interface.c gtk-radicals-interface.c gtk-radicals-callbacks.c gtk-printing.c gtk-kanjipad-callbacks.c  gtk-kanjipad-interface.c gtk-kanjipad-interface-candidatearea.c gtk-kanjipad-interface-drawingarea.c gtk-main-interface-tabs.c gtk-settings-interface-install-line.c
else
gwaei_LDADD += $(DEPS_LIBS) @LIBINTL@
INCLUDES += $(DEPS_CFLAGS)

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

EXTRA_DIST = include/gwaei/console-main-interface.h include/gwaei/definitions.h include/gwaei/definitions.h.in include/gwaei/dictinfo-object.h include/gwaei/dictionary-objects.h include/gwaei/dictlist-object.h include/gwaei/engine.h include/gwaei/gettext.h include/gwaei/gtk.h include/gwaei/gtk-kanjipad-callbacks.h include/gwaei/gtk-kanjipad-interface-candidatearea.h include/gwaei/gtk-kanjipad-interface-drawingarea.h include/gwaei/gtk-kanjipad-interface.h include/gwaei/gtk-main-callbacks.h include/gwaei/gtk-main-interface.h include/gwaei/gtk-main-interface-sexy.h include/gwaei/gtk-main-interface-tabs.h include/gwaei/gtk-main-interface-unique.h include/gwaei/gtk-printing.h include/gwaei/gtk-radicals-callbacks.h include/gwaei/gtk-radicals-interface.h include/gwaei/gtk-settings-callbacks.h include/gwaei/gtk-settings-interface.h include/gwaei/gtk-settings-interface-install-line.h include/gwaei/historylist-object.h include/gwaei/io.h include/gwaei/main.h include/gwaei/ncurses-main-interface.h include/gwaei/preferences.h include/gwaei/queryline-object.h include/gwaei/regex.h include/gwaei/resultline-object.h include/gwaei/searchitem-object.h include/gwaei/search-objects.h include/gwaei/utilities.h
