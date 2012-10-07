dnl Process this file with autoconf to produce a configure script.
dnl

AC_INIT([netlogger-calipers],trunk,[dkgunter@lbl.gov],[netlogger-calipers])

printf '
 _____       _    __
|   | | ___ | |_ |  |    ___  ___  ___  ___  ___
| | | || -_||  _||  |__ | . || . || . || -_||  _|
|_|___||___||_|  |_____||___||_  ||_  ||___||_|
                             |___||___|

NetLogger Calipers API

For copyright and License see the file 'LICENSE' in the top level
source-code directory

'
AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR(netlogger_calipers.h)
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER(defs/nlconfig.h)
AC_CONFIG_MACRO_DIR([m4])

dnl -- An ugly hack for Mac
dnl case "$host_os" in
dnl *darwin*)
dnl 	ARCH_LIBS=-lSystemStubs
dnl 	;;
dnl *)
dnl 	ARCH_LIBS=
dnl 	;;
dnl esac
ARCH_LIBS=

dnl --------------------------------------------------------------------
dnl Checks for programs.
dnl

AC_PROG_CC
dnl AM_PROG_CC_C_O
AC_PROG_LIBTOOL

dnl --------------------------------------------------------------------
dnl Checks for C header files.
dnl
AC_HEADER_STDC
AC_CHECK_HEADERS(malloc.h sys/time.h unistd.h)

dnl --------------------------------------------------------------------
dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM

dnl --------------------------------------------------------------------
dnl Checks for library functions.

AC_FUNC_STRFTIME
AC_CHECK_FUNCS(gettimeofday)
AC_SEARCH_LIBS([sqrt], [m])

dnl --------------------------------------------------------------------
dnl Makefiles
dnl
AC_CONFIG_FILES([
Makefile
])

AC_OUTPUT
