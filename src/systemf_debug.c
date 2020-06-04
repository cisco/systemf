#include <stdarg.h>
#include <stdio.h>

static int debug_flags = 0;
static FILE *debug_file = NULL;

int systemf1_debug_set(int flags)
{
    debug_flags = flags;
    return debug_flags;
}

int systemf1_debug_get()
{
    return debug_flags;
}

FILE *systemf1_debug_file_set(FILE *file)
{
    debug_file = file;
    return debug_file;
}

FILE *systemf1_debug_file_get()
{
    // Initialize to dynamic stderr (which can't be done compile-time)
    if (debug_file == NULL) {
        debug_file = stderr;
    }
    return debug_file;
}
