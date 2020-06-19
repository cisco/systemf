#ifndef __systemf_internal_h__
#define __systemf_internal_h__

#include <stdlib.h>
#include <stdarg.h>
#include "task.h"


typedef struct {
    _sf1_task *tasks;
    va_list *argpp;
} _sf1_parse_args ;

typedef enum {
    SYL_ESCAPE_GLOB=1,
    SYL_IS_GLOB=2,
    SYL_IS_FILE=4,
    SYL_IS_TRUSTED=8,
} _sf1_syl_flags;

typedef struct _sf1_syllable_ {
    struct _sf1_syllable_ *next;
    struct _sf1_syllable_ *next_word;
    _sf1_syl_flags flags;
    char text[];
} _sf1_syllable;

#ifndef YY_TYPEDEF_YY_SCANNER_T
    #define YY_TYPEDEF_YY_SCANNER_T
    typedef void* yyscan_t;
#endif

extern _sf1_redirect *_sf1_merge_redirects(_sf1_redirect *left, _sf1_redirect *right);
extern _sf1_redirect *_sf1_create_redirect(_sf1_stream stream, _sf1_stream target, int append, _sf1_syllable *file_syllables);
extern _sf1_task *_sf1_create_cmd(_sf1_syllable *syllables, _sf1_redirect *redirects);
extern void _sf1_create_redirect_pipe (_sf1_task *left, _sf1_task *right);

extern int _sf1_file_sandbox_check(char *trusted_path, char *path);

#endif /* __systemf_internal_h__ */
