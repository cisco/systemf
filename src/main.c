#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "systemf.h"

int main() {
    extern int systemf1_yydebug;
    extern int errno;
    int stat;
    systemf1_yydebug = 0;
    errno = 0;
    //                 123456789_123456789_123456789_12345
    stat = systemf1("/bin/echo delete-this-%p.txt && /usr/bin/true && echo unneeded || echo needed", "3");
    printf("systemf returned with %3d %3d %3d %3d %3d\n", errno,
        WIFEXITED(stat), WEXITSTATUS(stat), WIFSIGNALED(stat), WTERMSIG(stat));
    return 0;
}
