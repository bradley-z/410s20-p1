#include <p1kern.h>
#include <asm.h>
#include <timer_defines.h>
#include <interrupt_defines.h>
#include <seg.h>
#include <stdint.h>
#include <simics.h>
#include <keyhelp.h>

#include <handlers_asm.h>
#include <kb.h>

#define GATE_SIZE 8

#define PRESENT_MASK 0x8000
#define GATE_MASK 0xF00 
#define DPL_OFFSET 13
#define SEGSEL_OFFSET 16
#define CYCLES_10_MS 11932

unsigned int numTicks = 0;
void (*timer_function)(unsigned int) = 0;
// TODO: is this even big enough? what if we press like three modifiers?
extern int keypresses[CIRCULAR_BUFFER_SIZE];
extern unsigned int read_index;
extern unsigned int write_index;

uint64_t pack(uint32_t dpl, uint32_t offset, uint32_t present, uint32_t seg_sel)
{
    uint32_t top_half = offset & 0xFFFF0000;
    if (present) {
        top_half |= PRESENT_MASK;
    }
    top_half |= (dpl << DPL_OFFSET);
    top_half |= GATE_MASK;
    uint32_t bottom_half = seg_sel << SEGSEL_OFFSET | (offset & 0xFFFF);
    return (uint64_t)top_half << 32 | (uint64_t)bottom_half;
}

void timer_handler()
{
    numTicks++;
    if (timer_function) {
        timer_function(numTicks);
    }

    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}

int readchar(void)
{
    int curr_scancode;
    kh_type aug_char;
    while (read_index != write_index) {
        curr_scancode = keypresses[read_index];
        aug_char = process_scancode(curr_scancode);

        read_index = (read_index + 1) % CIRCULAR_BUFFER_SIZE;

        if (KH_HASDATA(aug_char) && KH_ISMAKE(aug_char)) {
            return KH_GETCHAR(aug_char);
        }
    }
    return -1;
}

// TODO: make this threadsafe??? cas on the buffer?
void kb_handler()
{
    if ((read_index + 1) % 256 == write_index) {
        return;
    }
    // okay so we're not going to be prempted by another kb interrupt,
    // what happens if we get premepted by the timer? or readchar?

    int keypress = inb(KEYBOARD_PORT);

    keypresses[write_index] = keypress;

    write_index = (write_index + 1) % CIRCULAR_BUFFER_SIZE;

    outb(INT_CTL_PORT, INT_ACK_CURRENT);
}

// TODO: in what case would i error?
int handler_install(void (*tickback)(unsigned int))
{
    void *idt_base_addr = idt_base();
    uint32_t timer_offset = TIMER_IDT_ENTRY * GATE_SIZE;
    uint64_t *timer_idt_addr = (uint64_t*)((char*)idt_base_addr + timer_offset);

    numTicks = 0;
    timer_function = tickback;

    read_index = 0;
    write_index = 0;

    uint64_t timer_gate = pack(0, (uint32_t)timer_handler_wrapper, 1, SEGSEL_KERNEL_CS);
    *timer_idt_addr = timer_gate;

    outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
    uint8_t period_lsb = (uint8_t)(CYCLES_10_MS | 0xFF);
    uint8_t period_msb = (uint8_t)(CYCLES_10_MS >> 8 | 0xFF);
    outb(TIMER_PERIOD_IO_PORT, period_lsb);
    outb(TIMER_PERIOD_IO_PORT, period_msb);

    uint32_t kb_offset = KEY_IDT_ENTRY * GATE_SIZE;
    uint64_t *kb_idt_addr = (uint64_t*)((char*)idt_base_addr + kb_offset);

    uint64_t kb_gate = pack(0, (uint32_t)kb_handler_wrapper, 1, SEGSEL_KERNEL_CS);
    *kb_idt_addr = kb_gate;

    return 0;
}