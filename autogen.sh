#! /bin/sh

aclocal --force \
&& gnome-doc-prepare --copy --force \
&& automake --copy --force --add-missing \
&& autoconf --force
