#include "ui_component_button.h"
#include <esp_log.h>

namespace {
constexpr const char* TAG = "UI.Button";
}

namespace Ui {

/*====================== Ciclo de vida ======================*/

void Button::create(lv_obj_t* parent) {
    root_ = lv_btn_create(parent);
    if (!root_) { ESP_LOGE(TAG, "Error creando lv_btn"); return; }

    // Layout interno: fila con gap desde tokens
    auto& S = Ui::getThemeStyles();
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(root_, S.tokens.btnIconGap, 0);

    // Texto
    label_ = lv_label_create(root_);
    lv_label_set_text(label_, "");

    // (icon_ se creará on-demand en setIcon)

    registerAllEvents();
    applyTheme();
}

/*====================== API pública ========================*/

void Button::setText(const char* txt) {
    if (label_) lv_label_set_text(label_, txt ? txt : "");
}

void Button::setIcon(const char* iconTxt) {
    if (!root_) return;

    if (!iconTxt || !*iconTxt) {
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

    // Aplica estilo de icono en función de la variante actual
    auto& S = Ui::getThemeStyles();
    switch (variant_) {
        case Variant::Primary:
        case Variant::Secondary:
        case Variant::Destructive:
        case Variant::Success:
        case Variant::Warning:
            Ui::applyIconOnPrimary(icon_, S, S.tokens.iconSizeMd);
            break;
        case Variant::Ghost:
            Ui::applyIconOnSurface(icon_, S, S.tokens.iconSizeMd);
            break;
    }

    // Orden visual (izq/der) según flag
    uint32_t cnt = lv_obj_get_child_cnt(root_);
    if (iconLeft_) {
        lv_obj_move_to_index(icon_, 0);
        if (label_) lv_obj_move_to_index(label_, cnt - 1);
    } else {
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

void Button::setVariant(Variant v) {
    if (variant_ == v) return;
    variant_ = v;
    applyTheme();
}

/*====================== Theming ============================*/

void Button::applyVariantStyles() {
    auto& S = Ui::getThemeStyles();
    switch (variant_) {
        case Variant::Primary:     Ui::applyButtonPrimary   (root_, S, /*setSize=*/false); break;
        case Variant::Secondary:   Ui::applyButtonSecondary (root_, S, /*setSize=*/false); break;
        case Variant::Ghost:       Ui::applyButtonGhost     (root_, S, /*setSize=*/false); break;
        case Variant::Destructive: Ui::applyButtonDestructive(root_, S, /*setSize=*/false); break;
        case Variant::Success:     Ui::applyButtonSuccess   (root_, S, /*setSize=*/false); break;
        case Variant::Warning:     Ui::applyButtonWarning   (root_, S, /*setSize=*/false); break;
    }
}

void Button::applyTheme() {
    auto& S = Ui::getThemeStyles();

    // 1) Cuerpo del botón (colores, padding, radius, estados…)
    applyVariantStyles();

    // 2) Texto
    if (label_) {
        lv_obj_add_style(label_, &S.labelBody, 0);
        lv_obj_set_style_text_font(label_, S.tokens.fontBody, 0);

        // Color de texto según variante
        switch (variant_) {
            case Variant::Primary:
                lv_obj_set_style_text_color(label_, S.tokens.colorOnPrimary, 0);
                break;
            case Variant::Secondary:
                lv_obj_set_style_text_color(label_, S.tokens.colorOnSecondary, 0);
                break;
            case Variant::Destructive:
                lv_obj_set_style_text_color(label_, S.tokens.colorOnError, 0);
                break;
            case Variant::Success:
                lv_obj_set_style_text_color(label_, S.tokens.colorOnSuccess, 0);
                break;
            case Variant::Warning:
                lv_obj_set_style_text_color(label_, S.tokens.colorOnWarning, 0);
                break;
            case Variant::Ghost:
                lv_obj_set_style_text_color(label_, S.tokens.colorOnSurface, 0);
                break;
        }

        // Disabled
        lv_obj_set_style_text_color(label_, S.tokens.colorMuted, LV_STATE_DISABLED);
    }

    // 3) Icono (si existe): tamaño/color acordes a la variante
    if (icon_) {
        switch (variant_) {
            case Variant::Ghost:
                Ui::applyIconOnSurface(icon_, S, S.tokens.iconSizeMd);
                break;
            default:
                Ui::applyIconOnPrimary(icon_, S, S.tokens.iconSizeMd);
                break;
        }
    }
}

/*====================== Factory: Back ======================*/

Button* Button::CreateBack(lv_obj_t* parent) {
    BackProps def{};
    return CreateBack(parent, def);
}

Button* Button::CreateBack(lv_obj_t* parent, const BackProps& p) {
    auto* b = new Button();
    b->create(parent);

    // Variante por defecto: Primary (se puede cambiar si lo necesitas)
    b->setVariant(Variant::Primary);

    // Icono back a la izquierda
    b->setIcon(LV_SYMBOL_LEFT);
    b->setIconLeft(p.iconLeft);

    // Texto (opcional)
    if (p.showText) b->setText(p.text);
    else            b->setText("");

    // Enabled / callback
    b->setEnabled(p.enabled);
    if (p.onClick) b->setOnClick(p.onClick, p.user);

    // Alineación en su contenedor
    lv_obj_align(b->root(), p.align, p.ofsX, p.ofsY);
    return b;
}

/*====================== Eventos ============================*/

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
