/**
 * @file ui_preset_label.cpp
 * @brief Implementación de presets de Label.
 * @ingroup ui_preset_label
 */

#include "ui/preset/ui_preset_label.h"

namespace Ui::Preset::Label {
using Ui::Component::Label::Props;
using Ui::Component::Label::create;
using Ui::Component::Label::Variant;

/* ------------------------------ Presets base ------------------------------ */

Handle Title(lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text, lv_text_align_t align) {
    Props p{ .text = text, .variant = Variant::Title, .align = align };
    return create(parent, s, p);
}

Handle Section(lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text, lv_text_align_t align) {
    // Base Body, elevamos la fuente a Title para jerarquía intermedia sin nuevo estilo.
    Props p{ .text = text, .variant = Variant::Body, .align = align };
    Handle h = create(parent, s, p);
    lv_obj_set_style_text_font(h.root, s.tokens.fontTitle, LV_PART_MAIN);
    // Color ya viene del estilo Body; no forzamos colorText aquí.
    return h;
}

Handle Body(lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text, lv_text_align_t align) {
    Props p{ .text = text, .variant = Variant::Body, .align = align };
    return create(parent, s, p);
}

Handle Caption(lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text, lv_text_align_t align) {
    Props p{ .text = text, .variant = Variant::Caption, .align = align };
    return create(parent, s, p);
}

Handle Value(lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text, lv_text_align_t align) {
    // Valor destacado: usamos estilo Title para ganar presencia.
    Props p{ .text = text, .variant = Variant::Title, .align = align };
    Handle h = create(parent, s, p);
    // Color ya viene del estilo Title; no forzamos colorText aquí.
    return h;
}

/* ------------------------------ Preset Status ----------------------------- */

Handle Status(lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text, StatusKind kind, lv_text_align_t align) {
    // Base tipográfica Body y color según estado.
    Props p{ .text = text, .variant = Variant::Body, .align = align };
    Handle h = create(parent, s, p);

    switch (kind) {
        case StatusKind::Success:
            lv_obj_set_style_text_color(h.root, s.tokens.colorSuccess, LV_PART_MAIN);
            break;
        case StatusKind::Warning:
            lv_obj_set_style_text_color(h.root, s.tokens.colorWarning, LV_PART_MAIN);
            break;
        case StatusKind::Error:
            lv_obj_set_style_text_color(h.root, s.tokens.colorError, LV_PART_MAIN);
            break;
    }
    return h;
}

/* --------------------------- Overloads sencillos --------------------------- */

static inline Ui::UiThemeStyles& S() { Ui::themeInitOnce(); return Ui::getThemeStyles(); }

Handle Title   (lv_obj_t* parent, const char* text, lv_text_align_t align) { return Title  (parent, S(), text, align); }
Handle Section (lv_obj_t* parent, const char* text, lv_text_align_t align) { return Section(parent, S(), text, align); }
Handle Body    (lv_obj_t* parent, const char* text, lv_text_align_t align) { return Body   (parent, S(), text, align); }
Handle Caption (lv_obj_t* parent, const char* text, lv_text_align_t align) { return Caption(parent, S(), text, align); }
Handle Value   (lv_obj_t* parent, const char* text, lv_text_align_t align) { return Value  (parent, S(), text, align); }

Handle StatusSuccess(lv_obj_t* parent, const char* text, lv_text_align_t align) {
    return Status(parent, S(), text, StatusKind::Success, align);
}
Handle StatusWarning(lv_obj_t* parent, const char* text, lv_text_align_t align) {
    return Status(parent, S(), text, StatusKind::Warning, align);
}
Handle StatusError  (lv_obj_t* parent, const char* text, lv_text_align_t align) {
    return Status(parent, S(), text, StatusKind::Error, align);
}

} // namespace Ui::Preset::Label
