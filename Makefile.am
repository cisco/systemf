SUBDIRS = src

AM_CFLAGS = # CFLAGS applicable to all executables (products)
AM_CPPFLAGS = -I$(top_srcdir)/src # so that tests also find header files
AM_YFLAGS = -d


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
