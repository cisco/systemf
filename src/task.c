#include "task.h"
#include <stdlib.h>
#include <unistd.h>
#include <glob.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <signal.h>

#define DEBUG 1
#define VA_ARGS(...) , ##__VA_ARGS__
#define DBG(fmt, ...) if (DEBUG) { printf("%s:%-3d:%24s: " fmt "\n", __FILE__, __LINE__, __FUNCTION__ VA_ARGS(__VA_ARGS__)); }

typedef struct glob_list_ {
    struct glob_list_ *next;
    glob_t glob;
} glob_list;

systemf1_task *systemf1_task_create() {
    return calloc(1, sizeof(systemf1_task));
}

/*
 * systemf1_task_add_redirects - Adds a linked of redirections.
 *
 * task - Destination Task - Current redirects must be NULL,
 * redirects - A linked list of systemf1_redirects where:
 *   each redirect is malloced
 *   filename - Is NULL or the destination malloced filename (if appropriate)
 *   path - Is the malloced verification path for filename where NULL is current directory.
 *   append - Specifies if the redirect shoud append to the file.
 *
 * Note that all malloced memory passed in will be freed with systemf1_task_free().
 * Note that validation of parameters is delayed until that task start happens.
 */
void systemf1_task_add_redirects (systemf1_task *task, systemf1_redirect *redirects)
{
    assert(task->redirects == NULL);
    task->redirects = redirects;
}

/*
 * systermf1_task_set_run_if - Sets the condition that this task will run.
 */
void systemf1_task_set_run_if (systemf1_task *task, systemf1_run_if run_if) {
    task->run_if = run_if;
}

/*
 * systemf1_task_add_arg - Adds the specified argument to the task.
 *
 * task - Destination Task
 * text - The text of the argument.  Must be malloced.
 * path - If path verification is to be done this is the pre-realpath path.  Either NULL or malloced.
 * is_glob - This is a glob expression that glob expansion will be needed for.
 * to_front - specifies the command should be added to the front of the argument list.
 *
 * Note that all malloced memory passed in will be freed with systemf1_task_free()
 */
systemf1_task_arg *systemf1_task_add_arg (systemf1_task *task, char *text, char *path, int is_glob, int to_front)
{
    systemf1_task_arg *arg;

    arg = malloc(sizeof(systemf1_task_arg));
    if (arg == NULL) {
        return NULL;
    }

    arg->text = text;
    arg->path = path;
    arg->is_glob = is_glob;

    // Find the last item and append pp
    if (to_front) {
        arg->next = task->args;
        task->args = arg;
    } else {
        systemf1_task_arg **next_pp = &(task->args);
        while (*next_pp) {
            next_pp = &((*next_pp)->next);
        }
        *next_pp = arg;
    }
    return arg;
}

/*
 * Walks all of the redirects and verifies that they are entered in a sane way.
 * Returns 0 if not and 1 if good.
 */
static int redirects_are_sane(systemf1_task *tasks)
{
    DBG("begin");
    for (systemf1_task *t = tasks; t; t = t->next) {
        int in = 0;
        int out = 0;
        int err = 0;
        for (systemf1_redirect *r = t->redirects; r; r = r ->next) {
            int count;
            switch (r->stream) {
            case SYSTEMF1_STDIN:
                DBG("");
                count = in = in + 1;
                break;
            case SYSTEMF1_STDOUT:
                DBG("");
                count = out = out + 1;
                break;
            case SYSTEMF1_STDERR:
                DBG("");
                count = err = err + 1;
                break;
            default:
                DBG("");
                // Only STDIN, STDOUT, and STDERR
                return 0;
            }
            if (count > 1) {
                // There should only be at most 1 STDIN, STDOUT, and STDERR
                return 0;
            }
            if ((r->target == SYSTEMF1_FILE) && !(r->text)) {
                // A filename must be supplied.
                return 0;
            }
        }
    }
    DBG("end");
    return 1;
}

/*
 * Extracts all the globs from the task arguments for every task.
 * tasks - The task list.
 * returns:
 *   0 on success
 *   GLOB_NOSPACE for running out of memory,
 *   GLOB_ABORTED for a read error, and
 *   GLOB_NOMATCH for when the number of matches doesn't match the specified allowed match count.
 */
static int extract_globs(systemf1_task *tasks)
{
    for (systemf1_task *t = tasks; t != NULL; t = t->next) {
        for (systemf1_task_arg *a = t->args; a != NULL; a = a->next) {
            if (a->is_glob) {
                int ret = glob(a->text, 0, NULL, &a->glob);
                // FIXME: do a bounds check.
                switch (ret) {
                case GLOB_NOSPACE:
                    return ENOMEM;
                case GLOB_ABORTED:
                    return EBADF;
                case GLOB_NOMATCH:
                    return EINVAL;
                }
            }
        }
    }
    return 0;
}

void systemf1_task_free(systemf1_task *task)
{
    systemf1_task *next;
    for (; task != NULL; task = next) {
        systemf1_task_arg *anext;
        for (systemf1_task_arg *a = task->args; a != NULL; a = anext) {
            globfree(&a->glob);
            free(a->text);
            free(a->path);
            anext = a->next;
            free(a);
        }

        systemf1_redirect *rnext;
        for (systemf1_redirect *r = task->redirects; r != NULL; r = rnext) {
            free(r->text);
            free(r->path);
            rnext = r->next;
            free(r);
        }

        free(task->argv);
        next = task->next;
        free(task);
    }
    return;
}

int systemf1_tasks_run(systemf1_task *tasks) {
    pid_t pid;
    int stat;
    char **argv;
    systemf1_task_arg *arg;
    size_t argc = 1;
    glob_list *globs = NULL;
    glob_list **next_glob_pp = &globs;
    int ret;
    int newerrno = 0;

    if (!redirects_are_sane(tasks)) {
        return -1;
    }

    ret = extract_globs(tasks);
    if (ret) {
        errno = ret;
        return -1;
    }

    // We don't support tasks reuse, so argv MUST be null coming into this.
    assert(tasks->argv == NULL);

    for (systemf1_task *task = tasks; task; task = task->next) {
        // Count the arguments.
        for (arg = task->args; arg != NULL; arg = arg->next) {
            if (arg->is_glob) {
                argc = arg->glob.gl_pathc;
            } else {
                argc += 1;
            }
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
        printf("_____________________ err exi exs sig tsig\n");

        // If we don't flush, both forks will send the buffered data and it will be seen twice.
        fflush(stdout);
        fflush(stderr);

        // FIXME: Determine if the file exists and it is executable before attempting
        // to fork which doesn't know how to handle results.

        newerrno = 0;
        pid = fork();
        if (pid == 0) {
            DBG("Running %s", task->argv[0]);
            stat = execv(*task->argv, task->argv);
            printf("Execv   returned with %3d %3d %3d %3d %3d\n", errno,
                WIFEXITED(stat), WEXITSTATUS(stat), WIFSIGNALED(stat), WTERMSIG(stat));

            // If execv exits, we don't have an easy way to send the results back other
            // than some extra socket.  Instead we should try catching all exceptions
            // (file not found, file not executable) before forking.  Kill thyself.
            kill(getpid(), SIGKILL);
        }

        waitpid(pid, &stat, 0);

        if (WIFSIGNALED(stat)) {
            newerrno = EINTR;
        }

        printf("waitpid returned with %3d %3d %3d %3d %3d\n", errno,
            WIFEXITED(stat), WEXITSTATUS(stat), WIFSIGNALED(stat), WTERMSIG(stat));


        if (WIFSIGNALED(stat) || (WIFEXITED(stat) && WEXITSTATUS(stat))) {
            if (task->next && (task->next->run_if == SYSTEMF1_RUN_IF_PREV_SUCCEEDED)) {
                DBG("exiting because previous failed");
                break;
            }
        } else {
            if (task->next && (task->next->run_if == SYSTEMF1_RUN_IF_PREV_FAILED)) {
                DBG("exiting because previous succeeded");
                break;
            }
        }
    }

    if (newerrno) {
        errno = newerrno;
    }
    return stat;
}

