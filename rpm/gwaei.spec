%define prefix /usr/local
%define sysconfdir /etc

%define localedir ${datarootdir}/locale

Summary: Japanese-English Dictionary for Gnome
Name: gwaei
Version: 1.0.1
Release: 1
License: GPL
Group: Applications/Utilities
Source: ./gwaei-1.0.1.tar.gz
URL: http://gwaei.sourceforge.net
BuildRoot: /var/tmp/gwaei-%{PACKAGE_VERSION}-root
%description
gWaei is a drop-in replacement for Gjiten. The main
feature of this dictonary program is regular expression
support. For example, you can use periods in place of
unknown Kanji of a word to search for it.
%prep
%setup -q
./configure --prefix /usr/local --sysconfdir=/etc --disable-schemas-install
[ -f rpm/rpm_gconf_install_post.sh ]
[ -f rpm/rpm_gconf_uninstall_pre.sh ]
%build
make
%install
make DESTDIR=$RPM_BUILD_ROOT install
%clean
rm -rf $RPM_BUILD_ROOT
%post
%include rpm/rpm_gconf_install_post.sh
%preun
%include rpm/rpm_gconf_uninstall_pre.sh
%files
%defattr(-, root, root)
#%doc NEWS INSTALL COPYING ChangeLog
%{prefix}/bin/gwaei
%{prefix}/bin/waei
%{prefix}/share/applications/gwaei.desktop
%{prefix}/share/gnome/
%{prefix}/share/gwaei/
%{prefix}/share/icons/
%{prefix}/share/locale/
%{prefix}/share/doc/
%{prefix}/share/omf/gwaei/gwaei-C.omf
%{sysconfdir}/gconf/schemas/gwaei.schemas


