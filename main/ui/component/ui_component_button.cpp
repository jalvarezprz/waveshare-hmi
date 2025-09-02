#include "ui/component/ui_component_button.h"
#include "esp_log.h"
#include "ui/theme/ui_theme_tokens.h"

static const char* TAG_BTN = "UI/Button";

bool Button::stylesInited_ = false;
lv_style_t Button::styleBtnBase_;
lv_style_t Button::styleBtnPressed_;
lv_style_t Button::styleBtnFocused_;
lv_style_t Button::styleIconLabel_;
lv_style_t Button::styleTextLabel_;

void Button::ensureStyles()
{
    if (stylesInited_) return;
    stylesInited_ = true;

    using namespace Ui::Tokens;

    // Base
    lv_style_init(&styleBtnBase_);
    lv_style_set_bg_color(&styleBtnBase_, button_primary_bg());
    lv_style_set_border_width(&styleBtnBase_, 0);
    lv_style_set_pad_left(&styleBtnBase_,   button_pad_lr());
    lv_style_set_pad_right(&styleBtnBase_,  button_pad_lr());
    lv_style_set_pad_top(&styleBtnBase_,    button_pad_tb());
    lv_style_set_pad_bottom(&styleBtnBase_, button_pad_tb());
    lv_style_set_text_color(&styleBtnBase_, button_primary_text());

    // Estados derivados con helpers LVGL
    lv_style_init(&styleBtnPressed_);
    lv_style_set_bg_color(&styleBtnPressed_, lv_color_darken(button_primary_bg(), LV_OPA_30));

    lv_style_init(&styleBtnFocused_);
    lv_style_set_bg_color(&styleBtnFocused_, lv_color_lighten(button_primary_bg(), LV_OPA_20));

    // Labels
    lv_style_init(&styleIconLabel_);
    lv_style_set_text_font(&styleIconLabel_, font_icon());
    lv_style_set_text_color(&styleIconLabel_, button_primary_text());

    lv_style_init(&styleTextLabel_);
    lv_style_set_text_font(&styleTextLabel_, font_text());
    lv_style_set_text_color(&styleTextLabel_, button_primary_text());
}

Button Button::create(lv_obj_t* parent,
                      const char* text,
                      const char* symbol,
                      void (*onClick)(void),
                      lv_coord_t width,
                      lv_coord_t height)
{
    ensureStyles();

    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn,
        width  > 0 ? width  : Ui::Tokens::button_width(),
        height > 0 ? height : Ui::Tokens::button_height()
    );
    lv_obj_set_style_radius(btn, Ui::Tokens::button_radius(), 0);

    lv_obj_add_style(btn, &styleBtnBase_,    0);
    lv_obj_add_style(btn, &styleBtnPressed_, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &styleBtnFocused_, LV_STATE_FOCUSED);

    lv_obj_t* iconObj = nullptr;
    if (symbol && *symbol) {
        iconObj = lv_label_create(btn);
        lv_obj_add_style(iconObj, &styleIconLabel_, 0);
        lv_label_set_text(iconObj, symbol);
        lv_obj_align(iconObj, LV_ALIGN_LEFT_MID, 4, 0);
    }

    lv_obj_t* textObj = nullptr;
    if (text && *text) {
        textObj = lv_label_create(btn);
        lv_obj_add_style(textObj, &styleTextLabel_, 0);
        lv_label_set_text(textObj, text);
        if (iconObj) lv_obj_align_to(textObj, iconObj, LV_ALIGN_OUT_RIGHT_MID, Ui::Tokens::button_icon_gap(), 0);
        else         lv_obj_center(textObj);
    }

    if (onClick) {
        lv_obj_add_event_cb(
            btn,
            [](lv_event_t* e) {
                if (lv_event_get_code(e) == LV_EVENT_CLICKED) {
                    auto cb = reinterpret_cast<void (*)()>(lv_event_get_user_data(e));
                    if (cb) cb();
                }
            },
            LV_EVENT_ALL,
            (void*)onClick
        );
    }

    ESP_LOGD(TAG_BTN, "Button created (hasIcon=%d, hasText=%d)", iconObj != nullptr, textObj != nullptr);
    return Button(btn, iconObj, textObj);
}

void Button::setText(const char* txt)
{
    if (!textLabel_) return;
    lv_label_set_text(textLabel_, txt ? txt : "");
}

void Button::setEnabled(bool enabled)
{
    if (!btn_) return;
    if (enabled) lv_obj_clear_state(btn_, LV_STATE_DISABLED);
    else         lv_obj_add_state  (btn_, LV_STATE_DISABLED);
}
