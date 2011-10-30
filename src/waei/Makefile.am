SUBDIRS = include/waei 

PACKAGE = @PACKAGE@
VERSION = @VERSION@
ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = waei

datadir = @datadir@
localedir = $(datadir)/locale
DEFINITIONS =-DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DBINDIR=\"$(bindir)\" -DUNZIP=\"$(UNZIP)\" -DGUNZIP=\"$(GUNZIP)\" -DDATADIR2=\"$(datadir)\" -DLIBDIR=\"$(libdir)\" -DGTK_DISABLE_DEPRECATED -DG_SEAL_ENABLE

waei_SOURCES = waei.c console.c console-callbacks.c
waei_LDADD =  $(BASE_LIBS) ../libwaei/libwaei.la
waei_CPPFLAGS = $(DEFINITIONS) $(BASE_CFLAGS) $(BASE_DEFS) -I$(top_srcdir)/src/libwaei/include -I$(top_srcdir)/src/waei/include -Wall


if OS_MINGW
MINGW_DEFS = -DLC_MESSAGES=LC_ALL -DCURL_STATICLIB 
waei_LDADD += $(MINGW_LIBS)
waei_CPPFLAGS += $(MINGW_CFLAGS) $(MINGW_DEFS)
endif


