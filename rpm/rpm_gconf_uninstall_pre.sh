#!/bin/sh -e

#Install and update the gconf schema database
INSTALL_SOURCE=$(/usr/bin/gconftool-2 --get-default-source | sed 's!^[^/]*/!/!' | sed 's!/[^/]*$!/schemas/gwaei.schemas!')
echo $INSTALL_SOURCE
GCONF_CONFIG_SOURCE=xml:merged:/etc/gconf/gconf.xml.defaults \
/usr/bin/gconftool-2 --makefile-uninstall-rule $INSTALL_SOURCE
/usr/bin/killall -q -HUP gconfd-2

#Force the icons to update
touch -c /usr/share/icons/hicolor
