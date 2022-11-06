#include "labs/fiber.h"

void long_computation(addr_t* pmain_stack, addr_t* pf_stack, int* pfibre_status) {

    addr_t& main_stack = *pmain_stack; // boilerplate: to ease the transition from existing code
    addr_t& f_stack    = *pf_stack;

    int& fibre_status = *pfibre_status;

    int i = 0;

    for(; i < 100000; i++) {
        hoh_debug("Performing Long Computation (fiber): Iteration " << i);   // save this 'i' in stack somehow 
        fibre_status = 2;
        stack_saverestore(f_stack,main_stack);
    }
    
    fibre_status = 0;
    stack_saverestore(f_stack,main_stack);
}

//apps.shell_state, main_stack, preempt, apps.f_stack, apps.f_array, apps.f_arraysize, lapic
void shell_step_fiber(shellstate_t& shellstate, addr_t& main_stack,preempt_t& preempt, addr_t& f_stack, addr_t f_array, uint32_t f_arraysize, dev_lapic_t& lapic){

  //insert your code here
  if(shellstate.fibre_status == 0) {
      return;
  } else if(shellstate.fibre_status == 1) {
      shellstate.fibre_status = 2;
      stack_init3(f_stack, f_array, f_arraysize, &long_computation, &main_stack, &f_stack, &shellstate.fibre_status);
      
      

  } else {  
      stack_saverestore(main_stack,f_stack);
      if(shellstate.fibre_status == 0) {
      }
  }

}

