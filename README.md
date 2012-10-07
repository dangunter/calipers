# Introduction #

This is the NetLogger Calipers API, which is a C
API for instrumenting high-frequency events like
data transfers. It produces a summary of a stream of
timings as periodic reports that include the min, max,
mean, etc. of the timings as well as a histogram of the
"rate" (value per time) and "gap" (time per value).

# Build and Install #

The build has only been tested on UNIX and Mac OSX systems.
Code is in the `src/` subdirectory. The build system uses
the GNU automake, autoconf, libtool toolchain. First you
must have these installed on your system. Then:

    cd src
    ./autogen.sh
    ./configure.sh [options]
    make
    # make test TBD!!
    make install

# Using the API #

See the detailed online documentation (TBD!), or
build the docs yourself (requires Sphinx and Doxygen):

    cd doc
    # edit paths in Makefile
    make


