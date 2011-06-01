
SUBDIRS = include/gwaei

LIBRARY_VERSION = 1:0:0
ACLOCAL_AMFLAGS = -I m4
datadir = @datadir@
localedir = $(datadir)/locale
DEFINITIONS = -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DUNZIP=\"$(UNZIP)\" -DGZIP=\"$(GZIP)\" -DLOCALEDIR=\"$(localedir)\" -DDATADIR2=\"$(datadir)\" -DGLIB_DISABLE_DEPRECATED -DG_SEAL_ENABLE -DGWAEI_LOCALEDIR=\"${GWAEI_LOCALEDIR}\"

lib_LTLIBRARIES = libwaei.la
libwaei_la_SOURCES = backend.c dictinfo.c dictinfolist.c dictinst.c dictinstlist.c queryline.c engine.c utilities.c io.c regex.c searchitem.c historylist.c resultline.c preferences.c
libwaei_la_LDFLAGS = -version-info $(LIBRARY_VERSION)  $(BASE_LIBS)
libwaei_la_CPPFLAGS = $(BASE_CFLAGS) -I$(top_srcdir)/src/backend/include $(DEFINITIONS)

