check_PROGRAMS = test-runner cmd

test_runner_SOURCES = test-runner.c
test_runner_LDADD = ../src/libsystemf.la

test_runner_CFLAGS = $(AM_CFLAGS) $(CODE_COVERAGE_CFLAGS)
test_runner_LDFLAGS = $(CODE_COVERAGE_LDFLAGS)

cmd_SOURCES = cmd.c
test_runner_CFLAGS = $(AM_CFLAGS) $(CODE_COVERAGE_CFLAGS)
test_runner_LDFLAGS = $(CODE_COVERAGE_LDFLAGS)

test-runner.c: test.json test-generator.py
	python3 ./test-generator.py

TESTS = test.test
TESTS_ENVIRONMENT =
TEST_EXTENSIONS = .test

TEST_LOG_COMPILE = $(PYTHON)
TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) $(top_srcdir)/build-aux/tap-driver.sh

