#ifndef CURSOR_H

#define CURSOR_H

#include <stdint.h>
#define VGA_WIDTH 80
void update_cursor(int x, int y);
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
uint16_t get_cursor_position(void);

#endif // !CURSOR_H
