SUBDIRS = include/gwaei ui help

PACKAGE = @PACKAGE@
VERSION = @VERSION@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gwaei 

datadir = @datadir@
localedir = $(datadir)/locale
DEFINITIONS =-DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\" -DENCHANT=\"$(ENCHANT)\" -DGWAEI_LOCALEDIR=\"${GWAEI_LOCALEDIR}\" -DGTK_DISABLE_DEPRECATED -DG_SEAL_ENABLE

#gwaei_SOURCES = dictionaryinstall-callbacks.c dictionaryinstall-window.c dictinfolist.c gwaei.c installprogress-callbacks.c installprogress-window.c kanjipad-callbacks.c kanjipad-candidatearea.c kanjipad-drawingarea.c kanjipad-window.c output-callbacks.c pluginmanager.c printing.c radicals-callbacks.c radicals-window.c search-callbacks.c search-tabs.c search-window.c settings-callbacks.c settings-listeners.c settings-window.c spellcheck.c
gwaei_SOURCES = gwaei.c common.c output-callbacks.c window.c search-callbacks.c search-window.c search-data.c dictinfolist.c dictinfolist-callbacks.c spellcheck.c spellcheck-callbacks.c settings-window.c settings-callbacks.c printing.c

gwaei_LDADD =  $(GNOME_LIBS) ../libwaei/libwaei.la
gwaei_CPPFLAGS = $(DEFINITIONS) $(GNOME_CFLAGS) $(GNOME_DEFS) -I$(top_srcdir)/src/libwaei/include -I$(top_srcdir)/src/gwaei/include 

if ENABLE_WIN32
gwaei_LDFLAGS = -Wl,-subsystem,windows 
WIN32_DEFS = -DENABLE_WIN32 -DLC_MESSAGES=LC_ALL -DCURL_STATICLIB 
gwaei_LDADD += $(WIN32_LIBS) 
gwaei_CPPFLAGS += $(WIN32_CFLAGS) $(WIN32_DEFS)
endif

if WITH_LIBUNIQUE
LIBUNIQUE_DEFS =-DWITH_LIBUNIQUE
gwaei_SOURCES += libunique.c
gwaei_LDADD += $(LIBUNIQUE_LIBS)
gwaei_CPPFLAGS +=$(LIBUNIQUE_CFLAGS)$(LIBUNIQUE_DEFS)
endif


