/**
 * @file ui_menu_field_factory.h
 * @brief Factoría de widgets LVGL para campos de una vista detail (JSON).
 * @ingroup ui_menu
 */
#pragma once

// Forward mínimo para no arrastrar dependencias a los consumidores.
struct cJSON;
struct _lv_obj_t;
typedef _lv_obj_t lv_obj_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Crea el widget adecuado para un campo JSON (slider, dropdown, number, text, button).
 *
 * Lee las claves del objeto JSON `f`:
 *   - "widget_type": "slider" | "dropdown" | "number" | "text" | "button"
 *   - "label": texto informativo (lo pinta el caller)
 *   - "unit": unidad (p.ej. "°C")
 *   - "mock": valor inicial simulado
 *   - "min","max","step" (slider)
 *   - "options": array de strings (dropdown)
 *   - "editable": bool (habilitado/disabled)
 *
 * @param parent  Contenedor LVGL donde anclar el control.
 * @param f       Objeto cJSON del campo.
 * @return lv_obj_t* al widget creado (o contenedor en el caso de slider), o nullptr si no soportado.
 */
lv_obj_t* ui_menu_field_create_widget(lv_obj_t* parent, const cJSON* f);

#ifdef __cplusplus
}
#endif
