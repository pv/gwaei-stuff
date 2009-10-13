#! /bin/sh

aclocal \
&& automake -c -f --add-missing \
&& autoconf -f
