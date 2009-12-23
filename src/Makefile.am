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

gwaei_SOURCES = main.c dictinfo-object.c dictlist-object.c queryline-object.c engine.c utilities.c io.c regex.c main-interface-console.c searchitem-object.c historylist-object.c resultline-object.c include/gwaei/main.h include/gwaei/dictinfo-object.h include/gwaei/dictlist-object.h include/gwaei/utilities.h include/gwaei/io.h include/gwaei/regex.h include/gwaei/engine.h include/gwaei/definitions.h include/gwaei/preferences.h include/gwaei/resultline-object.h include/gwaei/historylist-object.h include/gwaei/searchitem-object.h include/gwaei/search-objects.h include/gwaei/dictionary-objects.h include/gwaei/queryline-object.h
gwaei_LDADD =  $(DEPS_LIBS) @LIBINTL@
gwaei_LDFLAGS = -Wall -Wextra -pedantic -lncursesw

AM_CFLAGS = $(DEPS_CFLAGS)
INCLUDES = -I $(top_srcdir)/src/include

##WINDOWS SETUP
if ENABLE_WIN32
gwaei_SOURCES += main-callbacks-gtk.c main-interface-gtk.c settings-callbacks-gtk.c settings-interface-gtk.c radicals-interface-gtk.c radicals-callbacks-gtk.c printing-gtk.c include/gwaei/interface.h include/gwaei/callbacks.h include/gwaei/radicals.h include/gwaei/settings.h include/gwaei/printing.h include/gwaei/gtk.h kanjipad/callbacks.c kanjipad/callbacks.h  kanjipad/kanjipad.c kanjipad/kanjipad.h kanjipad/padarea.c kanjipad/padarea.h tabs.c include/gwaei/tabs.h
gwaei_SOURCES += sexy-disable.c
gwaei_SOURCES += preferences-none.c
gwaei_LDADD += $(WIN32_LIBS)
AM_CFLAGS += $(WIN32_CFLAGS)
DEFS += -DINTERFACE=\"GTK\"
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
gwaei_SOURCES += preferences-none.c
gwaei_SOURCES += include/gwaei/interface.h include/gwaei/callbacks.h main-interface-none.c sexy-disable.c
DEFS += -DINTERFACE=\"NONE\"

else
gwaei_SOURCES += preferences-gconf.c 
gwaei_SOURCES += sexy-enable.c
gwaei_SOURCES += main-callbacks-gtk.c main-interface-gtk.c settings-callbacks-gtk.c settings-interface-gtk.c radicals-interface-gtk.c radicals-callbacks-gtk.c printing-gtk.c include/gwaei/interface.h include/gwaei/callbacks.h include/gwaei/radicals.h include/gwaei/settings.h include/gwaei/printing.h include/gwaei/gtk.h kanjipad/callbacks.c kanjipad/callbacks.h  kanjipad/kanjipad.c kanjipad/kanjipad.h kanjipad/padarea.c kanjipad/padarea.h include/gwaei/sexy.h tabs.c include/gwaei/tabs.h
gwaei_LDADD += $(GNOME_LIBS)
AM_CFLAGS += $(GNOME_CFLAGS)
DEFS += -DINTERFACE=\"GTK\"

##if DISABLE_SEXY
##gwaei_SOURCES += sexy-disable.c
##else
##gwaei_SOURCES += sexy-enable.c include/gwaei/sexy.h
##gwaei_LDADD += $(SEXY_LIBS)
##AM_CFLAGS += $(SEXY_CFLAGS)
##endif
endif
endif
