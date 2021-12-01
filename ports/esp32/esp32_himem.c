#include "esp32_himem.h"

#include <string.h>

#include "esp32/himem.h"

static esp_himem_handle_t s_mh;
static esp_himem_rangehandle_t s_rh;

static size_t s_himem_size = 0;
static size_t s_prev_ram_offset = -1;
static uint8_t *s_himem_ptr = NULL;

static size_t calc_range_offset(uint8_t **pptr, uint32_t addr)
{
    uint8_t *ptr = *pptr;
    size_t ram_offset = addr / ESP_HIMEM_BLKSZ * ESP_HIMEM_BLKSZ;
    size_t range_offset = addr - ram_offset;
    if (s_prev_ram_offset != ram_offset)
    {
        if (ptr)
        {
            esp_himem_unmap(s_rh, ptr, ESP_HIMEM_BLKSZ);
        }
        esp_himem_map(s_mh, s_rh, ram_offset, 0, ESP_HIMEM_BLKSZ, 0, (void**)&ptr);
        s_prev_ram_offset = ram_offset;
        *pptr = ptr;
    }
    return range_offset;
}

size_t himem_blksz(void)
{
    return ESP_HIMEM_BLKSZ;
}

void* _himem_read_cb(void* buf, size_t offset, void* data, size_t len)
{
    return memcpy(((char*)buf) + offset, data, len);
}

size_t himem_read_ex(uint32_t addr, void* ptr, size_t len, himem_read_cb cb)
{
    if (!ptr || len == 0)
    {
        return 0;
    }
    if (!cb)
    {
        cb = _himem_read_cb;
    }

    size_t offset = calc_range_offset(&s_himem_ptr, addr);
    size_t n = offset + len > ESP_HIMEM_BLKSZ ? ESP_HIMEM_BLKSZ - offset : len;
    cb(ptr, 0, s_himem_ptr + offset, n);
    if (n < len)
    {
        offset = calc_range_offset(&s_himem_ptr, addr + n);
        cb(ptr, n, s_himem_ptr + offset, len - n);
    }
    return len;
}

size_t himem_write(uint32_t addr, const void* buf, size_t len)
{
    if (!buf || len == 0)
    {
        return 0;
    }

    size_t offset = calc_range_offset(&s_himem_ptr, addr);
    size_t n = offset + len > ESP_HIMEM_BLKSZ ? ESP_HIMEM_BLKSZ - offset : len;
    memcpy(s_himem_ptr + offset, buf, n);
    if (n < len)
    {
        offset = calc_range_offset(&s_himem_ptr, addr + n);
        memcpy(s_himem_ptr + offset, ((const char*)buf) + n, len - n);
    }
    return len;
}

size_t himem_open(void)
{
    if (s_himem_size == 0)
    {
        s_himem_size = esp_himem_get_phys_size();
        // TODO size_t memfree = esp_himem_get_free_size();

        if (s_himem_size > 0)
        {
            esp_himem_alloc_map_range(ESP_HIMEM_BLKSZ, &s_rh);
            esp_himem_alloc(s_himem_size, &s_mh);
        }
    }
    return s_himem_size;
}

void himem_close(void)
{
    if (s_himem_ptr)
    {
        esp_himem_unmap(s_rh, s_himem_ptr, ESP_HIMEM_BLKSZ);
        s_himem_ptr = NULL;
    }
    s_prev_ram_offset = -1;

    if (s_himem_size > 0)
    {
        esp_himem_free(s_mh);
        esp_himem_free_map_range(s_rh);
        s_himem_size = 0;
    }
}
