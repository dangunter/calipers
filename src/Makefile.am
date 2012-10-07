## Process this file with automake to produce Makefile.in
#
# Author: Dan Gunter <dkgunter@lbl.gov>
# Created: 9 Oct 2009
# RCSID: $Id$


# Header files
ACLOCAL_AMFLAGS			 = -I m4
include_HEADERS			 = nl_calipers.h bson.h platform_hacks.h

# Library
lib_LTLIBRARIES			 	= libnl_calipers.la
libnl_calipers_la_SOURCES 	= nl_calipers.c bson.c numbers.c
LDADD				 		= libnl_calipers.la

#EXTRA_DIST = $(other_headers)

# Add this to make debugging easier
# AM_CFLAGS = -g3 -O0 -ggdb

AM_CFLAGS=-DMONGO_HAVE_STDINT

clean-local:
	/bin/rm -f *~
