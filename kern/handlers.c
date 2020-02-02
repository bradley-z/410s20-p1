/** @file handlers.c
 *  @brief handler_install implementation
 *
 *  Implementation for the handler_install function, which installs interrupt
 *  handlers for both the timer interrupt and keyboard interrupt.
 *
 *  @author Bradley Zhou (bradleyz)
 *  @bug No known bugs.
 */
#include <p1kern.h>
#include <stddef.h>             /* NULL */
#include <stdint.h>             /* uint32_t, uint64_t */
#include <stdbool.h>            /* bool */
#include <asm.h>                /* inb, outb */
#include <timer_defines.h>      /* TIMER_IDT_ENTRY */
#include <interrupt_defines.h>  /* INT_CTL_PORT, INT_ACK_CURRENT */
#include <idt.h>                /* IDT_USER_START, IDT_ENTS */
#include <seg.h>                /* SEGSEL_TSS, SEGSEL_KERNEL_CS */
#include <keyhelp.h>            /* KEY_IDT_ENTRY, KEYBOARD_PORT */

#include <handlers_asm.h>
#include <timer.h>              /* timer_t, timer_initialize, timer_tick */
#include <kb_buffer.h>          /* kb_buffer, kb_buf_initialize, kb_buf_write */

/* size of all interrupt gates in bytes */
#define GATE_SIZE       8

/* present bit in the top 32 bits of the interrupt gate */
#define PRESENT_MASK    0x8000
/* dpl starts at index 13 in the top 32 bits of the interrupt gate */
#define DPL_SHIFT       13
/* size bit in the top 32 bits of the interrupt gates */
#define SIZE_MASK       0x800
/* top 16 bits of a word */
#define TOP_HALF_MASK   0xFFFF0000
/* lower 16 bits of a word */
#define LOWER_HALF_MASK 0xFFFF
/* segment selector startsa at index 16 in bottom 32 bits of interrupt gate */
#define SEGSEL_SHIFT    16

/* mask for bits 8, 9, 10 in the top 32 bits of the interrupt gate */
typedef enum {
    TASK = 0x500,
    INTERRUPT = 0x600,
    TRAP = 0x700,
} gate_t;

/* global timer struct that keeps track of ticks and callback */
timer_t timer;
/* keyboard buffer defined in kb.c */
extern kb_buf_t kb_buffer;

/** @brief takes all information found in interrupt gate and packs into 64 bits
 *
 *  All the information that describes an interrupt gate is passed as
 *  parameters to this function, which then does the appropriate shifting and
 *  masking to put each piece of information in its correct bits.
 *
 *  @param gate_type type of gate
 *  @param dpl descriptor privilege level
 *  @param offset offest in bytes from the base of interrupt descriptor table
 *  @param present present bit in the interrupt gate
 *  @param seg_sel segment selector as defined in seg.h
 *  @param gate_size whether or not gate is 32 bits
 *  @return 64 bit representation of interrupt gate
 */
static uint64_t idt_entry_pack(gate_t gate_type, uint32_t dpl, uint32_t offset,
                               bool present, uint32_t seg_sel, bool gate_size)
{
    uint32_t top_half = 0;
    uint32_t bottom_half = 0;
    if (gate_type == TASK) {
        if (present) {
            top_half |= PRESENT_MASK;
        }
        top_half |= dpl << DPL_SHIFT;
        top_half |= TASK;

        bottom_half |= SEGSEL_TSS << SEGSEL_SHIFT;
    }
    else {
        top_half |= offset & TOP_HALF_MASK;
        if (present) {
            top_half |= PRESENT_MASK;
        }
        if (gate_size) {
            top_half |= SIZE_MASK;
        }
        top_half |= dpl << DPL_SHIFT;
        top_half |= gate_type;

        bottom_half = seg_sel << SEGSEL_SHIFT | (offset & LOWER_HALF_MASK);
    }

    return (uint64_t)top_half << 32 | (uint64_t)bottom_half;
}

/** @brief installs an interrupt handler in the table in kernel mode
 *
 *  If the idt entry index is out of range, return false. If handler is NULL,
 *  return false. This function packs the gate then stores the interrupt gate
 *  at the correct location in the idt.
 *
 *  @param base_addr starting address of the interrupt descriptor table
 *  @param idt_entry index in the idt to install interrupt at
 *  @param handler function to be invoked upon interrupt being received
 *  @return whether or not the handler could be validly installed
 */
static bool install_idt_km(void *base_addr,
                           unsigned int idt_entry,
                           void *handler)
{
    if (idt_entry < IDT_USER_START || idt_entry >= IDT_ENTS) {
        return false;
    }
    if (handler == NULL) {
        return false;
    }
    uint32_t offset = idt_entry * GATE_SIZE;
    uint64_t *idt_entry_addr = (uint64_t*)((char*)base_addr + offset);
    uint64_t packed_gate = idt_entry_pack(TRAP, 0, (uint32_t)handler,
                                          true, SEGSEL_KERNEL_CS, true);
    *idt_entry_addr = packed_gate;

    return true;
}

/** @brief C timer handler function
 *
 *  This is the handler function called by the assembly wrapper function that
 *  is invoked upon receiving an interrupt.
 *
 *  @return Void.
 */
void timer_handler()
{
    timer_tick(&timer);
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}

/** @brief C timer handler function
 *
 *  This is the handler function called by the assembly wrapper function that
 *  is invoked upon receiving an interrupt.
 *
 *  We call inb() before we check if the buffer is full because dropping
 *  keypresses is preferable to blocking additional keypresses.
 *
 *  @return Void.
 */
void kb_handler()
{
    int keypress = inb(KEYBOARD_PORT);
    if (!kb_buf_write(&kb_buffer, keypress)) {
        /* nothing to do if write fails, we can just return */
    }
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}

int handler_install(void (*tickback)(unsigned int))
{
    timer_initialize(&timer, tickback);
    kb_buf_initialize(&kb_buffer);

    void *base_addr = idt_base();

    if (!install_idt_km(base_addr, TIMER_IDT_ENTRY, timer_handler_wrapper)) {
        return -1;
    }
    if (!install_idt_km(base_addr, KEY_IDT_ENTRY, kb_handler_wrapper)) {
        return -1;
    }

    return 0;
}
