## Process this file with automake to produce Makefile.in
#
# Author: Dan Gunter <dkgunter@lbl.gov>
# Created: Oct 2012


# Header files
ACLOCAL_AMFLAGS			= -I m4

# Add this to make debugging easier
# AM_CFLAGS = -g3 -O0 -ggdb
AM_CFLAGS=-DMONGO_HAVE_STDINT -I..

# Library
LDADD				 = ../libnl_calipers.la

# Programs
noinst_PROGRAMS					= nl_calipers_ex1 \
				      			  ps_calipers_bench \
				      			  disk_bench
nl_calipers_ex1_SOURCES 		= nl_calipers_ex1.c
ps_calipers_bench_SOURCES		= ps_calipers_bench.c
disk_bench_SOURCES				= disk_bench.c

#EXTRA_DIST = $(other_headers)


clean-local:
	/bin/rm -f *~
