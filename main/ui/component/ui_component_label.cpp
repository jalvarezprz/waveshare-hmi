/**
 * @file ui_component_label.cpp
 * @brief Implementación del componente Label.
 * @ingroup ui_component_label
 */

#include "ui/component/ui_component_label.h"
#include "ui/theme/ui_theme_styles.h"
#include "lvgl.h"

namespace Ui::Component::Label {

static void apply_variant(lv_obj_t* lb, Ui::UiThemeStyles& s, Variant v) {
    // Limpiamos estilos anteriores por si se re-aplica
    lv_obj_remove_style_all(lb);

    switch (v) {
        case Variant::Title:
            lv_obj_add_style(lb, &s.labelTitle,   LV_PART_MAIN);
            break;
        case Variant::Body:
            lv_obj_add_style(lb, &s.labelBody,    LV_PART_MAIN);
            break;
        case Variant::Caption:
            lv_obj_add_style(lb, &s.labelCaption, LV_PART_MAIN);
            break;
    }
}

Handle create(lv_obj_t* parent, Ui::UiThemeStyles& styles, const Props& p) {
    Ui::themeInitOnce();
    Handle h{};
    h.root = lv_label_create(parent);

    apply_variant(h.root, styles, p.variant);
    lv_label_set_text(h.root, p.text ? p.text : "");
    lv_obj_set_style_text_align(h.root, p.align, LV_PART_MAIN);

    return h;
}

void setText(Handle& h, const char* txt) {
    if (!h.root) return;
    lv_label_set_text(h.root, txt ? txt : "");
}

void setVariant(Handle& h, Ui::UiThemeStyles& s, Variant v) {
    if (!h.root) return;
    apply_variant(h.root, s, v);
}

void setAlign(Handle& h, lv_text_align_t align) {
    if (!h.root) return;
    lv_obj_set_style_text_align(h.root, align, LV_PART_MAIN);
}

} // namespace Ui::Component::Label
