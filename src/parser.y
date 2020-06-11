%output  "derived-parser.c"
%defines "derived-parser.h"
%define api.pure full
%define api.prefix {_sf1_yy}
%define api.value.type union
%locations
%lex-param   { yyscan_t scanner }
%lex-param   { _sf1_parse_args *results }
%parse-param { yyscan_t scanner }
%parse-param { _sf1_parse_args *results }


%code requires {
    #if 1 // This compiles in debug code enabled by _sf1_yydebug;
        #undef SYSTEMF1_YYDEBUG
        #define SYSTEMF1_YYDEBUG 1
        extern int _sf1_yydebug;
    #endif

    #include "systemf-internal.h"

}

%code provides
{
    #define YYSTYPE _SF1_YYSTYPE
    #define YYLTYPE _SF1_YYLTYPE
    #define YY_DECL int _sf1_yylex(YYSTYPE * yylval_param , YYLTYPE *yylloc, yyscan_t yyscanner, _sf1_parse_args *results)
    extern YY_DECL;

    int _sf1_yyerror(_SF1_YYLTYPE *locp, yyscan_t scanner, _sf1_parse_args *results, const char *msg);
}

%code {
    /*
     * Put the parser code in a file that IDE's better understand, but
     * don't expose the statics to the world.
     */
    #include "systemf-internal.h"
}

%token <_sf1_syllable *> SYLLABLE
%token SPACE QUOTE LESSER TWO_GREATER_AND_ONE TWO_GREATER AND_GREATER GREATER TWO_GREATER_GREATER AND_GREATER_GREATER
%token GREATER_GREATER GREATER_AND_TWO AND_AND OR_OR SEMICOLON OR
%type <_sf1_syllable *> syllables words
%type <_sf1_task *> cmd cmds
%type <_sf1_redirect *> redirect redirects

%%

cmds:
  cmd		               { results->tasks = $1; }
| cmd SEMICOLON cmds   { results->tasks = $1; $1->next = $3; $3->run_if = _SF1_RUN_ALWAYS; }
| cmd OR_OR cmds       { results->tasks = $1; $1->next = $3; $3->run_if = _SF1_RUN_IF_PREV_FAILED; }
| cmd AND_AND cmds     { results->tasks = $1; $1->next = $3; $3->run_if = _SF1_RUN_IF_PREV_SUCCEEDED;  }
/* | cmd OR cmds          { results->tasks = $1; $1->next = $3; $3->run_if = _SF1_RUN_ALWAYS;  }  _SF1_PIPE comes in here*/

cmd:
  words redirects        { $$ = _sf1_create_cmd($1, $2); }

redirects:
  redirect redirects     { $$ = _sf1_merge_redirects($1, $2); }
| redirect               { $$ = $1; }
| /* empty */            { $$ = NULL; }

redirect:
  LESSER syllables                  { $$ = _sf1_create_redirect(_SF1_STDIN,  _SF1_FILE,  0, $2); }
| GREATER syllables                 { $$ = _sf1_create_redirect(_SF1_STDOUT, _SF1_FILE,  0, $2); }
| GREATER_GREATER syllables         { $$ = _sf1_create_redirect(_SF1_STDOUT, _SF1_FILE,  1, $2); }
| GREATER_AND_TWO                   { $$ = _sf1_create_redirect(_SF1_STDOUT, _SF1_SHARE, 0, NULL); }
| TWO_GREATER_AND_ONE               { $$ = _sf1_create_redirect(_SF1_STDERR, _SF1_SHARE, 0, NULL); }
| TWO_GREATER syllables             { $$ = _sf1_create_redirect(_SF1_STDERR, _SF1_FILE,  0, $2); }
| TWO_GREATER_GREATER syllables     { $$ = _sf1_create_redirect(_SF1_STDERR, _SF1_FILE,  1, $2); }
| AND_GREATER syllables             { $$ = _sf1_create_redirect(_SF1_STDERR, _SF1_SHARE, 0, NULL);
                                $$->next = _sf1_create_redirect(_SF1_STDOUT, _SF1_FILE,  0, $2); }
| AND_GREATER_GREATER syllables     { $$ = _sf1_create_redirect(_SF1_STDERR, _SF1_SHARE, 1, NULL);
                                $$->next = _sf1_create_redirect(_SF1_STDOUT, _SF1_FILE,  1, $2); }

words:
  syllables              { $$ = $1; }
| syllables SPACE words  { $1->next_word = $3; $$ = $1; }
| error                  { YYABORT; }

syllables:
  SYLLABLE	             { $$ = $1; }
| SYLLABLE syllables     { $1->next = $2; $$ = $1; }


%%

