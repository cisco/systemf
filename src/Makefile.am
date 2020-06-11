
lib_LTLIBRARIES = libsystemf.la
libsystemf_la_SOURCES := systemf.c systemf_debug.c task.c derived-lexer.c derived-parser.c
nodist_include_HEADERS := derived-lexer.h derived-parser.h
libsystemf_la_LDFLAGS = -avoid-version -shared
libsystemf_la_CFLAGS := -D_FORTIFY_SOURCE=2

derived-lexer.c derived-lexer.h: lexer.l
	flex --version | grep flex || (echo Flex is required ; false)
	flex lexer.l

derived-parser.c derived-parser.h: parser.y
	bison --version | grep 'bison.* 3' || (echo Bison 3 is required ; false)
	bison -v parser.y
