%option outfile="derived-lexer.c" header-file="derived-lexer.h"
%option reentrant
%option bison-bridge
%option noyywrap nounput noinput
%option prefix="systemf1_yy"

%{
#include <stdarg.h>
#include "derived-parser.h"

static void yy_user_action(SYSTEMF1_YYLTYPE *yylloc, const char *my_yytext)
{
    yylloc->first_line = yylloc->last_line;
    yylloc->first_column = yylloc->last_column;
    for(int i = 0; my_yytext[i] != '\0'; i++) {
        if(my_yytext[i] == '\n') {
            yylloc->last_line++;
            yylloc->last_column = 0;
        } else {
            yylloc->last_column++;
        }
    }
}

#define YY_USER_ACTION yy_user_action(yylloc, yytext);

static syllable *syl (systemf1_parse_args *results, char *text, int flags) {
    size_t bufsize = strlen(text) + 1;
    syllable *syl = malloc(sizeof(*syl) + bufsize);
    syl->flags = flags;
    memcpy(syl->text, text, bufsize);
    syl->next = NULL;
    syl->next_word = NULL;
    return syl;
}
static syllable *syl_s (systemf1_parse_args *results) {
    return syl(results, va_arg(*results->argpp, char *), SYL_ESCAPE_GLOB);
}
static syllable *syl_file (systemf1_parse_args *results) {
    return syl(results, va_arg(*results->argpp, char *), SYL_IS_FILE|SYL_ESCAPE_GLOB);
}
static syllable *syl_glob (systemf1_parse_args *results) {
    return syl(results, va_arg(*results->argpp, char *), SYL_IS_FILE|SYL_IS_GLOB);
}
static syllable *syl_trusted_file (systemf1_parse_args *results) {
    return syl(results, va_arg(*results->argpp, char *), SYL_IS_FILE|SYL_IS_TRUSTED|SYL_ESCAPE_GLOB);
}
static syllable *syl_d (systemf1_parse_args *results) {
    char text[20];
    int val = va_arg(*results->argpp, int);
    snprintf(text, sizeof(text), "%d", val);
    return syl(results, text, 0);
}

%}


%%

[[:alnum:]/_.-]+           { yylval->SYLLABLE = syl(results, yytext, SYL_IS_TRUSTED); return SYLLABLE; }
\\[*\[\]?]                { yylval->SYLLABLE = syl(results, yytext, SYL_IS_FILE|SYL_IS_GLOB|SYL_IS_TRUSTED); return SYLLABLE; }
[\*\?]                    { yylval->SYLLABLE = syl(results, yytext, SYL_IS_FILE|SYL_IS_GLOB|SYL_IS_TRUSTED); return SYLLABLE; }
%s                        { yylval->SYLLABLE = syl_s(results); return SYLLABLE; }
%p                        { yylval->SYLLABLE = syl_file(results); return SYLLABLE; }
%!p                       { yylval->SYLLABLE = syl_trusted_file(results); return SYLLABLE; }
%\*p                      { yylval->SYLLABLE = syl_glob(results); return SYLLABLE; }
%d                        { yylval->SYLLABLE = syl_d(results); return SYLLABLE; }
[ \t]+                    { return SPACE;}
[ \t]*\<[ \t]*            { return LESSER; }
[ \t]*2>&1                { return TWO_GREATER_AND_ONE; }
[ \t]*2>[ \t]*            { return TWO_GREATER; }
[ \t]*&>[ \t]*            { return AND_GREATER; }
[ \t]*>[ \t]*             { return GREATER; }
[ \t]*>&2                 { return GREATER_AND_TWO; }
[ \t]*2>>[ \t]*           { return TWO_GREATER_GREATER; }
[ \t]*&>>[ \t]*           { return AND_GREATER_GREATER; }
[ \t]*>>[ \t]*            { return GREATER_GREATER; }
[ \t]*&&[ \t]*            { return AND_AND; }
[ \t]*\|[ \t]*            { return OR; }
[ \t]*\|\|[ \t]*          { return OR_OR; }
[ \t]*;[ \t]*             { return SEMICOLON; }
.|\n                      { return yytext[0]; }

%%

