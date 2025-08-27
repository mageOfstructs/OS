#include "idt.h"
#include <stdbool.h>
#include <stdint.h>

#define IDT_MAX_DESCRIPTORS 256
#define INT_TYPE_R0 0x8E
#define TRAP_TYPE_R0 0x8F

typedef struct {
  uint16_t isr_low;   // The lower 16 bits of the ISR's address
  uint16_t kernel_cs; // The GDT segment selector that the CPU will load into CS
                      // before calling the ISR
  uint8_t reserved;   // Set to zero
  uint8_t attributes; // Type and attributes; see the IDT page
  uint16_t isr_high;  // The higher 16 bits of the ISR's address
} __attribute__((packed)) idt_entry_t;

__attribute__((aligned(0x10))) static idt_entry_t
    idt[256]; // Create an array of IDT entries; aligned for performance

typedef struct {
  uint16_t limit;
  uint32_t base;
} __attribute__((packed)) idtr_t;

static idtr_t idtr;

// void idt_set_descriptor(uint8_t vector, void *isr, uint8_t flags);
void idt_set_descriptor(uint8_t vector, uint32_t isr, uint8_t flags) {
  idt_entry_t *descriptor = &idt[vector];

  descriptor->isr_low = (uint32_t)isr & 0xFFFF;
  descriptor->kernel_cs = 0x08; // this value can be whatever offset your kernel
                                // code selector is in your GDT
  descriptor->attributes = flags;
  descriptor->isr_high = (uint32_t)isr >> 16;
  descriptor->reserved = 0;
}

static bool vectors[IDT_MAX_DESCRIPTORS];

extern uint32_t isr_stub_table[];

extern uint32_t isr_test;
extern uint32_t isr_keyboard;

void idt_init() {
  idtr.base = (uintptr_t)&idt[0];
  idtr.limit = (uint16_t)sizeof(idt_entry_t) * IDT_MAX_DESCRIPTORS - 1;

  for (uint8_t vector = 0; vector < 32; vector++) {
    idt_set_descriptor(vector, isr_stub_table[vector], INT_TYPE_R0);
    vectors[vector] = true;
  }

  // We use an imported *label* (isr_test) here. A label in asm is a very
  // abstract concept, i.e. it doesn't leave any marks on the generated code by
  // itself. It can be thought of as a pointer dereference here, so when we say
  // isr_test, the assembler/compiler/whatever actually interprets this as the
  // first couple bytes of that procedure. That's why we have to use the
  // &-operator to refer to the label's *address*
  idt_set_descriptor(0x80, (uint32_t)&isr_test,
                     INT_TYPE_R0); // one of the most fundamental misunderstandings
  idt_set_descriptor(0x21, (uint32_t)&isr_keyboard,
                     INT_TYPE_R0);

  __asm__ volatile("lidt %0" : : "m"(idtr)); // load the new IDT
  __asm__ volatile("sti");                   // set the interrupt flag
}
