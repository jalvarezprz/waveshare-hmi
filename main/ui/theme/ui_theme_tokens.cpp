#include "ui_theme_tokens.h"

extern "C" {
    extern const lv_font_t montserrat_20_lat1;
    extern const lv_font_t montserrat_18_lat1;
    extern const lv_font_t montserrat_16_lat1;
}

namespace Ui {

UiThemeTokens makeDefaultTokens() {
    UiThemeTokens t{};

    // Paleta clara (ajústala luego a tu gusto)
    t.colorBg        = lv_color_hex(0xF5F7FA);
    t.colorSurface   = lv_color_hex(0xFFFFFF);
    t.colorPrimary   = lv_color_hex(0x2E7D32); // verde
    t.colorSecondary = lv_color_hex(0x1565C0); // azul
    t.colorText      = lv_color_hex(0x202124);
    t.colorMuted     = lv_color_hex(0x9AA0A6);
    t.colorSuccess   = lv_color_hex(0x2E7D32);
    t.colorWarning   = lv_color_hex(0xF59E0B);
    t.colorError     = lv_color_hex(0xD32F2F);

    t.opaEnabled  = LV_OPA_COVER;
    t.opaDisabled = LV_OPA_50;

    t.radiusSm = 6;
    t.radiusMd = 10;
    t.radiusLg = 16;

    t.spaceXs = 4;
    t.spaceSm = 8;
    t.spaceMd = 12;
    t.spaceLg = 16;

    // Fuentes: por ahora usa la global; más adelante activamos Montserrat con acentos
    /*
    t.fontTitle   = LV_FONT_DEFAULT;
    t.fontBody    = LV_FONT_DEFAULT;
    t.fontCaption = LV_FONT_DEFAULT;
    */

    t.fontTitle   = &montserrat_18_lat1;
    t.fontBody    = &montserrat_18_lat1;
    t.fontCaption = &montserrat_16_lat1;   // <- usar 16 px para textos secundarios


    // Alturas estándar de ítems
    t.itemHeightMd = 32;    //Valores típicos: 32, 44, 56, 64
    t.itemHeightLg = 40;    //Valores típicos: 40, 52, 64, 72

    return t;
}

} // namespace Ui
