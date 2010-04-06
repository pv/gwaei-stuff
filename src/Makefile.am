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
DEFS = -DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DBINDIR=\"$(bindir)\" -DRSYNC=\"$(RSYNC)\" -DUNZIP=\"$(UNZIP)\" -DGUNZIP=\"$(GUNZIP)\" -DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\"

gwaei_SOURCES = main.c dictinfo-object.c dictlist-object.c queryline-object.c engine.c utilities.c io.c regex.c console-main-interface.c ncurses-main-interface.c include/gwaei/console-main-interface.h include/gwaei/ncurses-main-interface.h searchitem-object.c historylist-object.c resultline-object.c include/gwaei/main.h include/gwaei/dictinfo-object.h include/gwaei/dictlist-object.h include/gwaei/utilities.h include/gwaei/io.h include/gwaei/regex.h include/gwaei/engine.h include/gwaei/definitions.h include/gwaei/preferences.h include/gwaei/resultline-object.h include/gwaei/historylist-object.h include/gwaei/searchitem-object.h include/gwaei/search-objects.h include/gwaei/dictionary-objects.h include/gwaei/queryline-object.h
gwaei_LDADD =  $(DEPS_LIBS) @LIBINTL@
gwaei_LDFLAGS = -Wall -Wextra -pedantic -lncursesw

AM_CFLAGS = $(DEPS_CFLAGS)
INCLUDES = -I $(top_srcdir)/src/include

##WINDOWS SETUP
if ENABLE_WIN32
gwaei_SOURCES += gtk-main-callbacks.c include/gwaei/gtk-main-callbacks.h gtk-main-interface.c gtk-settings-callbacks.c include/gwaei/gtk-settings-callbacks.h gtk-settings-interface.c gtk-radicals-interface.c gtk-radicals-callbacks.c include/gwaei/gtk-radicals-callbacks.h gtk-printing.c include/gwaei/gtk-main-interface.h include/gwaei/gtk-settings-interface.h include/gwaei/gtk-radicals-interface.h include/gwaei/gtk-printing.h include/gwaei/gtk.h gtk-kanjipad-callbacks.c include/gwaei/gtk-kanjipad-callbacks.h  gtk-kanjipad-interface.c include/gwaei/gtk-kanjipad-interface.h gtk-kanjipad-interface-drawingarea.c include/gwaei/gtk-kanjipad-interface-drawingarea.h gtk-main-interface-tabs.c include/gwaei/gtk-main-interface-tabs.h
gwaei_LDADD += $(WIN32_LIBS)
AM_CFLAGS += $(WIN32_CFLAGS)
DEFS += -DINTERFACE=\"GTK\" -DGW_WITH_GTK
DEFS += -DLC_MESSAGES=\"C\"
DEFS += -DDATADIR2=\"$(datadir)\"
DEFS += -DCURL_STATICLIB
##Libcurl stuffs for zlib, ssl and such
#gwaei_LDADD += -lws2_32 -lwinmm -leay32 -lssl32 -lssl -lssh2 -llibz -lcurl -lurl -lcrypt -lcrypt32 -lcrypto -lgnurx
gwaei_LDADD += -lws2_32 -lwinmm -lcurl -lurl -lgnurx -llibz
@ENABLE_WIN32_TRUE@am__append_1 = -LC_MESSAGE=\"C\"


##LINUX/UNIX/BSD SETUP
else
DEFS += -DDATADIR2=\"$(datadir)\"

if WITHOUT_GNOME
gwaei_SOURCES += none-preferences.c
DEFS += -DINTERFACE=\"NONE\"

else
gwaei_SOURCES += gtk-gconf-preferences.c 
gwaei_SOURCES += gtk-main-interface-sexy.c
gwaei_SOURCES += gtk-main-callbacks.c gtk-main-interface.c gtk-settings-callbacks.c gtk-settings-interface.c gtk-radicals-interface.c gtk-radicals-callbacks.c gtk-printing.c include/gwaei/gtk-main-interface.h include/gwaei/gtk-settings-interface.h include/gwaei/gtk-radicals-interface.h include/gwaei/gtk-printing.h include/gwaei/gtk.h include/gwaei/gtk-settings-interface-install-line.h gtk-kanjipad-callbacks.c include/gwaei/gtk-kanjipad-callbacks.h  gtk-kanjipad-interface.c include/gwaei/gtk-kanjipad-interface.h gtk-kanjipad-interface-candidatearea.c include/gwaei/gtk-kanjipad-interface-candidatearea.h gtk-kanjipad-interface-drawingarea.c include/gwaei/gtk-kanjipad-interface-drawingarea.h include/gwaei/gtk-main-interface-sexy.h gtk-main-interface-tabs.c include/gwaei/gtk-main-interface-tabs.h gtk-settings-interface-install-line.c
gwaei_LDADD += $(GNOME_LIBS)
AM_CFLAGS += $(GNOME_CFLAGS)
DEFS += -DINTERFACE=\"GTK\" -DGW_WITH_GTK

endif
endif
