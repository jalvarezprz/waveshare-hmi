#pragma once
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Bloquea el acceso a LVGL.
 * @param timeout_ms  Tiempo en ms para esperar el lock. Usa -1 para esperar indefinidamente.
 * @return true si se tomó el lock, false si expiró el timeout.
 */
bool example_lvgl_lock(int timeout_ms);

/**
 * Libera el lock de LVGL tomado con example_lvgl_lock().
 */
void example_lvgl_unlock(void);

#ifdef __cplusplus
}
#endif
