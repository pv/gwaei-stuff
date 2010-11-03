## Process this file with automake to produce Makefile.in

PACKAGE = @PACKAGE@
VERSION = @VERSION@
BINDIR  = @bindir@
DATADIR2 = @datadir@

LIBDIR  = $(DATADIR2)/$(PACKAGE)
PERL  = @PERL@
KPGENDINEDIR = $(top_srcdir)/src/kpengine

libdir_PROGRAMS = kpengine
libdirdir = $(libdir)/$(PACKAGE)
pkgdata_DATA = jdata.dat

DEFS = -DLOCALEDIR=\"$(localedir)\" -DVERSION=\"$(VERSION)\" -DPACKAGE=\"$(PACKAGE)\" -DFOR_PILOT_COMPAT -DKP_LIBDIR=\"$(LIBDIR)\" -DBINDIR=\"$(LIBDIR)\"
if ENABLE_WIN32
endif


AM_CFLAGS = $(DEPS_CFLAGS) 
INCLUDES =  -I jstroke


kpengine_SOURCES = jstroke/scoring.c jstroke/util.c kpengine.c jstroke/jstroke.h
kpengine_LDADD =  $(DEPS_LIBS) @LIBINTL@
kpengine_LDFLAGS = -Wall


if ENABLE_WIN32
WIN32_LIB = -L$(top_srcdir)/windows/lib
WIN32_INCLUDE = -I$(top_srcdir)/windows/include -I$(top_srcdir)/windows/include/glib-2.0 -I$(top_srcdir)/windows/lib/glib-2.0/include -I$(top_srcdir)/windows/include/gtk-2.0 -I$(top_srcdir)/windows/include/cairo -I$(top_srcdir)/windows/include/pango-1.0 -I$(top_srcdir)/windows/lib/gtk-2.0/include -I$(top_srcdir)/windows/include/gdk-pixbuf-2.0 -I$(top_srcdir)/windows/include/atk-1.0

WIN32_CURL_LIB = -lcurl -lws2_32
WIN32_REGEX_LIB = -lregex
WIN32_GETTEXT_LIB =  -lintl -lgettextlib
WIN32_CAIRO_LIB =  -lcairo
WIN32_PANGO_LIB =  -lpango-1.0 -lpangocairo-1.0
WIN32_GLIB_LIB = -lglib-2.0 -lgobject-2.0 -lgmodule-2.0 -lgthread-2.0 -lgio-2.0
WIN32_GTK_LIB = -lgtk-win32-2.0 -lgdk-win32-2.0
WIN32_GDK_PIXBUF_LIB = -lgdk_pixbuf-2.0
WIN32_ATK_LIB = -latk-1.0
WIN32_LIBS = $(WIN32_LIB) $(WIN32_REGEX_LIB) $(WIN32_CURL_LIB) $(WIN32_GLIB_LIB) $(WIN32_GETTEXT_LIB) $(WIN32_GTK_LIB) $(WIN32_CAIRO_LIB) $(WIN32_PANGO_LIB) $(WIN32_GDK_PIXBUF_LIB) $(WIN32_ATK_LIB)

INCLUDES += $(WIN32_INCLUDE)
kpengine_LDADD += $(WIN32_LIBS)
endif


## Some special rules are needed for jdata.dat installation
clean-local: clean-jdata
clean-jdata:
	rm -f jdata.dat
jdata.dat: jstroke/strokedata.h conv_jdata.pl
	$(PERL) $(KPGENDINEDIR)/conv_jdata.pl < $(KPGENDINEDIR)/jstroke/strokedata.h > jdata.dat
uninstall-local:
	@ rm -rf $(libdirdir)

EXTRA_DIST = conv_jdata.pl jstroke/jstroke.h jstroke/memowrite.h jstroke/strokedata.h jstroke/jstrokerc.h jstroke/pilotcompat.h
