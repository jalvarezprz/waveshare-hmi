#pragma once
/**
 * @file ui_preset_slider.h
 * @brief Presets del componente Slider para casos típicos (Temperatura ºC, Porcentaje %).
 * @defgroup ui_preset_slider Presets Slider
 * @ingroup ui_component_slider
 * @{
 *
 * Objetivo:
 * - Atajos semánticos para sliders comunes en HVAC.
 * - Devuelven el Handle del componente Slider.
 * - Overloads de conveniencia que resuelven Ui::getThemeStyles().
 */

#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_slider.h"
#include "lvgl.h"

namespace Ui::Preset::Slider {

// Aliases
using Handle    = Ui::Component::Slider::Handle;
using Callbacks = Ui::Component::Slider::Callbacks;

/** Consigna de temperatura (ºC). Rango por defecto [5..35], valor inicial 21. */
Handle TemperatureC(lv_obj_t* parent, Ui::UiThemeStyles& s,
                    int32_t initial = 21,
                    int32_t minC = 5, int32_t maxC = 35,
                    Callbacks cb = {});

/** Porcentaje 0..100 %. */
Handle Percentage(lv_obj_t* parent, Ui::UiThemeStyles& s,
                  int32_t initial = 50,
                  Callbacks cb = {});

/* Overloads sencillos (resuelven styles) */

Handle TemperatureC(lv_obj_t* parent, int32_t initial = 21,
                    int32_t minC = 5, int32_t maxC = 35);

Handle TemperatureC(lv_obj_t* parent, void (*onRelease)(int32_t newValue));

Handle Percentage(lv_obj_t* parent, int32_t initial);

Handle Percentage(lv_obj_t* parent, void (*onRelease)(int32_t newValue));

/** @} */ // end group
} // namespace Ui::Preset::Slider
