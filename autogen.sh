#! /bin/sh

gettextize --force \
&& intltoolize --copy --force --automake \
&& libtoolize --copy --force \
&& gnome-doc-prepare --copy --force \
&& aclocal --force \
&& autoheader -f \
&& automake --copy --force --add-missing \
&& autoconf --force
