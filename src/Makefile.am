## Process this file with automake to produce Makefile.in
#
# Author: Dan Gunter <dkgunter@lbl.gov>
# Created: 9 Oct 2009
# RCSID: $Id$


# Header files
ACLOCAL_AMFLAGS			 = -I m4
include_HEADERS			 = netlogger_calipers.h bson.h platform_hacks.h

# Library
lib_LTLIBRARIES			 = libnetlogger_calipers.la
libnetlogger_calipers_la_SOURCES = netlogger_calipers.c bson.c numbers.c
LDADD				 = libnetlogger_calipers.la

# Test programs
noinst_PROGRAMS			   = netlogger_calipers_example \
				     ps_calipers_bench
netlogger_calipers_example_SOURCES = netlogger_calipers_example.c
ps_calipers_bench_SOURCES		= ps_calipers_bench.c
disk_bench_SOURCES				= disk_bench.c

#EXTRA_DIST = $(other_headers)

# Add this to make debugging easier
# AM_CFLAGS = -g3 -O0 -ggdb

AM_CFLAGS=-DMONGO_HAVE_STDINT

clean-local:
	/bin/rm -f *~
