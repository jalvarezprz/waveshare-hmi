#include "ui/component/ui_component_label.h"
#include "ui/theme/ui_theme_styles.h"

namespace Ui {

UiLabel::UiLabel(const char* text, LabelVariant v)
: text_(text), variant_(v) {}

lv_obj_t* UiLabel::create(lv_obj_t* parent) {
    Ui::themeInitOnce();
    lv_obj_t* lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text_ ? text_ : "");

    auto& s = Ui::getThemeStyles();
    switch (variant_) {
        case LabelVariant::Title:   lv_obj_add_style(lbl, &s.labelTitle,   LV_PART_MAIN); break;
        case LabelVariant::Body:    lv_obj_add_style(lbl, &s.labelBody,    LV_PART_MAIN); break;
        case LabelVariant::Caption: lv_obj_add_style(lbl, &s.labelCaption, LV_PART_MAIN); break;
    }
    return lbl;
}

} // namespace Ui
