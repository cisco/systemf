#include <sys/resource.h>
#include "systemf-internal.h"
#include <errno.h>
#include <unistd.h>
#include <assert.h>

/*
 * Closes all other files but stdin, stdout, and stderr.
 * 
 * All documentation advises that all open files should be closed
 * after a fork.  But there is no consistent advice on how to close
 * these files.  It is expected that this implementation will become
 * several different versions detecting on what operating system 
 * ./configure detects.
 * 
 * This does not close fd 0, 1, and 2 (stdin, stdout, and stderr)
 */
void _sf1_close_upper_fd() {
    struct rlimit rlim;
    int prev_errno;

    // if getrlimit ever fails, we need to figure out why and then properly handle it.
    assert(!getrlimit(RLIMIT_NOFILE, &rlim));

    prev_errno = errno;
    for (int i = 3; i < rlim.rlim_cur; i++) {
        close(i);
    }
    errno = prev_errno;
}
