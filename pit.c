#include "io.h"
#include "pit.h"

#define PIT_CHAN0 0x40
#define PIT_COM 0x43

void setup_timer(uint16_t reload) {
  outb(PIT_COM, 0b00110000); // setup channel 0 in "interrupt on terminal count" mode
  io_wait();
  outb(PIT_CHAN0, (uint8_t)reload);
  io_wait();
  outb(PIT_CHAN0, (uint8_t)(reload >> 8));
}
