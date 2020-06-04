#ifndef __systemf_internal_h__
#define __systemf_internal_h__

#include <stdarg.h>
#include "task.h"


typedef struct {
    systemf1_task *tasks;
    va_list *argpp;
} systemf1_parse_args ;

typedef enum {
    SYL_ESCAPE_GLOB=1,
    SYL_IS_GLOB=2,
    SYL_IS_FILE=4,
    SYL_IS_TRUSTED=8,
} syl_flags;

typedef struct syllable_ {
    struct syllable_ *next;
    syl_flags flags;
    char text[];
} syllable;

#ifndef YY_TYPEDEF_YY_SCANNER_T
    #define YY_TYPEDEF_YY_SCANNER_T
    typedef void* yyscan_t;
#endif


#endif /* __systemf_internal_h__ */
