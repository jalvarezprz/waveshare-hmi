/**
 * @file ui_preset_button.cpp
 * @brief Implementacion de presets (catalogo de negocio) para botones.
 * @ingroup ui_preset_button
 */

#include "ui/preset/ui_preset_button.h"
#include "lvgl.h"

namespace Ui::Preset::Button {
using namespace Ui::Component::Button;

/**
 * @brief Helper interno para construir un preset con Props minimos.
 * @param parent Padre LVGL.
 * @param s      ThemeStyles activo.
 * @param txt    Texto del boton.
 * @param ic     Icono (LV_SYMBOL_*).
 * @param v      Variante visual.
 * @param toggle Si es conmutador.
 * @param cb     Callbacks.
 * @return Handle del boton.
 */
static inline Handle make(lv_obj_t* parent, Ui::UiThemeStyles& s,
                          const char* txt, const char* ic,
                          Variant v, bool toggle, const Callbacks& cb) {
    Props p{ .text=txt, .icon=ic, .variant=v, .toggle=toggle };
    return Button::create(parent, s, p, cb);
}

/* ===== API completa ===== */

Button::Handle Yes (lv_obj_t* parent, Ui::UiThemeStyles& s, Button::Callbacks cb) {
    return make(parent, s, "Si", LV_SYMBOL_OK, Variant::Success, false, cb);
}
Button::Handle No  (lv_obj_t* parent, Ui::UiThemeStyles& s, Button::Callbacks cb) {
    return make(parent, s, "No", LV_SYMBOL_CLOSE, Variant::Destructive, false, cb);
}
Button::Handle Back(lv_obj_t* parent, Ui::UiThemeStyles& s, Button::Callbacks cb) {
    return make(parent, s, "Atras", LV_SYMBOL_LEFT, Variant::Ghost, false, cb);
}
Button::Handle Next(lv_obj_t* parent, Ui::UiThemeStyles& s, Button::Callbacks cb) {
    return make(parent, s, "Siguiente", LV_SYMBOL_RIGHT, Variant::Primary, false, cb);
}

/* ===== Helpers de conveniencia ===== */

static inline Button::Callbacks cb_from(void (*onClick)(void)) {
    if (!onClick) return {};
    return Button::Callbacks{
        .onClick = [onClick](Button::Handle&, void*) { onClick(); }
    };
}

/* Overloads SIN callback */

Button::Handle Yes  (lv_obj_t* parent) {
    auto& s = Ui::getThemeStyles();
    return Yes(parent, s, {});
}
Button::Handle No   (lv_obj_t* parent) {
    auto& s = Ui::getThemeStyles();
    return No(parent, s, {});
}
Button::Handle Back (lv_obj_t* parent) {
    auto& s = Ui::getThemeStyles();
    return Back(parent, s, {});
}
Button::Handle Next (lv_obj_t* parent) {
    auto& s = Ui::getThemeStyles();
    return Next(parent, s, {});
}

/* Overloads CON callback simple */

Button::Handle Yes  (lv_obj_t* parent, void (*onClick)(void)) {
    auto& s = Ui::getThemeStyles();
    return Yes(parent, s, cb_from(onClick));
}
Button::Handle No   (lv_obj_t* parent, void (*onClick)(void)) {
    auto& s = Ui::getThemeStyles();
    return No(parent, s, cb_from(onClick));
}
Button::Handle Back (lv_obj_t* parent, void (*onClick)(void)) {
    auto& s = Ui::getThemeStyles();
    return Back(parent, s, cb_from(onClick));
}
Button::Handle Next (lv_obj_t* parent, void (*onClick)(void)) {
    auto& s = Ui::getThemeStyles();
    return Next(parent, s, cb_from(onClick));
}

} // namespace Ui::Preset::Button
