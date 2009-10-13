#! /bin/sh

aclocal \
&& gnome-doc-prepare -c -f \
&& automake -c -f --add-missing \
&& autoconf -f
