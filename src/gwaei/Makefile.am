SUBDIRS = include/gwaei ui help

PACKAGE = @PACKAGE@
VERSION = @VERSION@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gwaei 

datadir = @datadir@
localedir = $(datadir)/locale
DEFINITIONS =-DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\" -DENCHANT=\"$(ENCHANT)\" -DGWAEI_LOCALEDIR=\"${GWAEI_LOCALEDIR}\" -DGTK_DISABLE_DEPRECATED -DG_SEAL_ENABLE

##gwaei_SOURCES = gwaei.c output-callbacks.c window.c search-callbacks.c search-window.c search-data.c dictinfolist.c dictinfolist-callbacks.c spellcheck.c spellcheck-callbacks.c settings-window.c settings-callbacks.c printing.c radicals-window.c radicals-callbacks.c kanjipad-window.c kanjipad-callbacks.c kanjipad-candidatearea.c kanjipad-drawingarea.c dictionaryinstall-window.c dictionaryinstall-callbacks.c installprogress-window.c installprogress-callbacks.c
gwaei_SOURCES = gwaei.c application.c application-private.c application-callbacks.c output-callbacks.c output-popup-callbacks.c window.c window-private.c dictinfolist.c dictinfolist-callbacks.c search.c search-callbacks.c search-private.c search-data.c spellcheck.c spellcheck-callbacks.c printing.c radicals-window.c radicals-callbacks.c radicals-private.c kanjipad-callbacks.c kanjipad-drawingarea.c kanjipad-window.c kanjipad-candidatearea.c  kanjipad-private.c

gwaei_LDADD =  $(GNOME_LIBS) ../libwaei/libwaei.la
gwaei_CPPFLAGS = $(DEFINITIONS) $(GNOME_CFLAGS) $(GNOME_DEFS) -I$(top_srcdir)/src/libwaei/include -I$(top_srcdir)/src/gwaei/include -Wall

if OS_MINGW
gwaei_LDFLAGS = -Wl,-subsystem,windows 
MINGW_DEFS = -DLC_MESSAGES=LC_ALL -DCURL_STATICLIB 
gwaei_LDADD += $(MINGW_LIBS) 
gwaei_CPPFLAGS += $(MINGW_CFLAGS) $(MINGW_DEFS)
endif

