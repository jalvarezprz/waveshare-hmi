#include "ui/theme/ui_theme_tokens.h"
#include "ui/theme/ui_theme_styles.h"   // acceso a Ui::getThemeTokens()
#include <algorithm>

namespace Ui { namespace Tokens {

// --- util: clamp y shade ---
static inline uint8_t clampi(int v){ return (uint8_t)std::max(0, std::min(255, v)); }
static lv_color_t shade(lv_color_t c, int delta) {
    int r = clampi((int)c.ch.red   + delta);
    int g = clampi((int)c.ch.green + delta);
    int b = clampi((int)c.ch.blue  + delta);
    return lv_color_make(r, g, b);
}

static lv_color_t best_on_color(lv_color_t bg) {
    const int r = bg.ch.red, g = bg.ch.green, b = bg.ch.blue;
    const int L = (299*r + 587*g + 114*b) / 1000;
    return (L < 128) ? lv_color_white() : lv_color_black();
}

// ------------- Colores -------------
lv_color_t button_primary_bg() {
    auto &t = Ui::getThemeTokens();
    return t.colorPrimary;
}
lv_color_t button_primary_bg_pressed() {
    return shade(button_primary_bg(), -28);
}
lv_color_t button_primary_bg_focused() {
    return shade(button_primary_bg(), +18);
}
lv_color_t button_primary_text() {
    return best_on_color(button_primary_bg());
}

// ------------- Fuentes -------------
const lv_font_t* font_icon() {
    const lv_font_t* f = lv_theme_get_font_small(nullptr);
    return f ? f : lv_theme_get_font_normal(nullptr);
}
const lv_font_t* font_text() {
    auto &t = Ui::getThemeTokens();
    if (t.fontBody) return t.fontBody;
    const lv_font_t* f = lv_theme_get_font_large(nullptr);
    return f ? f : lv_theme_get_font_normal(nullptr);
}

// ------------- Medidas -------------
lv_coord_t button_width()    { return 120; }
lv_coord_t button_height()   { return 44;  }
lv_coord_t button_radius()   { return 12;  }
lv_coord_t button_pad_lr()   { return 10;  }
lv_coord_t button_pad_tb()   { return 6;   }
lv_coord_t button_icon_gap() { return 8;   }

}} // namespace Ui::Tokens
