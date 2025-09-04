#pragma once
/**
 * @file ui_theme_tokens.h
 * @brief Tokens de diseño (header-only). Átomos neutrales: colores, opacidades,
 *        espaciados, radios, tipografía, densidad, motion, elevación y tamaños base.
 *
 * OBJETIVO
 *  - Definir átomos neutrales (sin recetas por componente ni estados).
 *  - Servir de entrada a un snapshot (UiThemeTokens) que ThemeStyles consumirá.
 *
 * NOTA DE COMPATIBILIDAD
 *  - Al final hay una sección marcada como DEPRECATED (compat) con alias temporales
 *    para no romper styles actuales. Elimina esa sección cuando migres ThemeStyles.
 */

#include "lvgl.h"
#include <stdint.h>   // usar tipos stdint portables (uint32_t, etc.)

namespace Ui { namespace Tokens {

/*────────────────────────────  UTILIDADES  ───────────────────────────*/

/** Convierte RGBA 0xRRGGBBAA a lv_color_t (RGB; alfa se ignora en LVGL). */
static inline lv_color_t rgba_to_lv(uint32_t rgba) {
    return lv_color_hex(rgba & 0x00FFFFFFu);
}

/*────────────────────────────  COLORES  ──────────────────────────────*/
/** Roles cromáticos semánticos (valores por defecto tipo “dark”). */
namespace Color {
    // Superficies
    inline constexpr uint32_t Surface        = 0x121212FF;
    inline constexpr uint32_t OnSurface      = 0xEDEDEDFF;
    inline constexpr uint32_t SurfaceVariant = 0x1E1E1EFF;
    inline constexpr uint32_t Outline        = 0x3C3C3CFF;
    inline constexpr uint32_t Muted          = 0x9E9E9EFF;

    // Acciones / marca
    inline constexpr uint32_t Primary        = 0x42A5F5FF;
    inline constexpr uint32_t OnPrimary      = 0xFFFFFFFF;

    inline constexpr uint32_t Secondary      = 0x9C27B0FF;
    inline constexpr uint32_t OnSecondary    = 0xFFFFFFFF;

    // Semáforo / estado
    inline constexpr uint32_t Success        = 0x4CAF50FF;
    inline constexpr uint32_t OnSuccess      = 0xFFFFFFFF;

    inline constexpr uint32_t Warning        = 0xFFC107FF;
    inline constexpr uint32_t OnWarning      = 0x212121FF;

    inline constexpr uint32_t Error          = 0xF44336FF;
    inline constexpr uint32_t OnError        = 0xFFFFFFFF;

    // Overlay
    inline constexpr uint32_t OverlayBg      = 0x000000FF;
}

/** Helpers LVGL (conversión a lv_color_t). */
inline lv_color_t color_surface()         { return rgba_to_lv(Color::Surface); }
inline lv_color_t color_on_surface()      { return rgba_to_lv(Color::OnSurface); }
inline lv_color_t color_surface_variant() { return rgba_to_lv(Color::SurfaceVariant); }
inline lv_color_t color_outline()         { return rgba_to_lv(Color::Outline); }
inline lv_color_t color_muted()           { return rgba_to_lv(Color::Muted); }

inline lv_color_t color_primary()         { return rgba_to_lv(Color::Primary); }
inline lv_color_t color_on_primary()      { return rgba_to_lv(Color::OnPrimary); }
inline lv_color_t color_secondary()       { return rgba_to_lv(Color::Secondary); }
inline lv_color_t color_on_secondary()    { return rgba_to_lv(Color::OnSecondary); }

inline lv_color_t color_success()         { return rgba_to_lv(Color::Success); }
inline lv_color_t color_on_success()      { return rgba_to_lv(Color::OnSuccess); }
inline lv_color_t color_warning()         { return rgba_to_lv(Color::Warning); }
inline lv_color_t color_on_warning()      { return rgba_to_lv(Color::OnWarning); }
inline lv_color_t color_error()           { return rgba_to_lv(Color::Error); }
inline lv_color_t color_on_error()        { return rgba_to_lv(Color::OnError); }

inline lv_color_t color_overlay_bg()      { return rgba_to_lv(Color::OverlayBg); }

/*──────────────────────────  OPACIDADES  ─────────────────────────────*/
namespace Opa {
    inline constexpr lv_opa_t Enabled  = LV_OPA_COVER;
    inline constexpr lv_opa_t Disabled = LV_OPA_50;
    inline constexpr lv_opa_t Hover    = LV_OPA_20;
    inline constexpr lv_opa_t Pressed  = LV_OPA_30;
    inline constexpr lv_opa_t Focus    = LV_OPA_COVER;
    inline constexpr lv_opa_t Overlay  = LV_OPA_60;
}
inline lv_opa_t opa_enabled()   { return Opa::Enabled;  }
inline lv_opa_t opa_disabled()  { return Opa::Disabled; }
inline lv_opa_t opa_hover()     { return Opa::Hover;    }
inline lv_opa_t opa_pressed()   { return Opa::Pressed;  }
inline lv_opa_t opa_focus()     { return Opa::Focus;    }
inline lv_opa_t opa_overlay()   { return Opa::Overlay;  }

/*─────────────────────────  ESPACIADO / SHAPE  ───────────────────────*/
namespace Space  { inline constexpr int xs=4, sm=8, md=12, lg=16, xl=24; }
namespace Radius { inline constexpr int none=0, sm=4, md=8, lg=12, pill=999; }

/** Gaps globales para layouts declarativos. */
namespace LayoutGaps { inline constexpr int Row = Space::sm; inline constexpr int Column = Space::sm; }
inline lv_coord_t gap_row()    { return static_cast<lv_coord_t>(LayoutGaps::Row); }
inline lv_coord_t gap_column() { return static_cast<lv_coord_t>(LayoutGaps::Column); }

/** Densidad / accesibilidad. */
namespace Density { inline constexpr int MinTouch = 44; } // px
inline lv_coord_t min_touch() { return static_cast<lv_coord_t>(Density::MinTouch); }

/*────────────────────────────  TIPOGRAFÍA  ───────────────────────────*/
#if defined(UI_HAVE_MONTSERRAT_LAT1)
extern "C" {
    LV_FONT_DECLARE(montserrat_16_lat1)
    LV_FONT_DECLARE(montserrat_14_lat1)
    LV_FONT_DECLARE(montserrat_12_lat1)
}
inline const lv_font_t* font_title()   { return &montserrat_16_lat1; }
inline const lv_font_t* font_body()    { return &montserrat_14_lat1; }
inline const lv_font_t* font_caption() { return &montserrat_12_lat1; }
#else
inline const lv_font_t* font_title()   { return LV_FONT_DEFAULT; }
inline const lv_font_t* font_body()    { return LV_FONT_DEFAULT; }
inline const lv_font_t* font_caption() { return LV_FONT_DEFAULT; }
#endif

#if defined(UI_HAVE_ICON_FONT)
extern "C" { LV_FONT_DECLARE(ui_icon_font_16) }
inline const lv_font_t* font_icon() { return &ui_icon_font_16; }
#else
inline const lv_font_t* font_icon() { return LV_FONT_DEFAULT; }
#endif

/*────────────────────────────  FOCUS RING  ───────────────────────────*/
namespace Focus {
    inline constexpr int OutlineW   = 2;
    inline constexpr int OutlinePad = 2;
}
inline lv_coord_t focus_outline_w()    { return static_cast<lv_coord_t>(Focus::OutlineW); }
inline lv_coord_t focus_outline_pad()  { return static_cast<lv_coord_t>(Focus::OutlinePad); }
inline lv_color_t focus_outline_color(){ return color_primary(); }

/*──────────────────────────────  MOTION  ─────────────────────────────*/
namespace Motion {
    inline constexpr uint16_t Fast = 120;
    inline constexpr uint16_t Std  = 180;
    inline constexpr uint16_t Slow = 240;
}

/*────────────────────────────  ELEVACIÓN  ────────────────────────────*/
namespace Elevation {
    inline constexpr int ShadowW0 = 0;
    inline constexpr int ShadowW1 = 6;
    inline constexpr int ShadowW2 = 12;
    inline constexpr int OfsY1    = 2;
    inline constexpr int OfsY2    = 4;
    inline lv_color_t shadow_color() { return lv_color_black(); }
}

/*──────────────────────  OVERLAY / DIALOG / TOAST  ───────────────────*/
namespace OverlayTok {
    inline constexpr int  PadAll = Space::md;
    inline constexpr int  Radius = Radius::lg;
}
inline lv_color_t overlay_bg_color() { return color_overlay_bg(); }
inline lv_opa_t   overlay_bg_opa()   { return opa_overlay(); }

namespace ToastTok {
    inline constexpr int  PadAll    = Space::sm;
    inline constexpr int  Radius    = Radius::md;
    inline constexpr int  TimeoutMs = 1800;
}

/*────────────────────────  CONTROLES: GEOMETRÍA  ─────────────────────*/
namespace ButtonTok {
    inline constexpr int Width   = 160;
    inline constexpr int Height  = 44;
    inline constexpr int Radius  = 12;
    inline constexpr int PadLR   = 16;
    inline constexpr int PadTB   = 8;
    inline constexpr int IconGap = 8;
}
inline lv_coord_t button_width()    { return static_cast<lv_coord_t>(ButtonTok::Width); }
inline lv_coord_t button_height()   { return static_cast<lv_coord_t>(ButtonTok::Height); }
inline lv_coord_t button_radius()   { return static_cast<lv_coord_t>(ButtonTok::Radius); }
inline lv_coord_t button_pad_lr()   { return static_cast<lv_coord_t>(ButtonTok::PadLR); }
inline lv_coord_t button_pad_tb()   { return static_cast<lv_coord_t>(ButtonTok::PadTB); }
inline lv_coord_t button_icon_gap() { return static_cast<lv_coord_t>(ButtonTok::IconGap); }

namespace SwitchTok {
    inline constexpr int TrackW = 50;
    inline constexpr int TrackH = 28;
    inline constexpr int Knob   = 22; // diámetro
}
namespace SliderTok {
    inline constexpr int TrackH = 8;
    inline constexpr int Knob   = 18;
}

/*────────────────────────  SUPERFICIES: ÁTOMOS  ──────────────────────*/
namespace PanelTok {
    inline constexpr int PadAll  = Space::sm;
    inline constexpr int BorderW = 1;
}
namespace CardTok {
    inline constexpr int Radius     = Radius::md;
    inline constexpr int PadAll     = Space::md;
    inline constexpr int BorderW    = 1;
    inline constexpr int ShadowW    = Elevation::ShadowW1;
    inline constexpr int ShadowOfsY = Elevation::OfsY1;
}
namespace ListTok {
    inline constexpr int ItemHeightMd = 56;
    inline constexpr int ItemHeightLg = 64;
    inline constexpr int PadLR        = 16;
    inline constexpr int PadTB        = 8;
    inline constexpr int GapRow       = 8;
    inline constexpr int DividerWidth = 1;
    inline constexpr lv_opa_t DividerOpa = LV_OPA_40;
}

/* Helpers de medida/colores para superficies y listas (neutros) */
inline lv_coord_t panel_pad_all()      { return static_cast<lv_coord_t>(PanelTok::PadAll); }
inline lv_coord_t panel_border_w()     { return static_cast<lv_coord_t>(PanelTok::BorderW); }

inline lv_coord_t card_radius()        { return static_cast<lv_coord_t>(CardTok::Radius); }
inline lv_coord_t card_pad_all()       { return static_cast<lv_coord_t>(CardTok::PadAll); }
inline lv_coord_t card_border_w()      { return static_cast<lv_coord_t>(CardTok::BorderW); }
inline lv_coord_t card_shadow_w()      { return static_cast<lv_coord_t>(CardTok::ShadowW); }
inline lv_coord_t card_shadow_ofs_y()  { return static_cast<lv_coord_t>(CardTok::ShadowOfsY); }
inline lv_color_t card_shadow_color()  { return Elevation::shadow_color(); }

inline lv_coord_t list_item_height_md(){ return static_cast<lv_coord_t>(ListTok::ItemHeightMd); }
inline lv_coord_t list_item_height_lg(){ return static_cast<lv_coord_t>(ListTok::ItemHeightLg); }
inline lv_coord_t list_pad_lr()        { return static_cast<lv_coord_t>(ListTok::PadLR); }
inline lv_coord_t list_pad_tb()        { return static_cast<lv_coord_t>(ListTok::PadTB); }
inline lv_coord_t list_gap_row()       { return static_cast<lv_coord_t>(ListTok::GapRow); }
inline lv_coord_t list_divider_width() { return static_cast<lv_coord_t>(ListTok::DividerWidth); }
inline lv_opa_t   list_divider_opa()   { return ListTok::DividerOpa; }

inline lv_color_t panel_bg_color()     { return color_surface_variant(); }
inline lv_color_t panel_text_color()   { return color_on_surface(); }
inline lv_color_t panel_border_color() { return color_outline(); }

inline lv_color_t card_bg_color()      { return color_surface(); }
inline lv_color_t card_text_color()    { return color_on_surface(); }
inline lv_color_t card_border_color()  { return color_outline(); }

inline lv_color_t list_bg_color()      { return color_surface(); }
inline lv_color_t list_item_bg_color() { return color_surface(); }
inline lv_color_t list_text_color()    { return color_on_surface(); }

/*────────────────────────────  FIN TOKENS  ───────────────────────────*/

}} // namespace Ui::Tokens
