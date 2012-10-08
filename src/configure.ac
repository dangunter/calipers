dnl Process this file with autoconf to produce a configure script.
dnl

AC_INIT([nl-calipers],trunk,[dkgunter@lbl.gov],[nl-calipers])

printf '
         .                                                                   
     ~.   =+                                                                  
    .?:. ++?      88D.                                                        
     7~  +?.      .D                                                          
,:,::~~: ??I?+7????III?+=?II=                                                 
.,,:~~~~~~=~===++???++==+++++?++++??????++++?++=~~~:::::~~~~~~~~=+?~I.        
.,:~~~~~~=I===++?I?+8+=+?+++++$+++++?+?++++?O==?~~:O::~~~O+~:~~=+?II7=??I+I?  
.:~~~~~~====+++???+I$I$O$OO88ZI8Z+$7?7++++?++==~~::::+:?7O+7D7OO??7~$.        
.~~~~         ==+??7+?II?+=+II..          ...   .....     .      .. ....        
.~~~~         =++?...     .III .                                                
.~~:~         ++??           .                                                  
 ~:~=.      ..+??+.                                                             
 :~==       . ???.                                                              
  ~==       ..???                                                               
  ===.        ~?~                                                               
  .=I       ..I?                                                                
  .=?         ?I                                                                
     NetLogger 
     Calipers
        API

Copying and redistribution of the contents of the source code
files in this directory, unless otherwise indicated in the
files themselves, is restricted under the terms given in the file 'LICENSE.txt', found at the top-level directory of this distribution
or online at https://raw.github.com/dangunter/calipers/master/LICENSE.txt

'
AC_CANONICAL_TARGET
AC_CONFIG_SRCDIR(nl_calipers.h)
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
examples/Makefile
])

AC_OUTPUT
