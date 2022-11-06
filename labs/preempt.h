#pragma once

#include "util/config.h"


//
// preempt_t : State for your timer/preemption handler
//
// Note:
//  We've one instance of core_t per each core.
//  To access this instance, you need to use %gs:0
//  (The entire kernel doesn't have any global/static variables)
//
// %gs:core_offset_preempt will point to start of preempt_t instance
//
// for example: 
// %gs:0 will return pointer to core_t
// %gs:core_offset_mainstack will return core_t::main_stack
// %gs:core_offset_preempt+0 will return core_t::saved_stack
//
// etc.
//
// See Definition of core_t in x86/main.h
//

struct preempt_t{
  // your data structure, if any
  addr_t saved_stack; //feel free to change it - provided as an example
  int yield_started = 0; // To handle data race condition
  int preempted = 0; // To check if fibre returned was preempted or not
};


//
// 
// This macro is being called from x86/except.cc
//
//
// _name: label name
// _f   : C function to be called 
//        ex: we may have to do send EOI to LAPIC or PIC etc.
//
#  define  _ring0_preempt(_name,_f)            \
  asm volatile(                                       \
      "  .text                            \n\t"\
      " " STR(_name) ":                   \n\t"\
      "  pushl %edx                       \n\t"\
      "  pushl %ecx                       \n\t"\
      "  pushl %eax                       \n\t"\
      "  call " STR(_f) "                 \n\t"\
      "  popl  %eax                       \n\t"\
      "  popl  %ecx                       \n\t"\
      "  popl  %edx                       \n\t"\
      "                                   \n\t"\
      "  # insert your code here          \n\t"\
      "  cmp $1, %gs:24+4  \n\t"\
      "  je iret_toring0 \n\t"\
      "  pushl %eax                       \n\t"\
      "  pushl %ecx                       \n\t"\
      "  pushl %edx                       \n\t"\
      "  pushl %ebx                       \n\t"\
      "  pushl %esp                       \n\t"\
      "  pushl %ebp                       \n\t"\
      "  pushl %esi                       \n\t"\
      "  pushl %edi                       \n\t"\  
      "  pushl %eax                       \n\t"\
      "  pushl %ecx                       \n\t"\
      "  movl %esp, %ebp                  \n\t"\
      "  subl $512, %esp                  \n\t"\
      "  andl $0xfffffff0, %esp           \n\t"\
      "  fxsave (%esp)                    \n\t"\
      "  pushl %ebp                       \n\t"\
      "  pushl $1f                        \n\t"\
      "  sti                              \n\t"\
      "  movl  $1,%gs:24+8                \n\t"\
      "                                   \n\t"\
      "  movl  %esp,%gs:24+0              \n\t"\
      "  movl  %gs:8,%esp                 \n\t"\
      "                                   \n\t"\
      "  ret                              \n\t"\
      "1:                                 \n\t"\
      "  popl %ebp                        \n\t"\
      "  fxrstor (%esp)                   \n\t"\
      "  movl %ebp, %esp                  \n\t"\
      "  popl %ecx                        \n\t"\
      "  popl %eax                        \n\t"\
      "  popl %edi                        \n\t"\
      "  popl %esi                        \n\t"\
      "  popl %ebp                        \n\t"\
      "  popl %esp                        \n\t"\
      "  popl %ebx                        \n\t"\
      "  popl %edx                        \n\t"\
      "  popl %ecx                        \n\t"\
      "  popl %eax                        \n\t"\
      "  jmp iret_toring0                 \n\t"\
        )