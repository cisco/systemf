#include <sys/resource.h>
#include "systemf_internal.h"

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

    if (getrlimit(RLIMIT_NOFILE, &rlim);) {
        // FIXME: Should we raise an exception here?
        return;
    }

    prev_errno = errno;
    for (int i = 3; i < rl.rlim_cur; i++) {
        close(fd);
    }
    errno = prev_errno;
}
