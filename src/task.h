#ifndef __task_h__
#define __task_h__

#include <glob.h>

typedef enum {
    SYSTEMF1_STDIN,
    SYSTEMF1_STDOUT,
    SYSTEMF1_SHARE, // If in use FD or out.  If out, use FD of in.
    SYSTEMF1_STDERR,
    SYSTEMF1_PIPE,
    SYSTEMF1_FILE,
} systemf1_stream;

typedef enum {
    SYSTEMF1_RUN_ALWAYS = 0,
    SYSTEMF1_RUN_IF_PREV_FAILED = 1,
    SYSTEMF1_RUN_IF_PREV_SUCCEEDED = 2,
} systemf1_run_if;

typedef struct _systemf1_task_arg {
    struct _systemf1_task_arg *next;
    int is_glob;
    char *text;
    char *path;
    glob_t glob;
} systemf1_task_arg;

typedef struct _systemf1_redirect {
    struct _systemf1_redirect *next;
    systemf1_stream stream; // May only be STDIN, STDOUT, or STDERR
    systemf1_stream target;
    char *text;
    char *path;
    int append;
} systemf1_redirect;

typedef struct systemf1_task_ {
    char **argv;
    systemf1_run_if run_if;
    systemf1_redirect *redirects;
    systemf1_task_arg *args;
    struct systemf1_task_ *next;
} systemf1_task;

typedef struct systemf1_task_files_ {
    int in;
    int out;
    int err;
    int out_rd_pipe;
} systemf1_task_files;

extern systemf1_task *systemf1_task_create();
extern int systemf1_tasks_run(systemf1_task *task);
extern systemf1_task_arg *_sf1_task_add_arg(systemf1_task *task, char *text, char *path, int is_glob);
extern void systemf1_task_add_redirects(systemf1_task *task, systemf1_redirect *redirect);
extern void systemf1_task_set_run_if (systemf1_task *task, systemf1_run_if run_if);
extern void systemf1_task_free(systemf1_task *task);
extern char *systemf1_stream_name(systemf1_stream);
extern void _sf1_close_upper_fd(void);
#endif /* __task_h__ */
