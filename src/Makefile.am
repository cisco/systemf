
lib_LTLIBRARIES = libsystemf.la
libsystemf_la_SOURCES := systemf.c systemf_debug.c task.c derived-lexer.c derived-parser.c
nodist_include_HEADERS := derived-lexer.h derived-parser.h
libsystemf_la_LDFLAGS = -avoid-version -shared

# The test executable
bin_PROGRAMS = myexecutable # make all will generate ./my_executable

#include .c and .h in SOURCES so that both appear in dist
myexecutable_SOURCES = main.c
myexecutable_LDADD = libsystemf.la

myexecutable_CFLAGS = $(AM_CFLAGS) $(CODE_COVERAGE_CFLAGS) #--std=c11 # CFLAGS applicable to myexecutable_SOURCES
myexecutable_LDFLAGS = $(CODE_COVERAGE_LDFLAGS)

derived-lexer.c derived-lexer.h: lexer.l
	flex --version | grep flex || (echo Flex is required ; false)
	flex lexer.l

derived-parser.c derived-parser.h: parser.y
	bison --version | grep 'bison.* 3' || (echo Bison 3 is required ; false)
	bison -v parser.y
