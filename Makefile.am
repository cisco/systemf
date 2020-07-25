AM_CFLAGS = # CFLAGS applicable to all executables (products)
AM_CPPFLAGS = -I$(top_srcdir)/src # so that tests also find header files
AM_YFLAGS = -d
EXTRA_DIST := README.md \
    tests/README.md \
    tests/test.test \
    tests/test.json \
    tests/test-generator.py \
    tests/cmd \
    src/parser.y \
    src/lexer.l

lib_LTLIBRARIES = libsystemf.la
libsystemf_la_SOURCES := \
    src/close.c \
    src/derived-lexer.c \
    src/derived-lexer.h \
    src/derived-parser.c \
    src/derived-parser.h \
    src/file-sandbox-check.c \
    src/parser-support.c \
    src/pid-chain.c \
    src/systemf.c \
    src/task.c \
    src/systemf-internal.h

include_HEADERS := src/systemf.h
libsystemf_la_LDFLAGS = -avoid-version -shared $(CODE_COVERAGE_LDFLAGS)
libsystemf_la_CFLAGS := -D_FORTIFY_SOURCE=2 -Wall -Werror $(CODE_COVERAGE_CFLAGS)

src/derived-lexer.c src/derived-lexer.h: src/lexer.l
	$(LEX) --version | grep flex || (echo Flex is required ; false)
	flex src/lexer.l

src/derived-parser.c src/derived-parser.h: src/parser.y
	$(BISON) --version | grep 'bison.* 3' || (echo Bison 3 is required ; false)
	bison -v src/parser.y

# Anything ending in .test should be a runnable script that produces TAP output
# Example Output, with 2 tests:
# 1..2
# ok 1 - message
# not ok 2 - message
# See http://www.gnu.org/software/automake/manual/html_node/Use-TAP-with-the-Automake-test-harness.html
#
# Run tests with 'make check'

check_PROGRAMS = test-runner cmd

test_runner_SOURCES = tests/test-runner.c
test_runner_LDADD = libsystemf.la

test_runner_CFLAGS = $(AM_CFLAGS)

cmd_SOURCES = tests/cmd.c
cmd_CFLAGS = $(AM_CFLAGS)
cmd_LDFLAGS = $(CODE_COVERAGE_LDFLAGS)

tests/test-runner.c: tests/test.json tests/test-generator.py
	python3 ./tests/test-generator.py

TESTS = tests/test.test
TESTS_ENVIRONMENT =
TEST_EXTENSIONS = .test

TEST_LOG_COMPILE = $(PYTHON)
TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh


@VALGRIND_CHECK_RULES@
@CODE_COVERAGE_RULES@

# See all code_coverage_* options in the Makefile or m4/ax_code_coverage.m4
CODE_COVERAGE_OUTPUT_FILE = $(PACKAGE_NAME)-coverage.info
CODE_COVERAGE_OUTPUT_DIRECTORY = $(PACKAGE_NAME)-coverage
CODE_COVERAGE_IGNORE_PATTERN = $(abs_top_builddir)/tests/*
CODE_COVERAGE_IGNORE_PATTERN += $(abs_top_builddir)/src/derived*
CODE_COVERAGE_IGNORE_PATTERN += /usr/include/*
CODE_COVERAGE_IGNORE_PATTERN += /usr/include/*/bits/*

# Hack where there is much likely something better:
print-code-coverage:
	@ grep -A3 Lines systemf-coverage/index.html | sed -e's/.*".//' -e 's/<.*//' | fmt -w80 | sed -e's/ /_/' -e 's, ,/,' -e 's/ / = /' -e 's/_/ /'
