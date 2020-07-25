#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# From https://www.gnu.org/savannah-checkouts/gnu/autoconf/manual/autoconf-2.69/autoconf.html:
#
# Files used in preparing a software package for distribution, when using just Autoconf:
#
#      your source files --> [autoscan*] --> [configure.scan] --> configure.ac
#    
#      configure.ac --.
#                     |   .------> autoconf* -----> configure
#      [aclocal.m4] --+---+
#                     |   `-----> [autoheader*] --> [config.h.in]
#      [acsite.m4] ---'
#     
#      Makefile.in
# Additionally, if you use Automake, the following additional productions come into play:
#
#      [acinclude.m4] --.
#                       |
#      [local macros] --+--> aclocal* --> aclocal.m4
#                       |
#      configure.ac ----'
#     
#      configure.ac --.
#                     +--> automake* --> Makefile.in
#      Makefile.am ---'
# Files used in configuring a software package:
#
#                             .-------------> [config.cache]
#      configure* ------------+-------------> config.log
#                             |
#      [config.h.in] -.       v            .-> [config.h] -.
#                     +--> config.status* -+               +--> make*
#      Makefile.in ---'                    `-> Makefile ---'

AC_PREREQ([2.69])
AC_INIT([systemf], [1.0.0], [systemf@patnan.com])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign subdir-objects]) # Does not require NEWS, COPYING, AUTHORS, ChangeLog or README

# silent make https://autotools.io/automake/silent.html
# silent rules enabled by default with 'yes'
# disable silent runles with ./configure --disable-silent-rules
AM_SILENT_RULES([yes]) # less verbose make output 
# AM_SILENT_RULES() # use make -s to get silent output

AC_CONFIG_SRCDIR([src/systemf.c])
AC_CONFIG_HEADERS([config.h]) # use config.h instead of passing -D in the command line
AC_CONFIG_MACRO_DIR([m4])

AC_LANG([C]) # Use C not C++

# Checks for programs.
AC_PROG_CC

# In case that you want to check for specific versions of gcc 
# For example in case that you need C11 support you want to 
# check for gcc-4.9 
#AC_PROG_CC([gcc-4.9 gcc cc])

AC_PROG_CC_C99 # or AC_PROG_CC_89 to force C89 mode or AC_PROG_CC_STDC to go to latest supported standard (currently C99)

AC_PROG_INSTALL
AC_PROG_CC_C_O # Need to have per product flags myexecutable_CFLAG
AC_PROG_RANLIB # Need for to create libraries: .a 

# Need libtool init to make .so's
LT_INIT

dnl For now, use src/makefile to compile with bison and flex.
dnl AX_PROG_BISON()
dnl AM_PROG_LEX
dnl AC_PROG_YACC

# Checks for libraries.

# Found libraries are automatically addded to LIBS
# AC_SEARCH_LIBS([pthread_cond_wait], [pthread],[],[
#                 AC_MSG_ERROR([You need to install pthreads library.])
#                 ])

# AC_SEARCH_LIBS([g_test_init], [glib-2.0],[],[
#                 AC_MSG_ERROR([You need to install glib-2.0 library.])
#                 ])

# Checks for header files.
AC_HEADER_ASSERT # ./configure --disable-assert to define NDEBUG 
AC_CHECK_HEADER([stdlib.h])

# Check for C11's optional Atomic operations library 
# AC_CHECK_HEADER([stdatomic.h], [], [
#                 AC_MSG_ERROR([C11 with atomic support needed.])
#                  ])

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

# The following statement will use pkg-config --cflags --libs 
# to find out CFLAGS and -l options required to build a target that 
# it's going to link against glib2.0.
# The required CFLAGS and -l options are available as DEPS_CFLAGS
# and DEPS_LIBS in Makefile.am
# PKG_CHECK_MODULES([DEPS], [glib-2.0 >= 2.24.1]) 

AC_ARG_ENABLE([myfeature], 
              AS_HELP_STRING([--disable-myfeature],[disable myfeature to get remove support for this and that]),
              [enable_myfeature=${enableval}],[enable_myfeature=yes])

if test "x${enable_myfeature}" == "xyes"; then
  AC_DEFINE([MYFEATURE], 1, [myfeature is enabled])
else
  AC_MSG_WARN([
-----------------------------------------------------------------------------------
Are you sure that you want to have myfeature disabled? You will lose this and that.
-----------------------------------------------------------------------------------
               ])
fi

AC_CONFIG_FILES([
 Makefile
])
AC_REQUIRE_AUX_FILE([tap-driver.sh])
AX_VALGRIND_CHECK # http://www.gnu.org/software/autoconf-archive/ax_valgrind_check.html - make check-valgrind
AX_CODE_COVERAGE # http://www.gnu.org/software/autoconf-archive/ax_code_coverage.html#ax_code_coverage - make check-code-coverage generates coverage report
AC_OUTPUT
