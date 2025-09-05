#pragma once
/**
 * @file ui_preset_switch.h
 * @brief Presets de negocio para Switch (On/Off, Auto/Manual, Heating/Cooling, Start/Stop).
 * @defgroup ui_preset_switch Presets Switch
 * @ingroup ui_component_switch
 * @{
 *
 * Descripcion:
 * Atajos para crear switches tipicos de la HMI HVAC usando el componente
 * Ui::Component::Switch. Devuelven el Handle del componente original.
 *
 * Dos niveles de API:
 * 1) Completa: recibe UiThemeStyles y Callbacks tipados.
 * 2) Conveniencia: resuelve internamente Ui::getThemeStyles() y acepta
 *    un callback simple void(*)(bool) o ninguno.
 */

#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_switch.h"
#include "lvgl.h"

namespace Ui::Preset::Switch {

// Aliases a tipos del componente
using Handle    = Ui::Component::Switch::Handle;
using Callbacks = Ui::Component::Switch::Callbacks;

/** On/Off con labels "Off" y "On". checked=true => On */
Handle OnOff      (lv_obj_t* parent, Ui::UiThemeStyles& s, Callbacks cb = {});

/** Auto/Manual con labels "Auto" y "Manual". checked=true => Auto */
Handle AutoManual (lv_obj_t* parent, Ui::UiThemeStyles& s, Callbacks cb = {});

/** Heating/Cooling con labels "Heating" y "Cooling". checked=true => Heating */
Handle HeatingCooling (lv_obj_t* parent, Ui::UiThemeStyles& s, Callbacks cb = {});

/** Start/Stop con labels "Start" y "Stop". checked=true => Start (en marcha) */
Handle StartStop (lv_obj_t* parent, Ui::UiThemeStyles& s, Callbacks cb = {});

/* Overloads de conveniencia (sin Styles explicito) */

/** Sin callback */
Handle OnOff          (lv_obj_t* parent);
Handle AutoManual     (lv_obj_t* parent);
Handle HeatingCooling (lv_obj_t* parent);
Handle StartStop      (lv_obj_t* parent);

/** Con callback simple void(*)(bool checked) */
Handle OnOff          (lv_obj_t* parent, void (*onChange)(bool));
Handle AutoManual     (lv_obj_t* parent, void (*onChange)(bool));
Handle HeatingCooling (lv_obj_t* parent, void (*onChange)(bool));
Handle StartStop      (lv_obj_t* parent, void (*onChange)(bool));

/** @} */ // end group
} // namespace Ui::Preset::Switch
