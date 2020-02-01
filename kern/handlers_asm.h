/** @file handlers_asm.h
 *  @brief function prototypes for asm interrupt handler wrapper functions
 *
 *  This file contains the function prototype declarations for asm wrapper
 *  functions that call C code as interrupt handlers. There are prototypes
 *  for the timer interrupt handler wrapper and the keyboard interrupt handler
 *  wrapper.
 *
 *  @author Bradley Zhou (bradleyz)
 *  @bug No known bugs.
 */
#ifndef __HANDLERS_ASM_H_
#define __HANDLERS_ASM_H_

/** @brief Wrapper function to setup stack and call C timer_handler function
 * 
 *  Since our timer_handler will be clobbering the values of the general purpose
 *  registers, we need to save those on the stack before we start executing our
 *  timer_handler code. Those registers are saved with the pusha instruction,
 *  restored with the popa instruction after the C function returns, and then
 *  the iret instruction is called to return back to where interrupt occurred.
 * 
 *  @return Void.
 */
void timer_handler_wrapper(void);
void kb_handler_wrapper(void);

/** @brief Wrapper function to setup stack and call C kb_handler function
 * 
 *  Since our kb_handler will be clobbering the values of the general purpose
 *  registers, we need to save those on the stack before we start executing our
 *  kb_handler code. Those registers are saved with the pusha instruction,
 *  restored with the popa instruction after the C function returns, and then
 *  the iret instruction is called to return back to where interrupt occurred.
 * 
 *  @return Void.
 */
#endif /* __HANDLERS_ASM_H_ */
