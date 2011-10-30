
SUBDIRS = include/libwaei

LIBRARY_VERSION = 2:0:0
ACLOCAL_AMFLAGS = -I m4
datadir = @datadir@
localedir = $(datadir)/locale
DEFINITIONS = -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DUNZIP=\"$(UNZIP)\" -DGZIP=\"$(GZIP)\" -DLOCALEDIR=\"$(localedir)\" -DDATADIR2=\"$(datadir)\" -DGLIB_DISABLE_DEPRECATED -DG_SEAL_ENABLE -DGWAEI_LOCALEDIR=\"${GWAEI_LOCALEDIR}\" -Wall

lib_LTLIBRARIES = libwaei.la
libwaei_la_SOURCES = libwaei.c dictinfo.c dictinfolist.c dictinst.c dictinstlist.c queryline.c engine.c engine-data.c utilities.c io.c regex.c searchitem.c history.c resultline.c preferences.c
libwaei_la_LDFLAGS = -version-info $(LIBRARY_VERSION)  $(BASE_LIBS) -gall
libwaei_la_CPPFLAGS = $(BASE_CFLAGS) -I$(top_srcdir)/src/libwaei/include $(DEFINITIONS) -Wall

if OS_MINGW
libwaei_la_LDFLAGS += -Wl,-subsystem,windows 
MINGW_DEFS = -DLC_MESSAGES=LC_ALL -DCURL_STATICLIB 
libwaei_la_LDFLAGS += $(MINGW_LIBS) 
libwaei_la_CPPFLAGS += $(MINGW_CFLAGS) $(MINGW_DEFS)
endif


