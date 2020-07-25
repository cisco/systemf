#include <assert.h>

#include "systemf-internal.h"

/*
 * _sf1_pid_chain_wait - Waits for all processes in a chain of pids to finish.
 * 
 * stat_loc - The stat of the last pid in the pid_chain.
 * 
 * returns - The pid of the last pid in the chain or 0 on error.
 */
pid_t _sf1_pid_chain_waitpids(_sf1_pid_chain_t *pid_chain, int *stat_loc, int options) {
    // Currently this is always used in a run-to-completion, so don't support all waitpid options
    assert(options == 0);
    assert(stat_loc != NULL);
    pid_t pid;
    
    for (int i = 0; i < pid_chain->size; i++) {
        pid = waitpid(pid_chain->pids[i], stat_loc, 0);
        // This should never fail.
        assert(pid == pid_chain->pids[i]);
    }
    return pid;
}

size_t _sf1_pid_chain_size(int capacity) {
    return sizeof(_sf1_pid_chain_t) + sizeof(pid_t) * capacity;
}

/*
 * Adds a pid to the pid_chain.  If pid_chain is NULL, it allocates.  If needs to be increased,
 * it reallocates.  On failure, errno is set and NULL is returned.
 */
_sf1_pid_chain_t *_sf1_pid_chain_add(_sf1_pid_chain_t *pid_chain, pid_t pid) {
    const int cap_steps = 4;

    if (!pid_chain) {
        pid_chain = malloc(_sf1_pid_chain_size(cap_steps));
        if (!pid_chain) {
            return NULL;
        }
        pid_chain->capacity = cap_steps;
        pid_chain->size = 0;
    }

    if (pid_chain->capacity == pid_chain->size) {
        _sf1_pid_chain_t *save = pid_chain;

        pid_chain->capacity += cap_steps;
        pid_chain = realloc(pid_chain, _sf1_pid_chain_size(pid_chain->capacity));
        if (!pid_chain) {
            free(save);
            return NULL;
        }
    }

    pid_chain->pids[pid_chain->size] = pid;
    pid_chain->size += 1;
    return pid_chain;
}

/*
 * Removes memory for the pid_chain.  Always returns NULL.
 */
void _sf1_pid_chain_free(_sf1_pid_chain_t *pid_chain) {
    free(pid_chain);
}

/*
 * Resets the pid_chain to zero length.
 */
void _sf1_pid_chain_clear(_sf1_pid_chain_t *pid_chain) {
    pid_chain->size = 0;
}
