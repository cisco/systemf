#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "derived-parser.h"
#include "derived-lexer.h"
#include "task.h"

int systemf1_yyerror(SYSTEMF1_YYLTYPE *locp, yyscan_t scanner, systemf1_parse_args *result, const char *msg) {
  printf("ERROR: %d:%d:%s\n", locp->first_line, locp->first_column, msg);
  return 1;
}

int systemf1(const char *fmt, ...)
{
    // extern int systemf1_yydebug; systemf1_yydebug = 1; // for debugging issues
    va_list argp;
    yyscan_t scanner;
    systemf1_parse_args *result = calloc(1, sizeof(systemf1_parse_args));

    va_start(argp, fmt);
    result->argpp = &argp;

    if (systemf1_yylex_init(&scanner)) exit(1);
    YY_BUFFER_STATE buf = systemf1_yy_scan_string(fmt, scanner);
    if (systemf1_yyparse(scanner, result)) exit(1);
    va_end(argp);
    systemf1_yy_delete_buffer(buf, scanner);
    systemf1_yylex_destroy(scanner);

    int ret = systemf1_tasks_run(result->tasks);

    return (ret);
}
