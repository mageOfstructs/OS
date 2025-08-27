#include <stdint.h>
void PIC_remap(int offset1, int offset2);
void PIC_sendEOI(uint8_t irq);
char get_key_pressed();
