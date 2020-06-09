%output  "derived-parser.c"
%defines "derived-parser.h"
%define api.pure full
%define api.prefix {systemf1_yy}
%define api.value.type union
%locations
%lex-param   { yyscan_t scanner }
%lex-param   { systemf1_parse_args *results }
%parse-param { yyscan_t scanner }
%parse-param { systemf1_parse_args *results }


%code requires {
    #if 1 // This compiles in debug code enabled by systemf1_yydebug;
        #undef SYSTEMF1_YYDEBUG
        #define SYSTEMF1_YYDEBUG 1
        extern int systemf1_yydebug;
    #endif

    #include "systemf-internal.h"

}

%code provides
{
    #define YYSTYPE SYSTEMF1_YYSTYPE
    #define YYLTYPE SYSTEMF1_YYLTYPE
    #define YY_DECL int systemf1_yylex(YYSTYPE * yylval_param , YYLTYPE *yylloc, yyscan_t yyscanner, systemf1_parse_args *results)
    extern YY_DECL;

    int systemf1_yyerror(SYSTEMF1_YYLTYPE *locp, yyscan_t scanner, systemf1_parse_args *results, const char *msg);
}

%code {
    /*
     * Put the parser code in a file that IDE's better understand, but
     * don't expose the statics to the world.
     */
    #include "parser-inc.c"
    #include "systemf-internal.h"
}

%token <syllable *> SYLLABLE
%token SPACE QUOTE LESSER TWO_GREATER_AND_ONE TWO_GREATER AND_GREATER GREATER TWO_GREATER_GREATER AND_GREATER_GREATER
%token GREATER_GREATER GREATER_AND_TWO AND_AND OR_OR SEMICOLON OR
%type <syllable *> syllables
%type <systemf1_task *> words cmd cmds
%type <systemf1_redirect *> redirect redirects

%%

cmds:
  cmd		       { results->tasks = $1; }
| cmd SEMICOLON cmds   { results->tasks = $1; $1->next = $3; $3->run_if = SYSTEMF1_RUN_ALWAYS; }
| cmd OR_OR cmds       { results->tasks = $1; $1->next = $3; $3->run_if = SYSTEMF1_RUN_IF_PREV_FAILED; }
| cmd AND_AND cmds     { results->tasks = $1; $1->next = $3; $3->run_if = SYSTEMF1_RUN_IF_PREV_SUCCEEDED;  }
/* | cmd OR cmds          { results->tasks = $1; $1->next = $3; $3->run_if = SYSTEMF1_RUN_ALWAYS;  }*/

cmd:
  words redirects        { systemf1_task_add_redirects($1, $2); $$ = $1; }

redirects:
  redirect redirects     { $$ = merge_redirects($1, $2); }
| redirect               { $$ = $1; }
| /* empty */            { $$ = NULL; }

redirect:
  LESSER syllables                  { $$ = create_redirect(SYSTEMF1_STDIN,  SYSTEMF1_FILE,  0, $2); }
| GREATER syllables                 { $$ = create_redirect(SYSTEMF1_STDOUT, SYSTEMF1_FILE,  0, $2); }
| GREATER_GREATER syllables         { $$ = create_redirect(SYSTEMF1_STDOUT, SYSTEMF1_FILE,  1, $2); }
| GREATER_AND_TWO                   { $$ = create_redirect(SYSTEMF1_STDOUT, SYSTEMF1_SHARE, 0, NULL); }
| TWO_GREATER_AND_ONE               { $$ = create_redirect(SYSTEMF1_STDERR, SYSTEMF1_SHARE, 0, NULL); }
| TWO_GREATER syllables             { $$ = create_redirect(SYSTEMF1_STDERR, SYSTEMF1_FILE,  0, $2); }
| TWO_GREATER_GREATER syllables     { $$ = create_redirect(SYSTEMF1_STDERR, SYSTEMF1_FILE,  1, $2); }
| AND_GREATER syllables             { $$ = create_redirect(SYSTEMF1_STDERR, SYSTEMF1_SHARE, 0, NULL);
                                $$->next = create_redirect(SYSTEMF1_STDOUT, SYSTEMF1_FILE,  0, $2); }
| AND_GREATER_GREATER syllables     { $$ = create_redirect(SYSTEMF1_STDERR, SYSTEMF1_SHARE, 1, NULL);
                                $$->next = create_redirect(SYSTEMF1_STDOUT, SYSTEMF1_FILE,  1, $2); }

words:
  syllables              { $$ = add_argument(NULL, $1); }
| syllables SPACE words  { $$ = add_argument($3, $1); }
| error                  { YYABORT; }

syllables:
  SYLLABLE	         { $$ = $1; }
| SYLLABLE syllables     { $1->next = $2; $$ = $1; }


%%

