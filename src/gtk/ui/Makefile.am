## Process this file with automake to produce Makefile.in

PACKAGE = @PACKAGE@
VERSION = @VERSION@

all:
	cp --force main.ui main.glade
	cp --force radicals.ui radicals.glade
	cp --force settings.ui settings.glade
	cp --force kanjipad.ui kanjipad.glade
	cp --force dictionaryinstall.ui dictionaryinstall.glade
	cp --force installprogress.ui installprogress.glade

clean:
	rm --force *.glade
	

pkgdata_DATA = main.ui radicals.ui settings.ui kanjipad.ui dictionaryinstall.ui installprogress.ui

