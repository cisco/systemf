AM_CFLAGS = # CFLAGS applicable to all executables (products)
AM_CPPFLAGS = -I$(top_srcdir)/src # so that tests also find header files
AM_YFLAGS = -d

# The test executable
bin_PROGRAMS = myexecutable # make all will generate ./my_executable

lib_LTLIBRARIES = libsystemf.la
libsystemf_la_SOURCES = \
   src/derived-lexer.c \
   src/derived-lexer.h \
   src/derived-parser.h \
   src/derived-parser.c \
   src/systemf.c \
   src/systemf.h \
   src/systemf-internal.h \
   src/systemf_debug.c \
   src/task.c \
   src/task.h
libsystemf_la_LDFLAGS = -avoid-version -shared

# Yacc's derived parser.h
# For now, just use src/Makefile to create these build files.
# BUILT_SOURCES = src/parser.h src/lexer.h

#include .c and .h in SOURCES so that both appear in dist
myexecutable_SOURCES = src/main.c
myexecutable_LDADD = libsystemf.la

myexecutable_CFLAGS = $(AM_CFLAGS) $(CODE_COVERAGE_CFLAGS) #--std=c11 # CFLAGS applicable to myexecutable_SOURCES
myexecutable_LDFLAGS = $(CODE_COVERAGE_LDFLAGS)
# myexecutable_CFLAGS = $(CODE_COVERAGE_CFLAGS)

lexer:


# Tests
# 'check' comes from 'make check'

@VALGRIND_CHECK_RULES@
@CODE_COVERAGE_RULES@

# See all code_coverage_* options in the Makefile or m4/ax_code_coverage.m4
CODE_COVERAGE_IGNORE_PATTERN = tests/*
CODE_COVERAGE_IGNORE_PATTERN += stdio2.h

# Anything ending in .test should be a runnable script that produces TAP output
# Example Output, with 2 tests:
# 1..2
# ok 1 - message
# not ok 2 - message
# See http://www.gnu.org/software/automake/manual/html_node/Use-TAP-with-the-Automake-test-harness.html
TESTS_ENVIRONMENT =
TEST_EXTENSIONS = .test

TEST_LOG_COMPILE = $(PYTHON)
TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh

TESTS = tests/test.test
