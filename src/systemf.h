#ifndef __systemf_h__
#define __systemf_h__
#include <stdio.h>

extern int systemf1(const char *fmt, ...);

/*
 * Debug Flags used with the global systemf1_debug_set() and systemf1_debug_get()
 * Flags starting with SYSTEMF1_DBG_DBG_ only work if systemf is configured with --enhanced-debug
 */
enum {
 SYSTEMF1_DBG_ERRORS = 0x01,  // Print a debug any time a nonzero value would be returned.
 SYSTEMF1_DBG_EXEC = 0x02,    // Print the command and arguments being launched.
 SYSTEMF1_DBG_DBG_LEX = 0x04,
 SYSTEMF1_DBG_DBG_PARSER = 0x08,
};
extern int systemf1_debug_set(int flags);
extern int systemf1_debug_get();
extern FILE *systemf1_debug_file_set(FILE *file);
extern FILE *systemf1_debug_file_get();

#endif /* __systemf_h__ */
