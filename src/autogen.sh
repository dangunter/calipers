#!/bin/sh

set -x
glibtoolize -c --automake || libtoolize -c --automake
aclocal
autoheader
autoconf
automake -a
rm -rf autom4te*.cache

