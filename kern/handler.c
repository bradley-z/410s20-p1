#include <p1kern.h>
#include <stdio.h>

int handler_install(void (*tickback)(unsigned int))
{
    // get idt_base
    // compute offset using TIMER_IDT_ENTRY in timer_defines.h
    // format of trap gate is in 151 of intel-sys.pdf, create those two words
    // store at idt
    // set some timer mmio shit
  return -1;
}