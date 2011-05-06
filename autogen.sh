#!/bin/sh

gettextize --force \
&& gnome-doc-prepare --copy --force \
&& aclocal -I m4 --force \
&& autoheader -f \
&& automake --copy --force --add-missing \
&& autoconf --force

#this line should go after gettextize
#&& intltoolize --copy --force --automake \
