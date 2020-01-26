.globl timer_handler_wrapper

timer_handler_wrapper:
    pusha
    call timer_handler
    popa
    iret