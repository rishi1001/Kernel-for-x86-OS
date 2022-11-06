#include "labs/coroutine.h"

void long_computation(coroutine_t* pf_coro,f_t* pf_locals, int* plong_status) {
    coroutine_t& f_coro = *pf_coro; // boilerplate: to ease the transition from existing code
    int& long_status = *plong_status;
    int& i = pf_locals->i;

    h_begin(f_coro);

    for(i = 0; i < 100000; i++) {
        hoh_debug("Performing Long Computation (coroutine): Iteration " << i);
        h_yield(f_coro);
    }
    long_status = 0; h_end(f_coro);
}


void shell_step_coroutine(shellstate_t& shellstate, coroutine_t& f_coro, f_t& f_locals){

    //insert your code here
    if(shellstate.long_status == 0) {
        return;
    } else if(shellstate.long_status == 1) {
        coroutine_reset(f_coro);
        f_locals.i = 0;
        shellstate.long_status = 2;
    } else {
        long_computation(&f_coro, &f_locals, &shellstate.long_status);
        if(shellstate.long_status == 0) {
        }
    }

}


