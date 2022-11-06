#include "labs/fiber_scheduler.h"

//
// stackptrs:      Type: addr_t[stackptrs_size].  array of stack pointers (generalizing: main_stack and f_stack)
// stackptrs_size: number of elements in 'stacks'.
//
// arrays:      Type: uint8_t [arrays_size]. array of memory region for stacks (generalizing: f_array)
// arrays_size: size of 'arrays'. equal to stackptrs_size*STACK_SIZE.
//
// Tip: divide arrays into stackptrs_size parts.
// Tip: you may implement a circular buffer using arrays inside shellstate_t
//      if you choose linked lists, create linked linked using arrays in
//      shellstate_t. (use array indexes as next pointers)
// Note: malloc/new is not available at this point.
//
void fact(addr_t& pmain_stack, addr_t& pf_stack, shellstate_t* pstate, preempt_t& ppreempt) {

//does not yield
    addr_t& main_stack = pmain_stack; // boilerplate: to ease the transition from existing code
    addr_t& f_stack    = pf_stack;
    preempt_t& preempt = ppreempt;

    shellstate_t* state = pstate;
    int i = 1;
    state->schedule_return[state->schedule_pointer] = 1;
    for(; i <= state->schedule_argument[state->schedule_pointer]; i++) {
        state->schedule_return[state->schedule_pointer] *= i;
        state->schedule_return[state->schedule_pointer] %= 10000007;
        state->schedule_status[state->schedule_pointer] = 2;
        preempt.yield_started = 1;
        stack_saverestore(f_stack,main_stack);
    }
    
    preempt.yield_started = 1;
    state->schedule_status[state->schedule_pointer] = 0;
    stack_saverestore(f_stack,main_stack);
}

int fib_rec(int n) {
    if(n <= 1) {
        return n;
    }
    return (fib_rec(n-1) + fib_rec(n-2))%10000007;
}

void fib(addr_t& pmain_stack, addr_t& pf_stack, shellstate_t* pstate, preempt_t& ppreempt) {
    addr_t& main_stack = pmain_stack; // boilerplate: to ease the transition from existing code
    addr_t& f_stack    = pf_stack;
    preempt_t& preempt = ppreempt;

    shellstate_t* state = pstate;
    state->schedule_return[state->schedule_pointer] = fib_rec(state->schedule_argument[state->schedule_pointer]);
    preempt.yield_started = 1;
    state->schedule_status[state->schedule_pointer] = 0;
    stack_saverestore(f_stack,main_stack);
}

//apps.shell_state, main_stack, preempt, apps.stackptrs, apps.stackptrs_size, apps.arrays, apps.arrays_size, lapic
void shell_step_fiber_scheduler(shellstate_t& shellstate, addr_t& main_stack, preempt_t& preempt, addr_t stackptrs[], size_t stackptrs_size, addr_t arrays, size_t arrays_size, dev_lapic_t& lapic) {

    shellstate_t & state = shellstate;
    bool some = false;
    int temp = state.schedule_pointer + 1;
    int sum = 0;
    for(int i=0;i<5;i++){
        sum += state.schedule_status[i];
    }
    if(sum == 0) return;
    for(int i=temp;i<temp+5;i++){
        if(state.schedule_status[i%5] > 0) {
            some = true;
            int x = i%5;
            state.schedule_pointer = x;
            if(state.schedule_status[x] == 1) {
                state.schedule_status[x] = 2;
                if(state.schedule_type[x] == 3) {
                    stack_init4(stackptrs[x+1], addr_t(arrays)+arrays_size*(x+1)/10, arrays_size/10, &fact, &main_stack, &stackptrs[x+1], &state, &preempt);
                }
                else {
                    stack_init4(stackptrs[x+1], addr_t(arrays)+arrays_size*(x+1)/10, arrays_size/10, &fib, &main_stack, &stackptrs[x+1], &state,  &preempt);
                }
            }
            else {
                lapic.reset_timer_count(100000000); //start the timer to fire an interrupt
                stack_saverestore(main_stack, stackptrs[x+1]);
                lapic.reset_timer_count(0); //stop the timer
                preempt.yield_started = 0;
                if(preempt.preempted == 1) {
                    stackptrs[x+1] = preempt.saved_stack;
                    preempt.preempted = 0;
                }
                if(state.schedule_status[state.schedule_pointer] == 0) {
                    if(state.curr_row < 24)
                        state.curr_row++;
                    else {
                        state.curr_col = 0;
                        for(int i = 2; i < 24; i++) {
                            for(int j = 0; j < 80; j++) {
                                state.c[i][j] = state.c[i+1][j];
                                state.is_text[i][j] = state.is_text[i+1][j];
                            }
                        }
                        for(int j = 0; j < 80; j++) {
                            state.c[24][j] = ' ';
                            state.is_text[24][j] = false;
                        }
                    }
                    char temp[50];
                    int counter = 0;
                    int temp2 = state.schedule_return[state.schedule_pointer];
                    if(temp2 == 0) temp[counter++] = '0';
                    while(temp2 != 0) {
                        temp[counter++] = (temp2%10)+'0';
                        temp2 /= 10;
			        }
                    temp[counter++] = ':';
                    temp2 = state.schedule_argument[state.schedule_pointer];
                    if(temp2 == 0) temp[counter++] = '0';
                    while(temp2 != 0) {
                        temp[counter++] = (temp2%10)+'0';
                        temp2 /= 10;
			        }
                    if(state.schedule_type[state.schedule_pointer] == 3) {
                        temp[counter++] = 't'; temp[counter++] = 'c'; temp[counter++] = 'a'; temp[counter++] = 'f';
                    } else {
                        temp[counter++] = 'b'; temp[counter++] = 'i'; temp[counter++] = 'f';
                    }
                    for(int i = counter-1; i >= 0; i--) {
                        state.c[state.curr_row][counter-i-1] = temp[i];
                        state.is_text[state.curr_row][counter-i-1] = true;
                    }
                    // Setup kernel for next command
                    if(state.curr_row < 24)
                        state.start_row = ++state.curr_row;
                    else {
                        state.curr_col = 0;
                        state.start_row = state.curr_row;
                        for(int i = 2; i < 24; i++) {
                            for(int j = 0; j < 80; j++) {
                                state.c[i][j] = state.c[i+1][j];
                                state.is_text[i][j] = state.is_text[i+1][j];
                            }
                        }
                        for(int j = 0; j < 80; j++) {
                            state.c[24][j] = ' ';
                            state.is_text[24][j] = false;
                        }
                    }
                    char prompt[] = "hoh-user$ ";
                    for(int i = 0; i < 10; i++) {
                        state.c[state.curr_row][i] = prompt[i];
                        state.is_text[state.curr_row][i] = true;
                    }
                    state.curr_col = 10;
                }
            }
            break;
        }
    }

}
