.globl timer_handler_wrapper
.globl kb_handler_wrapper

timer_handler_wrapper:
    pusha               # save general purpose registers onto stack
    call timer_handler  # call the C interrupt handler code
    popa                # restore general purpose registers
    iret                # return from interrupt

kb_handler_wrapper:
    pusha               # save general purpose registers onto stack
    call kb_handler     # call the C interrupt handler code
    popa                # restore general purpose registers
    iret                # return from interrupt
