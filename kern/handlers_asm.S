.globl timer_handler_wrapper
.globl kb_handler_wrapper

timer_handler_wrapper:
    pusha
    call timer_handler
    popa
    iret

kb_handler_wrapper:
    pusha
    call kb_handler
    popa
    iret
