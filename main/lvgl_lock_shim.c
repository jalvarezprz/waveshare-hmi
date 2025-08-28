// main/lvgl_lock_shim.c
#include <stdbool.h>
#include <stdint.h>
#include "lvgl_port.h"

// Mantén tus nombres públicos y redirígelos al port de Espressif
bool example_lvgl_lock(int timeout_ms)
{
    uint32_t t = (timeout_ms < 0) ? UINT32_MAX : (uint32_t)timeout_ms;
    return lvgl_port_lock(t);
}

void example_lvgl_unlock(void)
{
    lvgl_port_unlock();
}
