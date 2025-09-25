#pragma once
#include "lvgl.h"
#include "ui/theme/ui_theme_styles.h"   // Ui::getThemeStyles, applyButton*, applyIcon*
#include <functional>

namespace Ui {

/**
 * Botón de alto nivel con:
 *  - Texto + icono opcional
 *  - onClick callback
 *  - Enabled/disabled
 *  - Aplicación de tema (tokens/styles) para colores y tipografía
 *
 * Incorpora:
 *  - Variantes (Primary/Secondary/Ghost/Destructive/Success/Warning)
 *  - Factory para botón “Back” con valores por defecto
 */
class Button {
public:
    enum class Variant {
        Primary,
        Secondary,
        Ghost,
        Destructive,
        Success,
        Warning
    };

    Button() = default;
    ~Button() = default;

    /** Crea el botón (root = lv_btn) y aplica layout + theme */
    void create(lv_obj_t* parent);

    /** Devuelve el objeto raíz (para layouts de la vista) */
    lv_obj_t* root() const { return root_; }

    /** Texto (seguro con UTF-8; depende de la fuente cargada) */
    void setText(const char* txt);

    /** Icono (texto: LV_SYMBOL_*, "", "↩", etc.). nullptr o "" para quitarlo */
    void setIcon(const char* iconTxt);

    /** Coloca el icono a la izquierda (true) o a la derecha (false) del texto */
    void setIconLeft(bool left);

    /** Habilita/inhabilita el botón */
    void setEnabled(bool en);
    bool isEnabled() const;

    /** Callback de click */
    void setOnClick(void (*cb)(void*), void* user_data);

    /** Cambia la variante visual y reaplica el theme */
    void setVariant(Variant v);

    /** Reaplica el tema (colores, tipografía). Útil si recargas Theme */
    virtual void applyTheme();

    /*----------------- Factory: Back -----------------*/
    struct BackProps {
        // Nota: usar UTF-8 explícito para evitar problemas de codificación en Windows
        const char* text     = "Atr\xc3\xa1s";   // "Atrás"
        bool        showText = true;
        bool        iconLeft = true;
        bool        enabled  = true;
        lv_align_t  align    = LV_ALIGN_LEFT_MID;
        lv_coord_t  ofsX     = 8;
        lv_coord_t  ofsY     = 0;
        void (*onClick)(void*) = nullptr;
        void* user = nullptr;
    };

    // Overload sin props (usa todos los defaults)
    static Button* CreateBack(lv_obj_t* parent);
    // Overload con props
    static Button* CreateBack(lv_obj_t* parent, const BackProps& p);

private:
    // Objetos LVGL
    lv_obj_t* root_  = nullptr;
    lv_obj_t* label_ = nullptr;
    lv_obj_t* icon_  = nullptr;

    // Estado
    bool    iconLeft_ = true;
    Variant variant_  = Variant::Primary;

    // Callback
    void (*onClick_)(void*) = nullptr;
    void* userData_ = nullptr;

    // Eventos
    void registerAllEvents();
    static void on_event_cb(lv_event_t* e);
    void onEvent(lv_event_t* e);

    // Aplica la variante actual
    void applyVariantStyles();
};

} // namespace Ui
