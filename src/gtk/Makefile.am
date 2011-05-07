SUBDIRS = include/gwaei ui images desktop help

PACKAGE = @PACKAGE@
VERSION = @VERSION@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = gwaei 

datadir = @datadir@
localedir = $(datadir)/locale
DEFINITIONS =-DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\" -DENCHANT=\"$(ENCHANT)\"

gwaei_SOURCES = gwaei.c common.c frontend.c main.c main-callbacks.c settings-callbacks.c settings.c radsearchtool.c radsearchtool-callbacks.c printing.c main-tabs.c dictionaryinstall.c installprogress.c dictionarymanager.c settings-listeners.c 
gwaei_LDADD =  $(GNOME_LIBS) ../backend/libgwaei-backend.la
gwaei_CPPFLAGS = $(DEFINITIONS) $(GNOME_CFLAGS) $(GNOME_DEFS) -I$(top_srcdir)/src/backend/include -I$(top_srcdir)/src/gtk/include 

if ENABLE_WIN32
gwaei_LDFLAGS = -Wl,-subsystem,windows 
WIN_PREFIX = /mingw/usr
WIN32_DEFS = -DENABLE_WIN32 -DLC_MESSAGES=\"C\" -DCURL_STATICLIB 
gwaei_LDADD += $(WIN32_LIBS) 
gwaei_CPPFLAGS += $(WIN32_CFLAGS) $(WIN32_DEFS) -I$(WIN_PREFIX)/include
endif

if WITH_LIBUNIQUE
LIBUNIQUE_DEFS =-DWITH_LIBUNIQUE
gwaei_SOURCES += libunique.c
gwaei_LDADD += $(LIBUNIQUE_LIBS)
gwaei_CPPFLAGS +=$(LIBUNIQUE_CFLAGS)$(LIBUNIQUE_DEFS)
endif


