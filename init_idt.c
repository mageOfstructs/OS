#include <stdint.h>

#define CODE_SEG_DESC (0x01 << 3)
#define INT_TYPE 0b10001110

struct InterruptDescriptor32 {
  uint16_t offset_1;       // offset bits 0..15
  uint16_t selector;       // a code segment selector in GDT or LDT
  uint8_t zero;            // unused, set to 0
  uint8_t type_attributes; // gate type, dpl, and p fields
  uint16_t offset_2;       // offset bits 16..31
};

typedef struct InterruptDescriptor32 IDT_Table[];
extern IDT_Table IDT_start;

extern unsigned long raw_int_keyboard;

void init_idt() {
  // setup keyboard interrupt
  struct InterruptDescriptor32 keyboardISR = {
      .offset_1 = (uint16_t)(raw_int_keyboard & 65535), // TODO
      .selector = CODE_SEG_DESC,
      .zero = 0,
      .type_attributes = INT_TYPE,
      .offset_2 = (uint16_t)(raw_int_keyboard >> 8), // TODO
  };
  IDT_start[0x21] = keyboardISR;
}
