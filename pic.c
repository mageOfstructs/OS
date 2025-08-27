#include "pic.h"
#include "io.h"
#include "printf.h"
#include <stdbool.h>

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define KEYBOARD_COM 0x60

/* reinitialize the PIC controllers, giving them specified vector offsets
   rather than 8h and 70h, as configured by default */

#define ICW1_ICW4 0x01      /* Indicates that ICW4 will be present */
#define ICW1_SINGLE 0x02    /* Single (cascade) mode */
#define ICW1_INTERVAL4 0x04 /* Call address interval 4 (8) */
#define ICW1_LEVEL 0x08     /* Level triggered (edge) mode */
#define ICW1_INIT 0x10      /* Initialization - required! */

#define ICW4_8086 0x01       /* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO 0x02       /* Auto (normal) EOI */
#define ICW4_BUF_SLAVE 0x08  /* Buffered mode/slave */
#define ICW4_BUF_MASTER 0x0C /* Buffered mode/master */
#define ICW4_SFNM 0x10       /* Special fully nested (not) */

#define CASCADE_IRQ 2

#define PIC_EOI 0x20 /* End-of-interrupt command code */

/*
arguments:
        offset1 - vector offset for master PIC
                vectors on the master become offset1..offset1+7
        offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(int offset1, int offset2) // call it with 0x20 and 0x28
{
  outb(PIC1_COMMAND,
       ICW1_INIT |
           ICW1_ICW4); // starts the initialization sequence (in cascade mode)
  io_wait();
  outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();
  outb(PIC1_DATA, offset1); // ICW2: Master PIC vector offset
  io_wait();
  outb(PIC2_DATA, offset2); // ICW2: Slave PIC vector offset
  io_wait();
  outb(PIC1_DATA, 1 << CASCADE_IRQ); // ICW3: tell Master PIC that there is a
                                     // slave PIC at IRQ2
  io_wait();
  outb(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
  io_wait();

  outb(PIC1_DATA,
       ICW4_8086); // ICW4: have the PICs use 8086 mode (and not 8080 mode)
  io_wait();
  outb(PIC2_DATA, ICW4_8086);
  io_wait();

  outb(KEYBOARD_COM, 0xF0);
  inb(KEYBOARD_COM); // discard ack
  outb(KEYBOARD_COM, 0x00);
  printf("Current scanset: %d\n", inb(KEYBOARD_COM));

  // Unmask both PICs.
  outb(PIC1_DATA, 1); // we haven't programmed the PIT yet, so we skip it
  outb(PIC2_DATA, 0);
}

void PIC_sendEOI(uint8_t irq) {
  if (irq >= 8)
    outb(PIC2_COMMAND, PIC_EOI);

  outb(PIC1_COMMAND, PIC_EOI);
}

static bool shift_pressed = false;
#define ESC 0x0e
// BUG: if both shift keys are pressed the character is lowercase
char conv_scancode(unsigned char sc) {
  char ret = 0;
  sc &= 127; // make sure we get a 'key pressed' sc
  if (sc > 1 && sc < 11) {
    ret = '0' + sc - 1;
  } else
    switch (sc) {
    case 1:
      return ESC;
    case 11:
      ret = '0';
      break;
    case 12:
      ret = '-';
      break;
    case 13:
      ret = '=';
      break;
    case 14:
      return 0; // TODO: Backspace
    case 15:
      return 0; // TODO: TAB
    case 0x10:
      ret = 'q';
      break;
    case 0x11:
      ret = 'w';
      break;
    case 0x12:
      ret = 'e';
      break;
    case 0x13:
      ret = 'r';
      break;
    case 0x14:
      ret = 't';
      break;
    case 0x15:
      ret = 'y';
      break;
    case 0x16:
      ret = 'u';
      break;
    case 0x17:
      ret = 'i';
      break;
    case 0x18:
      ret = 'o';
      break;
    case 0x19:
      ret = 'p';
      break;
    case 0x1a:
      ret = '[';
      break;
    case 0x1b:
      ret = ']';
      break;
    case 0x1c:
      return 0; // TODO: Enter
    case 0x1d:
      return 0; // TODO: CTRL
    case 0x1e:
      ret = 'a';
      break;
    case 0x1f:
      ret = 's';
      break;
    case 0x20:
      ret = 'd';
      break;
    case 0x21:
      ret = 'f';
      break;
    case 0x22:
      ret = 'g';
      break;
    case 0x23:
      ret = 'h';
      break;
    case 0x24:
      ret = 'j';
      break;
    case 0x25:
      ret = 'k';
      break;
    case 0x26:
      ret = 'l';
      break;
    case 0x27:
      ret = ';';
      break;
    case 0x28:
      ret = '\'';
      break;
    case 0x29:
      ret = '`';
      break;
    case 0x2a:
      shift_pressed = !shift_pressed;
      return 0; // TODO: LShift
    case 0x2b:
      return '\\';
    case 0x2c:
      ret = 'z';
      break;
    case 0x2d:
      ret = 'x';
      break;
    case 0x2e:
      ret = 'c';
      break;
    case 0x2f:
      ret = 'v';
      break;
    case 0x30:
      ret = 'b';
      break;
    case 0x31:
      ret = 'n';
      break;
    case 0x32:
      ret = 'm';
      break;
    case 0x33:
      ret = ',';
      break;
    case 0x34:
      ret = '.';
      break;
    case 0x35:
      ret = '/';
      break;
    case 0x36:
      shift_pressed = !shift_pressed;
      return 0; // TODO: RShift
    }
  if (shift_pressed && ret >= 'a' && ret <= 'z') {
    ret &= 0b11011111;
  }
  return ret;
}

char PIC_keyboard_get_scan_code() { return inb(KEYBOARD_COM); }
char PIC_keyboard_get_char() { return conv_scancode(inb(KEYBOARD_COM)); }

char get_key_pressed() {
  unsigned char sc = PIC_keyboard_get_scan_code();
  char key = conv_scancode(sc); // conv_scancode needs to be executed every time
                                // since it modifies global state
  if (sc < 128) {
    return key;
  }
  return 0;
}
