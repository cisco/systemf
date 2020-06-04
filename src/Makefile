# This makefile is a workaround for not understanding the automake handling
# of flex and bison.  It just creates the built c and h files.

all: lexer.c parser.c

lexer.c: lexer.l
	flex lexer.l

parser.c: parser.y
	bison --version | grep 'bison.* 3' || (echo Bison 3 is required ; false)
	bison -v parser.y
