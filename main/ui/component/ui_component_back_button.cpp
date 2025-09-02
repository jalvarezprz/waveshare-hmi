#include "ui_component_back_button.h"
#include "ui/theme/ui_theme_styles.h"   // tokens + styles
#include "lvgl.h"

namespace Ui {

lv_obj_t* create_back_button(lv_obj_t* parent, std::function<void()> on_click) {
    auto& s = Ui::getThemeStyles();
    auto& t = Ui::getThemeTokens();

    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, 120, 48);
    lv_obj_add_style(btn, &s.btnGhost, LV_PART_MAIN);

    lv_obj_t* lbl = lv_label_create(btn);
    lv_label_set_text(lbl, LV_SYMBOL_LEFT " Atrás");                 // texto único con tilde
    lv_obj_set_style_text_font(lbl, t.fontBody, LV_PART_MAIN);       // acentos OK
    lv_obj_center(lbl);

    if (on_click) {
        // guardamos el puntero al std::function en user_data para usarlo en el callback C
        auto *cb = new std::function<void()>(std::move(on_click));
        lv_obj_add_event_cb(btn, [](lv_event_t* e){
            auto *f = static_cast<std::function<void()>*>(lv_event_get_user_data(e));
            if (f && *f) (*f)();
        }, LV_EVENT_CLICKED, cb);
        // liberar al destruir el botón
        lv_obj_add_event_cb(btn, [](lv_event_t* e){
            auto *f = static_cast<std::function<void()>*>(lv_event_get_user_data(e));
            delete f;
        }, LV_EVENT_DELETE, cb);
    }
    return btn;
}

} // namespace Ui
