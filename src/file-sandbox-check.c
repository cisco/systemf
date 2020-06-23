#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

/*
 * The dir structure is used in walking directories.  For these two cases,
 * these map to directory tuples: {start, end, depth}:
 * char *a = "/job/one" {&a[0], &a[1], 1} {&a[1], &a[5], 2} {&a[5], &a[8], 3} {&a[8], &a[8], 4}}
 * char *b = "job/one"  {&a[0], &a[4], 1} {&a[4], &a[7], 2} {&a[7], &a[7], 3}
 * char *c = "/job/"    {&a[0], &a[1], 1} {&a[1], &a[5], 2} {&a[5], &a[5], 3}
 * 
 * In addition there is a zero {NULL, &a[0], 0} and and EOF {e, e, end+1) where e=&a[strlen(a))
 */
struct dir {
    char *buffer;
    char *start;
    char *end;
    int depth;
};

static int is_dot_dot(struct dir *path) {
    const char *s = path->start;
    return ((s[0] == '.') && (s[1] == '.') && ((s[2] == '/') || (!s[2])));
}

static int is_one_dot(struct dir *path) {
    const char *s = path->start;
    return ((s[0] == '.') && ((s[1] == '/') || (!s[1])));
}

static struct dir zero_dir(char *buffer) {
    struct dir dir;
    dir.buffer = buffer;
    dir.start = NULL;
    dir.end = buffer;
    dir.depth = 0;
    return dir;
}

static int is_zero_dir(struct dir *dir) {
    return dir->start == NULL;
}

static int is_first_dir(struct dir *dir) {
    return (dir->start == dir->buffer);
}

static int eof_dir(struct dir *dir) {
    // Have we even started processing and if so, are we at the end.
    return (dir->start != NULL) && !dir->start[0];
}

static int next_dir(struct dir *dir) {
    // We are already at the end.
    if (eof_dir(dir)) {
        return 0;
    }

    // Special case.  A leading '/' is a dir all by itself.
    if (is_zero_dir(dir) && dir->buffer[0] == '/') {
        dir->start = dir->buffer;
        dir->end = dir->buffer;
        dir->depth = 1;
        return 1;
    }

    // Walk over all leading '/'
    dir->start = dir->end + strspn(dir->end, "/");

    // Walk over non '/'
    dir->end = dir->start + strcspn(dir->start, "/");

    // Include the trailing '/' if it is present
    if (dir->end[0]) {
        dir->end += 1;
    }

    if (is_dot_dot(dir)) {
        dir->depth -= 1;
    } else if (!is_one_dot(dir)) {
        dir->depth += 1;
    }

    return 1;
}

static int prev_dir(struct dir *dir) {
    if (is_zero_dir(dir)) {
        return 0;
    }

    if (is_first_dir(dir)) {
        *dir = zero_dir(dir->buffer);
        return 1;
    }

    if (dir->start - 1 == dir->buffer) {
        // Special case where previous is the root '/'
        dir->start = dir->buffer;
        dir->end = &dir->buffer[1];
        return 1;
    }
    
    // Move dir->end and then walk over any doubled "//"
    for (dir->end = dir->start; dir->end[-1] == '/' && dir->end[-2] == '/'; dir->end -= 1);
 
    // Walk dir->start back 
    for (dir->start = dir->end - 1; 
        (dir->start[-1] != '/') && (dir->start != dir->buffer);
        dir->start--);

    if (is_dot_dot(dir)) {
        dir->depth += 1;
    } else if (!is_one_dot(dir)) {
        dir->depth -=1;
    }
    return 1;
}

static size_t path_copy(char *dest, struct dir *source) {
    size_t slen = source->end - source->start;
    memcpy(dest, source->start, slen);
    dest[slen] = 0;
    return slen;
}

static char *extract_simplified_path(char *path) {
    char *simple_path;
    struct dir simple_dir;
    struct dir cursor;

    simple_path = malloc(strlen(path)+1);
    if (!simple_path) {
        return NULL;
    }
    simple_dir = zero_dir(simple_path);
    
    cursor = zero_dir(path);
    while (next_dir(&cursor)) {
        if (is_one_dot(&cursor)) {
            continue;
        }
        if (is_dot_dot(&cursor) && !is_zero_dir(&simple_dir) && !is_dot_dot(&simple_dir)) {
            // This destroys the previous dir.
            prev_dir(&simple_dir);
            simple_dir.end[0] = 0;
            continue;
        }

        // Copy the data and move forward.
        path_copy(simple_dir.end, &cursor);
        next_dir(&simple_dir);
    }
    return (simple_path);
}

/*
 * simple_sandbox_check assumes that the trusted_path is the preamble
 * to the path (that is how systemf works) and it never has enough .. to
 * go into the trusted path".
 */
static int simple_sandbox_check(char *trusted_path, char *path) {
    char *untrusted_path = path + strlen(trusted_path);
    struct dir up = zero_dir(untrusted_path);

    while (next_dir(&up)) {
        if (up.depth < 0) {
            return(EACCES);
        }
    }
    return 0;
}

/*
 * complex_sandbox_check converts the paths to simplified paths
 * and then compares those resulting paths.
 */
static int complex_sandbox_check(char *trusted_path, char *path) {
    char *tp = extract_simplified_path(trusted_path);
    char *p = extract_simplified_path(path);
    int retval;

    if (p && tp) {
        size_t slen = strlen(tp) - 1;
        assert(tp[slen] == '/');

        if (strncmp(p, tp, slen)) {
            // No match even before the trailing slash.
            retval = EACCES;
        } else if (p[slen] && p[slen] != '/') {
            // EG: tp = /hope/ and p = /hopeless
            retval = EACCES;
        } else {
            retval = 0;
        }
    } else {
        retval = ENOMEM;
    }
    free(tp);
    free(p);

    return retval;
}

/*
 * Checks if the path is under the trusted_path.
 * Returns 0 on good and EACCES on bad or ENOMEM if unable to make check.
 */
int _sf1_file_sandbox_check(char *trusted_path, char *path) {
    int retval;

    if (0 == simple_sandbox_check(trusted_path, path)) {
        retval = 0;
    } else if (!trusted_path[0]) {
        // Fail: Trusted is current directory and simple check was EACCES.
        retval = EACCES;
    } else {
        retval = complex_sandbox_check(trusted_path, path);
    }

    if (retval) {
        fprintf(stderr, "systemf: %s: sandboxing %s\n", strerror(retval), path);
    }

    return retval;
}