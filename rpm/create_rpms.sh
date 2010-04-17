#!/bin/bash

topdir=$1
scriptdir=$(dirname $0)
scriptname=$(basename $0)
tarpkgname=$2-$3.tar.gz

distro=$(find $scriptdir -maxdepth 1 -type d | grep -v "^$scriptdir$" | sed 's#^\./##')


# Create rpm for distros
for i in $distro
do
	# Define variables
	toprpmdir=$topdir/$distro
	echo "Processing : $toprpmdir"

	# Create build environment
	# 1- Define rpmmacros
	if [ -f ~/.rpmmacros ]
		then
		# Backup the original env
		cp -f ~/.rpmmacros ~/.rpmmacros.backup
	fi

	echo "%_topdir      %$toprpmdir" > ~/.rpmmacros
	echo "%_smp_mflags  -j3" >> ~/.rpmmacros
	echo "%__arch_install_post   /usr/lib/rpm/check-rpaths   /usr/lib/rpm/check-buildroot" >> ~/.rpmmacros

	# 2- Create all rpmbuild directories
	mkdir -p $toprpmdir/SOURCES $toprpmdir/BUILD $toprpmdir/BUILDROOT $toprpmdir/RPMS $toprpmdir/SRPMS

	# 3- Move sources and patches
	cp -f $topdir/$tarpkgname $toprpmdir/SOURCES
	cp -f $toprpmdir/patches/* $toprpmdir/SOURCES

	exit
done



exit
rpm: dist
        @ make clean > /dev/null
        @ touch $(HOME)/.rpmmacros
        @ cp -f $(HOME)/.rpmmacros $(HOME)/.rpmmacros.backup
        @ echo "%_topdir $(PWD)/rpm" > $(HOME)/.rpmmacros
        @ mkdir -p $(top_srcdir)/rpm/SOURCES $(top_srcdir)/rpm/SPECS $(top_srcdir)/rpm/BUILD $(top_srcdir)/rpm/RPMS/@ARCH@ $(top_srcdir)/rpm/SRPMS/@ARCH@
        @ cp @PACKAGE@-@VERSION@.tar.gz $(top_srcdir)/rpm/SOURCES
        @ ${RPMBUILD} -ba --target @ARCH@ $(top_srcdir)/rpm/@PACKAGE@.spec
        @ cp $(RPM_RPMS)/@ARCH@/@PACKAGE@-@VERSION@-@RELEASE@.@ARCH@.rpm $(top_srcdir)
        @ rm -rf $(top_srcdir)/rpm/SOURCES $(top_srcdir)/rpm/SPECS $(top_srcdir)/rpm/BUILD $(top_srcdir)/rpm/RPMS $(top_srcdir)/rpm/SRPMS
        @ rm -f $(HOME)/.rpmmacros
        @ mv -f $(HOME)/.rpmmacros.backup $(HOME)/.rpmmacros
        @ echo "Success! Finished creating the rpm packge. Please have a sugary day."

fedora-rpm:
        @ $(top_srcdir)/configure --prefix /usr --sysconfdir /etc --docdir=/usr/share/doc/@PACKAGE@ > /dev/null
        @ make rpm

