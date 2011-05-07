## Process this file with automake to produce Makefile.in

PACKAGE = @PACKAGE@
VERSION = @VERSION@
datadir = @datadir@


dist:
	cp --force main.ui main.glade
	cp --force radicals.ui radicals.glade
	cp --force settings.ui settings.glade
	cp --force dictionaryinstall.ui dictionaryinstall.glade
	cp --force installprogress.ui installprogress.glade

pkgdata_DATA = main.ui radicals.ui settings.ui dictionaryinstall.ui installprogress.ui

EXTRA_DIST =  main.glade radicals.glade settings.glade dictionaryinstall.glade installprogress.glade main.ui radicals.ui settings.ui dictionaryinstall.ui installprogress.ui

