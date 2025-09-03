#ifndef FILDES_H
#define FILDES_H

#define VIRT_STREAM_TYPE 0
#define EXT2_FILE_TYPE 1
#define NULL_TYPE 255

#define SEEK_SET -1
#define SEEK_CUR 0
#define SEEK_END 1

#include <stdint.h>

struct fildes;
typedef struct fildes fildes_t;


extern const fildes_t NULL_FD;

int read(fildes_t *fildes, uint32_t n, void *ret);
int write(fildes_t *fildes, uint32_t n, const void *src);
void close(fildes_t *fildes);
void seek(fildes_t *fildes, int dir, uint32_t n);

#endif // !FILDES_H
