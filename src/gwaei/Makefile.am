SUBDIRS = include/gwaei ui help

PACKAGE = @PACKAGE@
VERSION = @VERSION@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gwaei 

datadir = @datadir@
localedir = $(datadir)/locale
DEFINITIONS =-DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\" -DENCHANT=\"$(ENCHANT)\" -DGWAEI_LOCALEDIR=\"${GWAEI_LOCALEDIR}\" -DGTK_DISABLE_DEPRECATED -DG_SEAL_ENABLE

##gwaei_SOURCES = gwaei.c output-callbacks.c window.c search-callbacks.c search-window.c search-data.c dictinfolist.c dictinfolist-callbacks.c spellcheck.c spellcheck-callbacks.c settings-window.c settings-callbacks.c printing.c radicals-window.c radicals-callbacks.c kanjipad-window.c kanjipad-callbacks.c kanjipad-candidatearea.c kanjipad-drawingarea.c dictionaryinstall-window.c dictionaryinstall-callbacks.c installprogress-window.c installprogress-callbacks.c
gwaei_SOURCES = gwaei.c application.c application-callbacks.c window.c dictinfolist.c dictinfolist-callbacks.c searchwindow.c searchwindow-callbacks.c searchwindow-output.c search-data.c spellcheck.c spellcheck-callbacks.c printing.c radicalswindow.c radicalswindow-callbacks.c kanjipadwindow-callbacks.c kanjipad-drawingarea.c kanjipad-candidatearea.c  kanjipadwindow.c settingswindow.c settingswindow-callbacks.c dictionaryinstallwindow.c dictionaryinstallwindow-callbacks.c 

gwaei_LDADD =  $(GNOME_LIBS) ../libwaei/libwaei.la
gwaei_CPPFLAGS = $(DEFINITIONS) $(GNOME_CFLAGS) $(GNOME_DEFS) -I$(top_srcdir)/src/libwaei/include -I$(top_srcdir)/src/gwaei/include -Wall

if OS_MINGW
gwaei_LDFLAGS = -Wl,-subsystem,windows 
MINGW_DEFS = -DLC_MESSAGES=LC_ALL -DCURL_STATICLIB 
gwaei_LDADD += $(MINGW_LIBS) 
gwaei_CPPFLAGS += $(MINGW_CFLAGS) $(MINGW_DEFS)
endif

