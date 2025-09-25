#include "ui_component_button.h"
#include <esp_log.h>

namespace {
constexpr const char* TAG = "UI.Button";
}

namespace Ui {

void Button::create(lv_obj_t* parent) {
    root_ = lv_btn_create(parent);
    if (!root_) { ESP_LOGE(TAG, "Error creando lv_btn"); return; }

    // Layout interno: fila con gap desde tokens
    auto& S = Ui::getThemeStyles();
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(root_, S.tokens.btnIconGap, 0);
    // Permite que el padre controle el tamaño (grid/flex). El tema ajusta paddings/radius.
    // Si quieres tamaño fijo: lv_obj_set_size(root_, S.tokens.btnWidth, S.tokens.btnHeight);

    // Texto
    label_ = lv_label_create(root_);
    lv_label_set_text(label_, "");

    // (Icono se creará on-demand en setIcon)

    registerAllEvents();
    applyTheme();
}

void Button::setText(const char* txt) {
    if (label_) lv_label_set_text(label_, txt ? txt : "");
}

void Button::setIcon(const char* iconTxt) {
    if (!root_) return;

    if (!iconTxt || !*iconTxt) {
        // Eliminar icono si existe
        if (icon_) {
            lv_obj_del(icon_);
            icon_ = nullptr;
        }
        return;
    }

    if (!icon_) {
        icon_ = lv_label_create(root_);
    }
    lv_label_set_text(icon_, iconTxt);

    // Aplica estilo de icono “sobre primary” (porque el botón usa applyButtonPrimary)
    auto& S = Ui::getThemeStyles();
    Ui::applyIconOnPrimary(icon_, S, S.tokens.iconSizeMd);

    // Orden visual (izq/der) según flag
    // LVGL v8: usa índices 0..(n-1). Mover icon a inicio/fin.
    uint32_t cnt = lv_obj_get_child_cnt(root_);
    if (iconLeft_) {
        lv_obj_move_to_index(icon_, 0);
        // Asegura que el label quede al final para icon-left
        if (label_) lv_obj_move_to_index(label_, cnt - 1);
    } else {
        // icono al final
        lv_obj_move_to_index(icon_, cnt - 1);
    }
}

void Button::setIconLeft(bool left) {
    iconLeft_ = left;
    if (!root_ || !icon_) return;

    uint32_t cnt = lv_obj_get_child_cnt(root_);
    if (iconLeft_) {
        lv_obj_move_to_index(icon_, 0);
        if (label_) lv_obj_move_to_index(label_, cnt - 1);
    } else {
        lv_obj_move_to_index(icon_, cnt - 1);
    }
}

void Button::setEnabled(bool en) {
    if (!root_) return;
    if (en) lv_obj_clear_state(root_, LV_STATE_DISABLED);
    else    lv_obj_add_state(root_,   LV_STATE_DISABLED);
}

bool Button::isEnabled() const {
    return root_ && !lv_obj_has_state(root_, LV_STATE_DISABLED);
}

void Button::setOnClick(void (*cb)(void*), void* user_data) {
    onClick_  = cb;
    userData_ = user_data;
}

void Button::applyTheme() {
    auto& S = Ui::getThemeStyles();

    // 1) Cuerpo del botón (colores, padding, radius, estados…)
    //    Usamos la variante PRIMARY del theme.
    Ui::applyButtonPrimary(root_, S, /*setSize=*/false);

    // 2) Texto
    if (label_) {
        // Tipografía y color base (Montserrat body vía styles)
        lv_obj_add_style(label_, &S.labelBody, 0);
        lv_obj_set_style_text_font(label_, S.tokens.fontBody, 0);
        // Sobre fondo primary el texto debe ir con onPrimary
        lv_obj_set_style_text_color(label_, S.tokens.colorOnPrimary, 0);
        // Disabled
        lv_obj_set_style_text_color(label_, S.tokens.colorMuted, LV_STATE_DISABLED);
    }

    // 3) Icono (si existe): tamaño/color correctos para fondo primary
    if (icon_) {
        Ui::applyIconOnPrimary(icon_, S, S.tokens.iconSizeMd);
    }
}

/* ──────────────── Eventos ──────────────── */

void Button::registerAllEvents() {
    if (!root_) return;
    lv_obj_add_event_cb(root_, &Button::on_event_cb, LV_EVENT_ALL, this);
}

void Button::on_event_cb(lv_event_t* e) {
    auto* self = static_cast<Button*>(lv_event_get_user_data(e));
    if (self) self->onEvent(e);
}

void Button::onEvent(lv_event_t* e) {
    switch (lv_event_get_code(e)) {
        case LV_EVENT_CLICKED:
            if (onClick_) onClick_(userData_);
            break;
        default:
            break;
    }
}

} // namespace Ui
