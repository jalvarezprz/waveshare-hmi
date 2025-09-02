#include "ui/component/ui_component_button.h"
#include "esp_log.h"
#include "ui/theme/ui_theme_styles.h"   // ← usamos tus tokens para las fuentes/colores

static const char* TAG_BTN = "UI/Button";

// ── Estáticos
bool Button::stylesInited_ = false;

lv_style_t Button::styleBtnBase_;
lv_style_t Button::styleBtnPressed_;
lv_style_t Button::styleBtnFocused_;

lv_style_t Button::styleIconLabel_;
lv_style_t Button::styleTextLabel_;

// ── Inicialización perezosa de estilos (una sola vez)
void Button::ensureStyles()
{
    if (stylesInited_) return;
    stylesInited_ = true;

    // --- Botón base (azul claro + texto blanco) ---
    lv_style_init(&styleBtnBase_);
    lv_style_set_bg_color(&styleBtnBase_, lv_color_hex(0x42A5F5)); // azul claro
    lv_style_set_border_width(&styleBtnBase_, 0);
    lv_style_set_pad_left(&styleBtnBase_, 10);
    lv_style_set_pad_right(&styleBtnBase_, 10);
    lv_style_set_pad_top(&styleBtnBase_, 6);
    lv_style_set_pad_bottom(&styleBtnBase_, 6);
    lv_style_set_text_color(&styleBtnBase_, lv_color_white());
    // (sin transición por compatibilidad con tu LVGL)

    // --- Botón pressed / focused ---
    lv_style_init(&styleBtnPressed_);
    lv_style_set_bg_color(&styleBtnPressed_, lv_color_hex(0x1E88E5));

    lv_style_init(&styleBtnFocused_);
    lv_style_set_bg_color(&styleBtnFocused_, lv_color_hex(0x64B5F6));

    // --- Fuentes del theme actual (si hay tema cargado) ---
    const lv_font_t* font_small  = lv_theme_get_font_small(nullptr);
    const lv_font_t* font_normal = lv_theme_get_font_normal(nullptr);
    const lv_font_t* font_large  = lv_theme_get_font_large(nullptr);

    // --- Label icono ---
    lv_style_init(&styleIconLabel_);
    // Usamos la "small" del theme; si el theme incluye símbolos, LV_SYMBOL_* se verá.
    // Si no, luego cambiamos a imagen cuando toquemos presets/tokens.
    lv_style_set_text_font(&styleIconLabel_, font_small ? font_small : font_normal);
    lv_style_set_text_color(&styleIconLabel_, lv_color_white());

    // --- Label texto ---
    lv_style_init(&styleTextLabel_);
    // Preferimos la "large" del theme; si no hay, usamos la normal.
    lv_style_set_text_font(&styleTextLabel_, font_large ? font_large : font_normal);
    lv_style_set_text_color(&styleTextLabel_, lv_color_white());
}


// ── Creación en una línea
Button Button::create(lv_obj_t* parent,
                      const char* text,
                      const char* symbol,
                      void (*onClick)(void),
                      lv_coord_t width,
                      lv_coord_t height)
{
    ensureStyles();

    // Botón
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_set_size(btn, width, height);
    lv_obj_set_style_radius(btn, 12, 0);

    // Estilos por estado (sin OR de enums → sin warnings)
    lv_obj_add_style(btn, &styleBtnBase_,    0);
    lv_obj_add_style(btn, &styleBtnPressed_, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &styleBtnFocused_, LV_STATE_FOCUSED);

    // Icono opcional (símbolo LVGL)
    lv_obj_t* iconObj = nullptr;
    if (symbol && *symbol) {
        iconObj = lv_label_create(btn);
        lv_obj_add_style(iconObj, &styleIconLabel_, 0);
        lv_label_set_text(iconObj, symbol);
        lv_obj_align(iconObj, LV_ALIGN_LEFT_MID, 4, 0);
    }

    // Texto opcional
    lv_obj_t* textObj = nullptr;
    if (text && *text) {
        textObj = lv_label_create(btn);
        lv_obj_add_style(textObj, &styleTextLabel_, 0);
        lv_label_set_text(textObj, text);

        if (iconObj) {
            lv_obj_align_to(textObj, iconObj, LV_ALIGN_OUT_RIGHT_MID, 8, 0);
        } else {
            lv_obj_center(textObj);
        }
    }

    // Callback
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

// ── Métodos de instancia
void Button::setText(const char* txt)
{
    if (!textLabel_) return;
    lv_label_set_text(textLabel_, txt ? txt : "");
}

void Button::setEnabled(bool enabled)
{
    if (!btn_) return;
    if (enabled) lv_obj_clear_state(btn_, LV_STATE_DISABLED);
    else         lv_obj_add_state(btn_,   LV_STATE_DISABLED);
}
