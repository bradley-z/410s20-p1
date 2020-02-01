#include <p1kern.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <asm.h>
#include <timer_defines.h>
#include <interrupt_defines.h>
#include <idt.h>
#include <seg.h>
#include <keyhelp.h>

#include <handlers_asm.h>
#include <timer.h>
#include <kb_buffer.h>

#define GATE_SIZE       8

#define PRESENT_MASK    0x8000
#define DPL_SHIFT       13
#define SIZE_MASK       0x800
#define TOP_HALF_MASK   0xFFFF0000
#define LOWER_HALF_MASK 0xFFFF
#define SEGSEL_SHIFT    16

typedef enum {
    TASK = 0x500,
    INTERRUPT = 0x600,
    TRAP = 0x700,
} gate_t;

timer_t timer;
extern kb_buf_t kb_buffer;

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

void timer_handler()
{
    timer_tick(&timer);
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}

// circ bufs are inherently thread safe with one reader/writer and no other
// interrupts touch the kb_buffer
void kb_handler()
{
    // design choice, if the buf is full, do i want to drop before or after inb?
    int keypress = inb(KEYBOARD_PORT);
    if (!kb_buf_write(&kb_buffer, keypress)) {
        // no clue what to do if it's full
    }
    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}

// TODO: in what case would handler_install error?
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
