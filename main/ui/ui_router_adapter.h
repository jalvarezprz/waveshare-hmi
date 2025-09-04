#pragma once
/**
 * @file ui_router_adapter.h
 * @brief Adaptador para delegar la navegación desde el renderer de grid hacia tu router real.
 *
 * Implementa esta función en tu proyecto para conectar con tu sistema de rutas.
 */
extern "C" {
    struct cJSON;
    void ui_router_on_menu_item_selected(const cJSON* item);
}
