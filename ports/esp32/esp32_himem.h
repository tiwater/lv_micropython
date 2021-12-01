#ifndef __ESP32_HIMEM_H
#define __ESP32_HIMEM_H

#include <stddef.h>
#include <stdint.h>

typedef void* (*himem_read_cb) (void* ptr, size_t offset, void* data, size_t len);

/**
  * we use the first block as memory cache for app data less than 32KB,
  * and the other blocks for file trans
  */
size_t himem_blksz(void);
#define himem_get_small_file_trans_addr()  (0)
#define himem_get_large_file_trans_addr()  himem_blksz()

size_t himem_open(void);
void himem_close(void);

size_t himem_read_ex(uint32_t addr, void* ptr, size_t len, himem_read_cb cb);
#define himem_read(addr, buf, len) \
    himem_read_ex((addr), (buf), (len), NULL)
size_t himem_write(uint32_t addr, const void* buf, size_t len);

#endif // __ESP32_HIMEM_H
