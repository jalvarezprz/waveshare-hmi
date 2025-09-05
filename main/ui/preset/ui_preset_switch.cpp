/**
 * @file ui_preset_switch.cpp
 * @brief Implementacion de presets de Switch.
 * @ingroup ui_preset_switch
 */

#include "ui/preset/ui_preset_switch.h"

namespace Ui::Preset::Switch {
using namespace Ui::Component::Switch;

static inline Handle make(lv_obj_t* parent, Ui::UiThemeStyles& s,
                          const char* left, const char* right,
                          bool checkedDefault,
                          Variant var,
                          const Callbacks& cb) {
    Props p{};
    p.checked    = checkedDefault;
    p.enabled    = true;
    p.variant    = var;
    p.labelLeft  = left;
    p.labelRight = right;
    return create(parent, s, p, cb);
}

/* API completa */

Handle OnOff(lv_obj_t* parent, Ui::UiThemeStyles& s, Callbacks cb) {
    // checked=true => On
    return make(parent, s, "Off", "On", false, Variant::Default, cb);
}

Handle AutoManual(lv_obj_t* parent, Ui::UiThemeStyles& s, Callbacks cb) {
    // checked=true => Auto
    return make(parent, s, "Auto", "Manual", true, Variant::Default, cb);
}

Handle HeatingCooling(lv_obj_t* parent, Ui::UiThemeStyles& s, Callbacks cb) {
    // checked=true => Heating, checked=false => Cooling
    // Variante neutra (Default). Si prefieres darle color “caliente”, podríamos
    // ampliar Variant o mapear a Success; de momento mantenemos Default.
    return make(parent, s, "Cooling", "Heating", true, Variant::Default, cb);
}

Handle StartStop(lv_obj_t* parent, Ui::UiThemeStyles& s, Callbacks cb) {
    // checked=true => Start (en marcha), checked=false => Stop (parado)
    // Usamos Success para que el estado activo (Start) vaya en color de exito.
    return make(parent, s, "Stop", "Start", false, Variant::Success, cb);
}

/* Overloads SIN callback */

Handle OnOff(lv_obj_t* parent) {
    auto& s = Ui::getThemeStyles();
    return OnOff(parent, s, {});
}

Handle AutoManual(lv_obj_t* parent) {
    auto& s = Ui::getThemeStyles();
    return AutoManual(parent, s, {});
}

Handle HeatingCooling(lv_obj_t* parent) {
    auto& s = Ui::getThemeStyles();
    return HeatingCooling(parent, s, {});
}

Handle StartStop(lv_obj_t* parent) {
    auto& s = Ui::getThemeStyles();
    return StartStop(parent, s, {});
}

/* Overloads CON callback simple void(*)(bool) */

static inline Callbacks cb_from(void (*onChange)(bool)) {
    if (!onChange) return {};
    return Callbacks{
        .onChange = [onChange](Handle&, bool checked, void*) { onChange(checked); }
    };
}

Handle OnOff(lv_obj_t* parent, void (*onChange)(bool)) {
    auto& s = Ui::getThemeStyles();
    return OnOff(parent, s, cb_from(onChange));
}

Handle AutoManual(lv_obj_t* parent, void (*onChange)(bool)) {
    auto& s = Ui::getThemeStyles();
    return AutoManual(parent, s, cb_from(onChange));
}

Handle HeatingCooling(lv_obj_t* parent, void (*onChange)(bool)) {
    auto& s = Ui::getThemeStyles();
    return HeatingCooling(parent, s, cb_from(onChange));
}

Handle StartStop(lv_obj_t* parent, void (*onChange)(bool)) {
    auto& s = Ui::getThemeStyles();
    return StartStop(parent, s, cb_from(onChange));
}

} // namespace Ui::Preset::Switch
