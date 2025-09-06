/**
 * @file ui_theme_styles.cpp
 * @brief Implementación de ThemeStyles construido desde Tokens (snapshot).
 */

#include "ui_theme_styles.h"
#include "ui_theme_tokens.h"   // Solo para construir el snapshot una vez

namespace Ui {

/*========================== Estado global =============================*/
static UiThemeTokens  g_tokens;
static UiThemeStyles  g_styles;

/*========================== Helpers internos ==========================*/
static void style_reset(lv_style_t& s) {
    if (s.prop_cnt) lv_style_reset(&s);
    else            lv_style_init(&s);
}

/** Selector tipado (OR de part/state sin warnings). */
static inline lv_style_selector_t sel(lv_part_t part, lv_state_t state) {
    return static_cast<lv_style_selector_t>(part | state);
}

/* Mezclas de color para estados (usando snapshot, no Tokens directos) */
static inline lv_color_t mix_pressed(lv_color_t base, const UiThemeTokens& t) {
    // "Hundido": mezcla contra Surface con opacidad de pressed
    return lv_color_mix(t.colorSurface, base, t.opaPressed);
}
static inline lv_color_t mix_hover(lv_color_t base, const UiThemeTokens& t) {
    return lv_color_mix(base, t.colorSurface, t.opaHover);
}

/*========================== Snapshot desde Tokens =====================*/
static UiThemeTokens makeSnapshotFromTokens() {
    UiThemeTokens t{};

    // Colores (roles)
    t.colorSurface         = Tokens::color_surface();
    t.colorOnSurface       = Tokens::color_on_surface();
    t.colorSurfaceVariant  = Tokens::color_surface_variant();
    t.colorOutline         = Tokens::color_outline();
    t.colorMuted           = Tokens::color_muted();

    t.colorPrimary         = Tokens::color_primary();
    t.colorOnPrimary       = Tokens::color_on_primary();
    t.colorSecondary       = Tokens::color_secondary();
    t.colorOnSecondary     = Tokens::color_on_secondary();

    t.colorSuccess         = Tokens::color_success();
    t.colorOnSuccess       = Tokens::color_on_success();
    t.colorWarning         = Tokens::color_warning();
    t.colorOnWarning       = Tokens::color_on_warning();
    t.colorError           = Tokens::color_error();
    t.colorOnError         = Tokens::color_on_error();

    t.colorOverlayBg       = Tokens::color_overlay_bg();

    // Opacidades
    t.opaEnabled  = Tokens::opa_enabled();
    t.opaDisabled = Tokens::opa_disabled();
    t.opaHover    = Tokens::opa_hover();
    t.opaPressed  = Tokens::opa_pressed();
    t.opaFocus    = Tokens::opa_focus();
    t.opaOverlay  = Tokens::opa_overlay();

    // Tipografía
    t.fontTitle   = Tokens::font_title();
    t.fontBody    = Tokens::font_body();
    t.fontCaption = Tokens::font_caption();
    t.fontIcon    = Tokens::font_icon();

    // Focus / layout / density
    t.focusOutlineW     = Tokens::focus_outline_w();
    t.focusOutlinePad   = Tokens::focus_outline_pad();
    t.focusOutlineColor = Tokens::focus_outline_color();

    t.gapRow            = Tokens::gap_row();
    t.gapCol            = Tokens::gap_column();
    t.minTouch          = Tokens::min_touch();

    // Superficies / listas
    t.panelPadAll       = Tokens::panel_pad_all();
    t.panelBorderW      = Tokens::panel_border_w();

    t.cardRadius        = Tokens::card_radius();
    t.cardPadAll        = Tokens::card_pad_all();
    t.cardBorderW       = Tokens::card_border_w();
    t.cardShadowW       = Tokens::card_shadow_w();
    t.cardShadowOfsY    = Tokens::card_shadow_ofs_y();
    t.cardShadowColor   = Tokens::card_shadow_color();

    t.listItemH_md      = Tokens::list_item_height_md();
    t.listItemH_lg      = Tokens::list_item_height_lg();
    t.listPadLR         = Tokens::list_pad_lr();
    t.listPadTB         = Tokens::list_pad_tb();
    t.listGapRow        = Tokens::list_gap_row();
    t.listDividerW      = Tokens::list_divider_width();
    t.listDividerOpa    = Tokens::list_divider_opa();

    // Controles
    t.btnWidth          = Tokens::button_width();
    t.btnHeight         = Tokens::button_height();
    t.btnRadius         = Tokens::button_radius();
    t.btnPadLR          = Tokens::button_pad_lr();
    t.btnPadTB          = Tokens::button_pad_tb();
    t.btnIconGap        = Tokens::button_icon_gap();

    return t;
}

/*========================== Construcción de styles =====================*/
static void initThemeStyles(UiThemeStyles& s, const UiThemeTokens& t) {
    s.tokens = t;

    /* ===== Base ===== */
    style_reset(s.base);
    lv_style_set_bg_color  (&s.base, t.colorSurface);
    lv_style_set_text_color(&s.base, t.colorOnSurface);

    /* ===== Header/Footer ===== */
    style_reset(s.header);
    lv_style_set_bg_color    (&s.header, t.colorSurfaceVariant);
    lv_style_set_text_color  (&s.header, t.colorOnSurface);
    lv_style_set_pad_all     (&s.header, t.panelPadAll);
    lv_style_set_border_width(&s.header, t.panelBorderW);
    lv_style_set_border_color(&s.header, t.colorOutline);

    s.footer = s.header; // mismas reglas

    /* ===== Content ===== */
    style_reset(s.content);
    lv_style_set_bg_color(&s.content, t.colorSurface);
    lv_style_set_pad_all (&s.content, t.panelPadAll);

    /* ===== Card ===== */
    style_reset(s.card);
    lv_style_set_bg_color     (&s.card, t.colorSurface);
    lv_style_set_text_color   (&s.card, t.colorOnSurface);
    lv_style_set_radius       (&s.card, t.cardRadius);
    lv_style_set_pad_all      (&s.card, t.cardPadAll);
    lv_style_set_border_width (&s.card, t.cardBorderW);
    lv_style_set_border_color (&s.card, t.colorOutline);
    lv_style_set_shadow_width (&s.card, t.cardShadowW);
    lv_style_set_shadow_ofs_y (&s.card, t.cardShadowOfsY);
    lv_style_set_shadow_color (&s.card, t.cardShadowColor);

    /* ===== Labels ===== */
    style_reset(s.labelTitle);
    lv_style_set_text_font (&s.labelTitle, t.fontTitle);
    lv_style_set_text_color(&s.labelTitle, t.colorOnSurface);

    style_reset(s.labelBody);
    lv_style_set_text_font (&s.labelBody, t.fontBody);
    lv_style_set_text_color(&s.labelBody, t.colorOnSurface);

    style_reset(s.labelCaption);
    lv_style_set_text_font (&s.labelCaption, t.fontCaption);
    lv_style_set_text_color(&s.labelCaption, t.colorMuted);

    /* ===== Listas ===== */
    style_reset(s.listContainer);
    lv_style_set_bg_color (&s.listContainer, t.colorSurface);
    lv_style_set_pad_ver  (&s.listContainer, t.listPadTB);
    lv_style_set_pad_hor  (&s.listContainer, t.listPadLR);

    style_reset(s.listItem);
    lv_style_set_bg_color  (&s.listItem, t.colorSurface);
    lv_style_set_bg_opa    (&s.listItem, t.opaEnabled);
    lv_style_set_radius    (&s.listItem, 4);
    lv_style_set_pad_left  (&s.listItem, t.listPadLR);
    lv_style_set_pad_right (&s.listItem, t.listPadLR);
    lv_style_set_pad_top   (&s.listItem, t.listPadTB);
    lv_style_set_pad_bottom(&s.listItem, t.listPadTB);
    lv_style_set_text_color(&s.listItem, t.colorOnSurface);
    lv_style_set_text_font (&s.listItem, t.fontBody);

    style_reset(s.listItemPressed);
    lv_style_set_bg_color(&s.listItemPressed, mix_pressed(t.colorSurface, t));

    style_reset(s.listItemFocused);
    lv_style_set_outline_width(&s.listItemFocused, t.focusOutlineW);
    lv_style_set_outline_color(&s.listItemFocused, t.focusOutlineColor);
    lv_style_set_outline_pad (&s.listItemFocused, t.focusOutlinePad);

    style_reset(s.listItemDisabled);
    lv_style_set_bg_opa    (&s.listItemDisabled, t.opaDisabled);
    lv_style_set_text_color(&s.listItemDisabled, t.colorMuted);

    style_reset(s.listDivider);
    lv_style_set_border_width(&s.listDivider, t.listDividerW);
    lv_style_set_border_side (&s.listDivider, LV_BORDER_SIDE_BOTTOM);
    lv_style_set_border_color(&s.listDivider, t.colorOutline);
    lv_style_set_border_opa  (&s.listDivider, t.listDividerOpa);

    /* ===== Botones (bases) ===== */
    style_reset(s.btnPrimary);
    lv_style_set_bg_color (&s.btnPrimary, t.colorPrimary);
    lv_style_set_bg_opa   (&s.btnPrimary, t.opaEnabled);
    lv_style_set_radius   (&s.btnPrimary, t.btnRadius);
    lv_style_set_pad_hor  (&s.btnPrimary, t.btnPadLR);
    lv_style_set_pad_ver  (&s.btnPrimary, t.btnPadTB);
    lv_style_set_text_color(&s.btnPrimary, t.colorOnPrimary);

    style_reset(s.btnSecondary);
    lv_style_set_bg_color (&s.btnSecondary, t.colorSecondary);
    lv_style_set_bg_opa   (&s.btnSecondary, t.opaEnabled);
    lv_style_set_radius   (&s.btnSecondary, t.btnRadius);
    lv_style_set_pad_hor  (&s.btnSecondary, t.btnPadLR);
    lv_style_set_pad_ver  (&s.btnSecondary, t.btnPadTB);
    lv_style_set_text_color(&s.btnSecondary, t.colorOnSecondary);

    style_reset(s.btnGhost);
    lv_style_set_bg_opa     (&s.btnGhost, LV_OPA_TRANSP);
    lv_style_set_border_width(&s.btnGhost, t.panelBorderW);
    lv_style_set_border_color(&s.btnGhost, t.colorOutline);
    lv_style_set_radius      (&s.btnGhost, t.btnRadius);
    lv_style_set_pad_hor     (&s.btnGhost, t.btnPadLR);
    lv_style_set_pad_ver     (&s.btnGhost, t.btnPadTB);
    lv_style_set_text_color  (&s.btnGhost, t.colorOnSurface);

    s.initialized = true;
}

/*========================== API pública ===============================*/
UiThemeStyles&       getThemeStyles() { return g_styles; }
const UiThemeTokens& getThemeTokens() { return g_tokens; }

void themeInitOnce() {
    if (!g_styles.initialized) {
        g_tokens = makeSnapshotFromTokens();
        initThemeStyles(g_styles, g_tokens);
    }
}

/*========================== Aplicadores públicos ======================*/

void applyHeader (lv_obj_t* obj, UiThemeStyles& s) { if (obj) lv_obj_add_style(obj, &s.header,  LV_PART_MAIN); }
void applyContent(lv_obj_t* obj, UiThemeStyles& s) { if (obj) lv_obj_add_style(obj, &s.content, LV_PART_MAIN); }
void applyFooter (lv_obj_t* obj, UiThemeStyles& s) { if (obj) lv_obj_add_style(obj, &s.footer,  LV_PART_MAIN); }

/* ---- List helpers ---- */
static void set_row_height(lv_obj_t* obj, lv_coord_t h) {
    lv_obj_set_style_min_height(obj, h, LV_PART_MAIN);
    lv_obj_set_height(obj, LV_SIZE_CONTENT);
    lv_obj_set_width(obj, LV_PCT(100));
}

void applyListContainer(lv_obj_t* obj, UiThemeStyles& s) {
    if (!obj) return;
    lv_obj_add_style(obj, &s.listContainer, LV_PART_MAIN);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(obj, s.tokens.listGapRow, LV_PART_MAIN);
    lv_obj_set_width(obj, LV_PCT(100));
}

void applyListItem(lv_obj_t* obj, UiThemeStyles& s, bool large, bool withDivider) {
    if (!obj) return;

    const lv_coord_t H = large ? s.tokens.listItemH_lg : s.tokens.listItemH_md;

    lv_obj_add_style(obj, &s.listItem, LV_PART_MAIN);
    lv_obj_add_style(obj, &s.listItemPressed, LV_STATE_PRESSED);
    lv_obj_add_style(obj, &s.listItemFocused,  LV_STATE_FOCUSED | LV_STATE_EDITED);
    lv_obj_add_style(obj, &s.listItemDisabled, LV_STATE_DISABLED);

    if (withDivider) {
        lv_obj_add_style(obj, &s.listDivider, LV_PART_MAIN);
    }

    set_row_height(obj, H);

    lv_obj_set_flex_flow (obj, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
}

/*========================== Botones: variantes ========================*/

static inline void apply_btn_base_and_size(lv_obj_t* btn, lv_style_t* style, const UiThemeTokens& t, bool setSize) {
    if (!btn || !style) return;
    lv_obj_add_style(btn, style, LV_PART_MAIN);
    if (setSize) {
        lv_obj_set_size(btn, t.btnWidth, t.btnHeight);
    }
}

void applyButtonPrimary(lv_obj_t* btn, UiThemeStyles& s, bool setSize) {
    apply_btn_base_and_size(btn, &s.btnPrimary, s.tokens, setSize);

    // PRESSED
    lv_obj_set_style_bg_color(btn, mix_pressed(s.tokens.colorPrimary, s.tokens),
                              sel(LV_PART_MAIN, LV_STATE_PRESSED));

    // FOCUS
    lv_obj_set_style_outline_width(btn, s.tokens.focusOutlineW, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_color(btn, s.tokens.focusOutlineColor, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_pad  (btn, s.tokens.focusOutlinePad, sel(LV_PART_MAIN, LV_STATE_FOCUSED));

    // DISABLED
    lv_obj_set_style_bg_opa(btn, s.tokens.opaDisabled, sel(LV_PART_MAIN, LV_STATE_DISABLED));
}

void applyButtonSecondary(lv_obj_t* btn, UiThemeStyles& s, bool setSize) {
    apply_btn_base_and_size(btn, &s.btnSecondary, s.tokens, setSize);

    lv_obj_set_style_bg_color(btn, mix_pressed(s.tokens.colorSecondary, s.tokens),
                              sel(LV_PART_MAIN, LV_STATE_PRESSED));

    lv_obj_set_style_outline_width(btn, s.tokens.focusOutlineW, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_color(btn, s.tokens.focusOutlineColor, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_pad  (btn, s.tokens.focusOutlinePad, sel(LV_PART_MAIN, LV_STATE_FOCUSED));

    lv_obj_set_style_bg_opa(btn, s.tokens.opaDisabled, sel(LV_PART_MAIN, LV_STATE_DISABLED));
}

void applyButtonGhost(lv_obj_t* btn, UiThemeStyles& s, bool setSize) {
    apply_btn_base_and_size(btn, &s.btnGhost, s.tokens, setSize);

    // PRESSED: tinte suave del surface
    lv_obj_set_style_bg_opa  (btn, s.tokens.opaPressed, sel(LV_PART_MAIN, LV_STATE_PRESSED));
    lv_obj_set_style_bg_color(btn, lv_color_mix(s.tokens.colorOnSurface,
                                                s.tokens.colorSurface,
                                                s.tokens.opaPressed),
                              sel(LV_PART_MAIN, LV_STATE_PRESSED));

    // FOCUS
    lv_obj_set_style_outline_width(btn, s.tokens.focusOutlineW, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_color(btn, s.tokens.focusOutlineColor, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_pad  (btn, s.tokens.focusOutlinePad, sel(LV_PART_MAIN, LV_STATE_FOCUSED));

    lv_obj_set_style_text_font(btn, s.tokens.fontBody, LV_PART_MAIN);
}

/* ---- NUEVAS variantes movidas desde el componente ---- */

void applyButtonDestructive(lv_obj_t* btn, UiThemeStyles& s, bool setSize) {
    // Base: geometría de Secondary (mismo padding/radius), recoloreado a Error
    apply_btn_base_and_size(btn, &s.btnSecondary, s.tokens, setSize);

    // MAIN
    lv_obj_set_style_bg_color  (btn, s.tokens.colorError,    LV_PART_MAIN);
    lv_obj_set_style_text_color(btn, s.tokens.colorOnError,  LV_PART_MAIN);

    // PRESSED
    lv_obj_set_style_bg_color(btn, mix_pressed(s.tokens.colorError, s.tokens),
                              sel(LV_PART_MAIN, LV_STATE_PRESSED));

    // FOCUS (anillo común)
    lv_obj_set_style_outline_width(btn, s.tokens.focusOutlineW, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_color(btn, s.tokens.focusOutlineColor, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_pad  (btn, s.tokens.focusOutlinePad, sel(LV_PART_MAIN, LV_STATE_FOCUSED));

    // DISABLED
    lv_obj_set_style_bg_opa(btn, s.tokens.opaDisabled, sel(LV_PART_MAIN, LV_STATE_DISABLED));
}

void applyButtonSuccess(lv_obj_t* btn, UiThemeStyles& s, bool setSize) {
    apply_btn_base_and_size(btn, &s.btnSecondary, s.tokens, setSize);

    lv_obj_set_style_bg_color  (btn, s.tokens.colorSuccess,   LV_PART_MAIN);
    lv_obj_set_style_text_color(btn, s.tokens.colorOnSuccess, LV_PART_MAIN);

    lv_obj_set_style_bg_color(btn, mix_pressed(s.tokens.colorSuccess, s.tokens),
                              sel(LV_PART_MAIN, LV_STATE_PRESSED));

    lv_obj_set_style_outline_width(btn, s.tokens.focusOutlineW, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_color(btn, s.tokens.focusOutlineColor, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_pad  (btn, s.tokens.focusOutlinePad, sel(LV_PART_MAIN, LV_STATE_FOCUSED));

    lv_obj_set_style_bg_opa(btn, s.tokens.opaDisabled, sel(LV_PART_MAIN, LV_STATE_DISABLED));
}

void applyButtonWarning(lv_obj_t* btn, UiThemeStyles& s, bool setSize) {
    apply_btn_base_and_size(btn, &s.btnSecondary, s.tokens, setSize);

    lv_obj_set_style_bg_color  (btn, s.tokens.colorWarning,   LV_PART_MAIN);
    lv_obj_set_style_text_color(btn, s.tokens.colorOnWarning, LV_PART_MAIN);

    lv_obj_set_style_bg_color(btn, mix_pressed(s.tokens.colorWarning, s.tokens),
                              sel(LV_PART_MAIN, LV_STATE_PRESSED));

    lv_obj_set_style_outline_width(btn, s.tokens.focusOutlineW, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_color(btn, s.tokens.focusOutlineColor, sel(LV_PART_MAIN, LV_STATE_FOCUSED));
    lv_obj_set_style_outline_pad  (btn, s.tokens.focusOutlinePad, sel(LV_PART_MAIN, LV_STATE_FOCUSED));

    lv_obj_set_style_bg_opa(btn, s.tokens.opaDisabled, sel(LV_PART_MAIN, LV_STATE_DISABLED));
}

void applyListStylesToChildren(lv_obj_t* parent, UiThemeStyles& s, bool large, bool withDivider) {
    if (!parent) return;
    applyListContainer(parent, s);

    uint32_t cnt = lv_obj_get_child_cnt(parent);
    for (uint32_t i = 0; i < cnt; ++i) {
        lv_obj_t* child = lv_obj_get_child(parent, i);
        applyListItem(child, s, large, withDivider);
    }
}

} // namespace Ui
