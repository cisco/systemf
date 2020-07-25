#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "systemf-internal.h"

#define DEBUG 0
#define VA_ARGS(...) , ##__VA_ARGS__
#define DBG(fmt, ...) if (DEBUG) { printf("%s:%-3d:%24s: " fmt "\n", __FILE__, __LINE__, __FUNCTION__ VA_ARGS(__VA_ARGS__)); }

typedef struct glob_list_ {
    struct glob_list_ *next;
    glob_t glob;
} glob_list;

_sf1_task *_sf1_task_create() {
    return calloc(1, sizeof(_sf1_task));
}

/*
 * _sf1_task_add_redirects - Adds a linked of redirections.
 *
 * task - Destination Task - Current redirects must be NULL,
 * redirects - A linked list of _sf1_redirects where:
 *   each redirect is malloced
 *   filename - Is NULL or the destination malloced filename (if appropriate)
 *   trusted_path - Is the malloced verification path for filename where NULL is current directory.
 *   append - Specifies if the redirect shoud append to the file.
 *
 * Note that all malloced memory passed in will be freed with _sf1_task_free().
 * Note that validation of parameters is delayed until that task start happens.
 */
void _sf1_task_add_redirects (_sf1_task *task, _sf1_redirect *redirects)
{
    assert(task->redirects == NULL);
    task->redirects = redirects;
}

/*
 * _sf1_task_add_arg - Adds the specified argument to the task.
 *
 * task - Destination Task
 * text - The text of the argument.  Must be malloced.
 * trusted_path - If path verification is to be done this is the pre-realpath path.  Either NULL or malloced.
 * is_glob - This is a glob expression that glob expansion will be needed for.
 *
 * Note that all malloced memory passed in will be freed with _sf1_task_free()
 */
_sf1_task_arg *_sf1_task_add_arg (_sf1_task *task, char *text, char *trusted_path, int is_glob)
{
    _sf1_task_arg *arg;

    arg = calloc(1, sizeof(_sf1_task_arg));
    if (arg == NULL) {
        return NULL;
    }

    arg->text = text;
    arg->trusted_path = trusted_path;
    arg->is_glob = is_glob;

    // Find the last item and append pp
    _sf1_task_arg **next_pp = &(task->args);
    while (*next_pp) {
        next_pp = &((*next_pp)->next);
    }
    *next_pp = arg;

    return arg;
}

/*
 * Maps a stream to a character string representation.
 */
char *_sf1_stream_name(_sf1_stream stream) {
    switch (stream) {
        case _SF1_STDIN: return "stdin"; break;
        case _SF1_STDOUT: return "stdout"; break;
        case _SF1_SHARE: return "shared"; break;
        case _SF1_STDERR: return "stderr"; break;
        case _SF1_PIPE: return "pipe"; break;
        case _SF1_FILE: return "file"; break;
        default: assert(0);
    }
}

/*
 * Walks all of the redirects and verifies that they are entered in a sane way.
 * Returns 0 if not and 1 if good.
 */
int _sf1_redirects_are_sane(_sf1_task *tasks)
{
    for (_sf1_task *t = tasks; t; t = t->next) {
        int in = 0;
        int out = 0;
        int err = 0;
        for (_sf1_redirect *r = t->redirects; r; r = r ->next) {
            int count;
            switch (r->stream) {
            case _SF1_STDIN:
                count = in = in + 1;
                break;
            case _SF1_STDOUT:
                count = out = out + 1;
                break;
            case _SF1_STDERR:
                count = err = err + 1;
                break;
            default:
                assert(0);
            }
            if (count > 1) {
                fprintf(stderr, "ERROR: There should only be one %s per command.", _sf1_stream_name(r->stream));
                return 0;
            }
            if ((r->target == _SF1_FILE) && !(r->text)) {
                // A filename must be supplied.
                return 0;
            }
        }
    }
    return 1;
}

/*
 * Extracts all the globs from the task arguments.
 * task - The task.
 * returns:
 *   0 on success
 *   GLOB_NOSPACE for running out of memory,
 *   GLOB_ABORTED for a read error, and
 *   GLOB_NOMATCH for when the number of matches doesn't match the specified allowed match count.
 */
int _sf1_extract_glob(_sf1_task *task)
{
    for (_sf1_task_arg *a = task->args; a != NULL; a = a->next) {
        if (a->is_glob) {
            int ret = glob(a->text, 0, NULL, &a->glob);
            // FIXME: do a bounds check.
            switch (ret) {
            case GLOB_NOSPACE:
                fprintf(stderr, "systemf: glob out of memory extracting: %s\n", a->text);
                return ENOMEM;
            case GLOB_ABORTED:
                fprintf(stderr, "systemf: glob aborted during extraction: %s\n", a->text);
                return EBADF;
            case GLOB_NOMATCH:
                fprintf(stderr, "systemf: no matches found: %s\n", a->text);
                return EINVAL;
            }
        }
    }
    return 0;
}

void _sf1_task_free(_sf1_task *task)
{
    _sf1_task *next;
    for (; task != NULL; task = next) {
        _sf1_task_arg *anext;
        for (_sf1_task_arg *a = task->args; a != NULL; a = anext) {
            globfree(&a->glob);
            free(a->text);
            free(a->trusted_path);
            anext = a->next;
            free(a);
        }

        _sf1_redirect *rnext;
        for (_sf1_redirect *r = task->redirects; r != NULL; r = rnext) {
            free(r->text);
            free(r->trusted_path);
            rnext = r->next;
            free(r);
        }

        free(task->argv);
        next = task->next;
        free(task);
    }
    return;
}

/*
 * Fills in the files for the tasks.
 * 
 * Returns -1 on failure and 0 on success.
 */
int _sf1_populate_task_files(_sf1_task *task, _sf1_task_files *files) {
    int pipefd[2];
    _sf1_redirect *redirect;
    int rwrwrw = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
    int prev_out_rd_pipe = files->out_rd_pipe;

    files->in = 0;
    files->out = 1;
    files->err = 2;
    files->out_rd_pipe = 0;

    for (redirect = task->redirects; redirect; redirect = redirect->next) {
        if (redirect->stream == _SF1_STDIN)  {
            if (redirect->target == _SF1_FILE) {
                files->in = open(redirect->text, O_RDONLY);
                if (files->in < 0) {
                    fprintf(stderr, "systemf: %s: %s\n", strerror(errno), redirect->text);
                    return -1;
                }
            } else { // _SF1_PIPE
                files->in = prev_out_rd_pipe;
            }
        } else if (redirect->stream == _SF1_STDOUT) {
            if (redirect->target == _SF1_FILE) {
                files->out = open(redirect->text, O_WRONLY | O_CREAT | (redirect->append ? O_APPEND : O_TRUNC), rwrwrw);
                if (files->out < 0) {
                    fprintf(stderr, "systemf: %s: %s\n", strerror(errno), redirect->text);
                    return -1;
                }
            } else if (redirect->target == _SF1_SHARE) {
                files->out = files->err;
            } else { // _SF1_PIPE
                if (pipe(pipefd)) {
                    fprintf(stderr, "systemf: %s opening a pipe\n", strerror(errno));
                    return -1;
                }
                files->out_rd_pipe = pipefd[0];
                files->out = pipefd[1];
            }
        } else { // _SF1_STDERR
            if (redirect->target == _SF1_FILE) {
                files->err = open(redirect->text, O_WRONLY | O_CREAT | (redirect->append ? O_APPEND : O_TRUNC), rwrwrw);
                if (files->err < 0) {
                    fprintf(stderr, "systemf: %s: %s\n\n", strerror(errno), redirect->text);
                    return -1;
                }
            } else if (redirect->target == _SF1_SHARE) {
                files->err = files->out;
            }
        }
    }
    return 0;
}

/*
 * Check all of the arguments for this task that the files are properly sandboxed.
 * On success, returns 0.  On failure sets the errno and returns -1;
 */
int _sf1_file_sandbox_check_args(_sf1_task *task) {
    _sf1_task_arg *arg;
    int ret;

    for (arg = task->args; arg != NULL; arg = arg->next) {
        if (arg->trusted_path) {
            if (arg->is_glob) {
                int i;
                for (i = 0, ret = 0; (i < arg->glob.gl_pathc) && !ret; i++) {
                    ret = _sf1_file_sandbox_check(arg->trusted_path, arg->glob.gl_pathv[i]);
                }
            } else {
                ret = _sf1_file_sandbox_check(arg->trusted_path, arg->text);
            }
            if (ret) {
                errno = ret;
                return -1;
            }
        }
    }
    return 0;
}


/*
 * Close the child in out and error if they aren't shared with the parent.
 */
void _sf1_close_child_files(_sf1_task_files *files) {
    if (files->in > 2) {
        close(files->in);
        files->in = 0;
    }
    if (files->out > 2) {
        close(files->out);
        files->out = 1;
    }
    if (files->err > 2) {
        close(files->err);
        files->err = 2;
    }
}

/*
 * Close the child in out, error, and pipe.
 */
void _sf1_close_child_files_and_pipe(_sf1_task_files *files) {
    if (files->out_rd_pipe) {
        close(files->out_rd_pipe);
        files->out_rd_pipe = 0;
    }
    _sf1_close_child_files(files);
}

int _sf1_tasks_run(_sf1_task *tasks) {
    pid_t pid;
    _sf1_pid_chain_t *pid_chain = NULL;
    int stat;
    char **argv;
    _sf1_task_arg *arg;
    size_t argc = 1; // 1 for terminating NULL
    int ret;
    int retval = -1;
    _sf1_task_files files = {.in=0, .out=1, .err=2, .out_rd_pipe=0};

    if (!_sf1_redirects_are_sane(tasks)) {
        goto exit_error;
    }

     // We don't support tasks reuse, so argv MUST be null coming into this.
    assert(tasks->argv == NULL);

    for (_sf1_task *task = tasks; task; task = task->next) {
        ret = _sf1_extract_glob(task);
        if (ret) {
            errno = ret;
            goto exit_error;
        }

        // Count the arguments.
        for (arg = task->args; arg != NULL; arg = arg->next) {
            if (arg->is_glob) {
                argc += arg->glob.gl_pathc;
            } else {
                argc += 1;
            }
        }

        if (_sf1_file_sandbox_check_args(task)) {
            goto exit_error;
        }

        task->argv = malloc(argc * sizeof(char *));
        // FIXME: handle NULL

        argv = task->argv;
        for (arg = task->args; arg != NULL; arg = arg->next) {
            if (arg->is_glob) {
                memcpy(argv, arg->glob.gl_pathv, sizeof(char *) * arg->glob.gl_pathc);
                argv += arg->glob.gl_pathc;
            } else {
                *argv = arg->text;
                argv++;
            }
        }
        *argv = NULL;
        DBG("_____________________ err exi exs sig tsig\n");

        if (_sf1_populate_task_files(task, &files)) {
            goto exit_error;
        }

        // If we don't flush, both forks will send the buffered data and it will be seen twice.
        fflush(stdout);
        fflush(stderr);

        // FIXME: Determine if the file exists and it is executable before attempting
        // to fork which doesn't know how to handle results.

        pid = fork();
        if (pid == 0) {
            dup2(files.in, 0);
            dup2(files.out, 1);
            dup2(files.err, 2);
            _sf1_close_upper_fd();

            DBG("Running %s", task->argv[0]);
            stat = execv(*task->argv, task->argv);
            DBG("Execv   returned with %3d %3d %3d %3d %3d\n", errno,
                WIFEXITED(stat), WEXITSTATUS(stat), WIFSIGNALED(stat), WTERMSIG(stat));

            // If execv exits, we don't have an easy way to send the results back other
            // than some extra socket.  Instead we should try catching all exceptions
            // (file not found, file not executable) before forking.  Kill thyself.
            kill(getpid(), SIGKILL);
        }
        _sf1_close_child_files(&files);

        pid_chain = _sf1_pid_chain_add(pid_chain, pid);
        if (!pid_chain) {
            fprintf(stderr, "systemf: pid_chain out of memory");
            goto exit_error;
        }

        // Only wait for completion if this is not piped.  
        // I.E. "cat | grep" should run the grep before waiting for the cat to complete.
        if (files.out_rd_pipe == 0) {
            if (_sf1_pid_chain_waitpids(pid_chain, &stat, 0) == 0) {
                // FIXME: Make sure this is the right return value and better recover from this.
                fprintf(stderr, "waitpid unexpectedly returned %s", strerror(errno));
                goto exit_error;
            }
            _sf1_pid_chain_clear(pid_chain);

            retval = WEXITSTATUS(stat);

            if (WIFSIGNALED(stat)) {
                fprintf(stderr, "waipid exited with signal %s\n", strsignal(WTERMSIG(stat)));
                goto exit_error;
            }

            DBG("waitpid returned with %3d %3d %3d %3d %3d\n", errno,
                WIFEXITED(stat), WEXITSTATUS(stat), WIFSIGNALED(stat), WTERMSIG(stat));

            if (WIFSIGNALED(stat) || (WIFEXITED(stat) && WEXITSTATUS(stat))) {
                if (task->next && (task->next->run_if == _SF1_RUN_IF_PREV_SUCCEEDED)) {
                    DBG("exiting because previous failed");
                    break;
                }
            } else {
                if (task->next && (task->next->run_if == _SF1_RUN_IF_PREV_FAILED)) {
                    DBG("exiting because previous succeeded");
                    break;
                }
            }
        }
    }
    if (0) {
        exit_error:
        retval = -1;
    }

    // Clean up everything locally created.
    _sf1_close_child_files(&files);
    _sf1_pid_chain_free(pid_chain);
    
    return retval;
}

