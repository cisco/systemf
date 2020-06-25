#ifndef __systemf_internal_h__
#define __systemf_internal_h__

#include <stdlib.h>
#include <stdarg.h>
#include <glob.h>

typedef enum {
    _SF1_STDIN,
    _SF1_STDOUT,
    _SF1_SHARE, // If in use FD or out.  If out, use FD of in.
    _SF1_STDERR,
    _SF1_PIPE,
    _SF1_FILE,
} _sf1_stream;

typedef enum {
    _SF1_RUN_ALWAYS = 0,
    _SF1_RUN_IF_PREV_FAILED = 1,
    _SF1_RUN_IF_PREV_SUCCEEDED = 2,
} _sf1_run_if;

typedef struct _sf1_task_arg_ {
    struct _sf1_task_arg_ *next;
    int is_glob;
    char *text;
    char *trusted_path;
    glob_t glob;
} _sf1_task_arg;

typedef struct _sf1_redirect_ {
    struct _sf1_redirect_ *next;
    _sf1_stream stream; // May only be STDIN, STDOUT, or STDERR
    _sf1_stream target;
    char *text;
    char *trusted_path;
    int append;
} _sf1_redirect;

typedef struct _sf1_task_ {
    char **argv;
    _sf1_run_if run_if;
    _sf1_redirect *redirects;
    _sf1_task_arg *args;
    struct _sf1_task_ *next;
} _sf1_task;

typedef struct _sf1_task_files_ {
    int in;
    int out;
    int err;
    int out_rd_pipe;
} _sf1_task_files;

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


extern _sf1_task *_sf1_task_create();
extern int _sf1_tasks_run(_sf1_task *task);
extern _sf1_task_arg *_sf1_task_add_arg(_sf1_task *task, char *text, char *trusted_path, int is_glob);
extern void _sf1_task_add_redirects(_sf1_task *task, _sf1_redirect *redirect);
extern void _sf1_task_free(_sf1_task *task);
extern char *_sf1_stream_name(_sf1_stream);
extern void _sf1_close_upper_fd(void);

#endif /* __systemf_internal_h__ */
