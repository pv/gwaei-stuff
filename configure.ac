AC_INIT(gwaei, 1.1Dev)
AC_CONFIG_SRCDIR([src/main.c])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(config.h)


RELEASE=1
AC_SUBST(RELEASE)
ARCH="i386"
AC_SUBST(ARCH)

GTK2_REQUIRED_VERSION=2.14.0
GLIB_REQUIRED_VERSION=2.8.0
GTHREAD_REQUIRED_VERSION=2.16.5
GCONF_REQUIRED_VERSION=2.22.0
LIBCURL_REQUIRED_VERSION=7.18.0
GMODULE_EXPORT_REQUIRED_VERSION=2.16.0
LIBSEXY_REQUIRED_VERSION=0.1.11
GDU_REQUIRED_VERSION=0.13.0

PKG_CHECK_MODULES(DEPS, glib-2.0           >= $GLIB_REQUIRED_VERSION
                        libcurl            >= $LIBCURL_REQUIRED_VERSION
                        gmodule-2.0        >= $GMODULE_EXPORT_REQUIRED_VERSION 
                        gthread-2.0        >= $GTHREAD_REQUIRED_VERSION       )
AC_SUBST(DEPS_CFLAGS)
AC_SUBST(DEPS_LIBS)


PKG_CHECK_MODULES(WIN32, gtk+-2.0           >= $GTK2_REQUIRED_VERSION )
AC_SUBST(WIN32_CFLAGS)
AC_SUBST(WIN32_LIBS)



PKG_CHECK_MODULES(GNOME, gtk+-2.0           >= $GTK2_REQUIRED_VERSION
                         gconf-2.0          >= $GCONF_REQUIRED_VERSION
                         libsexy            >= $LIBSEXY_REQUIRED_VERSION
                         gnome-doc-utils    >= $GDU_REQUIRED_VERSION
                          ,,AM_CONDITIONAL([WITHOUT_GNOME], true))
AC_SUBST(GNOME_CFLAGS)
AC_SUBST(GNOME_LIBS)


ENGLISH_DEFAULT_URI=ftp://ftp.monash.edu.au/pub/nihongo/edict.gz
AC_SUBST(ENGLISH_DEFAULT_URI)

KANJI_DEFAULT_URI=ftp://ftp.monash.edu.au/pub/nihongo/kanjidic.gz
AC_SUBST(KANJI_DEFAULT_URI)

RADICALS_DEFAULT_URI=ftp://ftp.monash.edu.au/pub/nihongo/kradfile.gz
AC_SUBST(RADICALS_DEFAULT_URI)

NAMES_DEFAULT_URI=ftp://ftp.monash.edu.au/pub/nihongo/enamdict.gz
AC_SUBST(NAMES_DEFAULT_URI)


MATCH_FG_DEFAULT=#000000
AC_SUBST(MATCH_FG_DEFAULT)

MATCH_BG_DEFAULT=#CCEECC
AC_SUBST(MATCH_BG_DEFAULT)

HEADER_FG_DEFAULT=#EE1111
AC_SUBST(HEADER_FG_DEFAULT)

HEADER_BG_DEFAULT=#FFDEDE
AC_SUBST(HEADER_BG_DEFAULT)

COMMENT_FG_DEFAULT=#2222DD
AC_SUBST(COMMENT_FG_DEFAULT)



AC_ARG_ENABLE([win32],
[  --enable-win32         turn on win32 (do not touch: experimental)],
[case "${enableval}" in
yes) win32=true ;;
no)  win32=false ;;
*) AC_MSG_ERROR([bad value ${enableval} for --enable-win32]) ;;
esac],[win32=false])
AM_CONDITIONAL([ENABLE_WIN32], [test x$win32 = xtrue])


AC_ARG_WITH([gnome],
[  --without-gnome         turn off the gnome gui interface],
[case "${withval}" in
yes) gnome=false ;;
no)  gnome=true ;;
*) AC_MSG_ERROR([bad value ${withval} for --without-gnome]) ;;
esac],[gnome=false])
AM_CONDITIONAL([WITHOUT_GNOME], [test x$gnome = xtrue])


AC_CONFIG_MACRO_DIR([m4])
GNOME_DOC_INIT


AM_GCONF_SOURCE_2
AC_PATH_PROG(GCONFTOOL, gconftool-2)
AC_PATH_PROG(PERL, perl)
AC_PATH_PROG(KILLALL, killall)
AC_PATH_PROG(RPMDIR, rpm)
AC_PATH_PROG(RPMBUILD, rpmbuild)
AC_PATH_PROG(DPKG_DEB, dpkg-deb)
AC_PATH_PROG(RSYNC, rsync)
AC_PATH_PROG(GUNZIP, gunzip)

AC_PROG_CC
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.17)

if([test x$gnome = xfalse]);then([cp help/Makefile.am.gnome help/Makefile.am]);fi
if([test x$gnome = xtrue]);then([cp help/Makefile.am.none help/Makefile.am]);fi

AC_CONFIG_FILES([Makefile src/Makefile help/Makefile help/gwaei.omf help/C/gwaei.xml src/xml/Makefile src/img/Makefile src/schemas/Makefile src/desktop/Makefile src/kpengine/Makefile src/kpengine/conv_jdata.pl deb/DEBIAN/control deb/DEBIAN/postinst deb/DEBIAN/prerm rpm/gwaei.spec po/Makefile.in rpm/rpm_gconf_install_post.sh rpm/rpm_gconf_uninstall_pre.sh src/schemas/gwaei.schemas src/include/gwaei/definitions.h] Doxyfile)
AC_OUTPUT
