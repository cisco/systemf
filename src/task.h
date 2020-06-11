#ifndef __task_h__
#define __task_h__

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
    char *path;
    glob_t glob;
} _sf1_task_arg;

typedef struct _sf1_redirect_ {
    struct _sf1_redirect_ *next;
    _sf1_stream stream; // May only be STDIN, STDOUT, or STDERR
    _sf1_stream target;
    char *text;
    char *path;
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

extern _sf1_task *_sf1_task_create();
extern int _sf1_tasks_run(_sf1_task *task);
extern _sf1_task_arg *_sf1_task_add_arg(_sf1_task *task, char *text, char *path, int is_glob);
extern void _sf1_task_add_redirects(_sf1_task *task, _sf1_redirect *redirect);
extern void _sf1_task_set_run_if (_sf1_task *task, _sf1_run_if run_if);
extern void _sf1_task_free(_sf1_task *task);
extern char *_sf1_stream_name(_sf1_stream);
extern void _sf1_close_upper_fd(void);
#endif /* __task_h__ */
