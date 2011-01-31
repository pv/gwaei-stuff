
SUBDIRS = include/gwaei

LIBRARY_VERSION = 1:0:0
ACLOCAL_AMFLAGS = -I m4
datadir = @datadir@
localedir = $(datadir)/locale
DEFINITIONS = -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DUNZIP=\"$(UNZIP)\" -DGUNZIP=\"$(GUNZIP)\" -DLOCALEDIR=\"$(localedir)\" -DDATADIR2=\"$(datadir)\" -DGLIB_DISABLE_DEPRECATED -DG_SEAL_ENABLE

lib_LTLIBRARIES = libgwaei-backend.la
libgwaei_backend_la_SOURCES = backend.c dictinfo.c dictinfolist.c dictinst.c dictinstlist.c queryline.c engine.c utilities.c io.c regex.c searchitem.c historylist.c resultline.c preferences.c
libgwaei_backend_la_LDFLAGS = -version-info $(LIBRARY_VERSION)  $(BASE_LIBS)
libgwaei_backend_la_CPPFLAGS = $(BASE_CFLAGS) -I$(top_srcdir)/src/backend/include $(DEFINITIONS)

