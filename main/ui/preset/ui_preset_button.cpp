/**
 * @file ui_preset_button.cpp
 * @brief Implementación de presets de botones (Menu / Back) en un único módulo.
 * @ingroup ui_preset
 */

#include "ui/preset/ui_preset_button.h"
#include "ui/theme/ui_theme_styles.h"
#include "lvgl.h"
#include <cstring>
#include <cstdio>

namespace Ui::Preset {

/* ========================= Helpers comunes (no públicos) ================== */
namespace {

/** Normaliza "lv:xxx" a LV_SYMBOL_XXX; retorna literal si no es "lv:". */
const char* normalize_icon(const char* iconId) {
    if (!iconId || !*iconId) return LV_SYMBOL_SETTINGS;
    if (std::strncmp(iconId, "lv:", 3) != 0) return iconId;
    const char* n = iconId + 3;
    if      (std::strcmp(n, "home")     == 0) return LV_SYMBOL_HOME;
    else if (std::strcmp(n, "settings") == 0) return LV_SYMBOL_SETTINGS;
    else if (std::strcmp(n, "power")    == 0) return LV_SYMBOL_POWER;
    else if (std::strcmp(n, "warning")  == 0) return LV_SYMBOL_WARNING;
    else if (std::strcmp(n, "ok")       == 0) return LV_SYMBOL_OK;
    else if (std::strcmp(n, "play")     == 0) return LV_SYMBOL_PLAY;
    else if (std::strcmp(n, "stop")     == 0) return LV_SYMBOL_STOP;
    else if (std::strcmp(n, "pause")    == 0) return LV_SYMBOL_PAUSE;
    else if (std::strcmp(n, "up")       == 0) return LV_SYMBOL_UP;
    else if (std::strcmp(n, "down")     == 0) return LV_SYMBOL_DOWN;
    else if (std::strcmp(n, "left")     == 0) return LV_SYMBOL_LEFT;
    else if (std::strcmp(n, "right")    == 0) return LV_SYMBOL_RIGHT;
    else if (std::strcmp(n, "refresh")  == 0) return LV_SYMBOL_REFRESH;
    else if (std::strcmp(n, "edit")     == 0) return LV_SYMBOL_EDIT;
    else if (std::strcmp(n, "trash")    == 0) return LV_SYMBOL_TRASH;
    return LV_SYMBOL_SETTINGS;
}

/** Layout vertical (icono sobre texto) con medidas desde tokens. */
void apply_layout_column(Ui::Component::Button::Handle& base, UiThemeStyles& s) {
    if (!base.root) return;
    lv_obj_set_flex_flow (base.root, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(base.root, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(base.root, s.tokens.btnIconGap, LV_PART_MAIN);
    if (s.tokens.minTouch > 0) lv_obj_set_style_min_height(base.root, s.tokens.minTouch, LV_PART_MAIN);
    if (base.label) {
        lv_obj_set_style_text_align(base.label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_label_set_long_mode(base.label, LV_LABEL_LONG_DOT);
        lv_obj_set_width(base.label, LV_PCT(100));
    }
    if (base.icon) lv_obj_set_style_text_align(base.icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
}

/** Layout horizontal (icono a la izquierda) con medidas desde tokens. */
void apply_layout_row(Ui::Component::Button::Handle& base, UiThemeStyles& s) {
    if (!base.root) return;
    lv_obj_set_flex_flow (base.root, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(base.root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(base.root, s.tokens.btnIconGap, LV_PART_MAIN);
    if (s.tokens.minTouch > 0) lv_obj_set_style_min_height(base.root, s.tokens.minTouch, LV_PART_MAIN);
    if (base.label) lv_obj_set_style_text_align(base.label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
}

/** Crea/actualiza el hint bajo la etiqueta principal. */
void set_hint(Ui::Component::Button::Handle& base, UiThemeStyles& s, lv_obj_t*& hintLbl, const char* text) {
    if (!text || !*text) {
        if (hintLbl) { lv_obj_del(hintLbl); hintLbl = nullptr; }
        return;
    }
    if (!hintLbl) {
        hintLbl = lv_label_create(base.root);
        lv_obj_set_style_text_align(hintLbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
        lv_obj_set_style_text_color(hintLbl, s.tokens.colorMuted,  LV_PART_MAIN);
        lv_obj_set_style_text_font (hintLbl, s.tokens.fontCaption, LV_PART_MAIN);
    }
    lv_label_set_text(hintLbl, text);
    if (base.label) lv_obj_move_to_index(hintLbl, lv_obj_get_index(base.label) + 1);
}

/** Crea/actualiza el badge (contador o punto). */
void set_badge(Ui::Component::Button::Handle& base, UiThemeStyles& s, lv_obj_t*& badge,
               int count, bool showDot)
{
    if (count < 0 && !showDot) {
        if (badge) { lv_obj_del(badge); badge = nullptr; }
        return;
    }
    if (!badge) {
        badge = lv_label_create(base.root);
        lv_obj_set_style_bg_color  (badge, s.tokens.colorPrimary,   LV_PART_MAIN);
        lv_obj_set_style_bg_opa    (badge, s.tokens.opaEnabled,     LV_PART_MAIN);
        lv_obj_set_style_text_color(badge, s.tokens.colorOnPrimary, LV_PART_MAIN);
        lv_obj_set_style_pad_hor   (badge, s.tokens.badgePadH,      LV_PART_MAIN);
        lv_obj_set_style_pad_ver   (badge, s.tokens.badgePadV,      LV_PART_MAIN);
        lv_obj_set_style_radius    (badge, s.tokens.badgeRadius,    LV_PART_MAIN);
    }
    if (count >= 0) {
        if (count > 99) lv_label_set_text(badge, "99+");
        else { char b[8]; std::snprintf(b, sizeof(b), "%d", count); lv_label_set_text(badge, b); }
    } else {
        lv_label_set_text(badge, "•");
    }
    lv_obj_align(badge, LV_ALIGN_TOP_RIGHT, s.tokens.badgeOffsetX, s.tokens.badgeOffsetY);
}

} // namespace (helpers)

/* ============================== ButtonMenu =============================== */
namespace ButtonMenu {

Handle create(lv_obj_t* parent, UiThemeStyles& s, const Props& p) {
    Ui::themeInitOnce();

    Handle h{};
    Ui::Component::Button::Props bp{};
    bp.text     = p.text;
    bp.icon     = normalize_icon(p.iconId);
    bp.variant  = p.variant;
    bp.iconPos  = Ui::Component::Button::IconPos::Left;
    bp.enabled  = !p.disabled;
    bp.toggle   = p.selected;
    bp.checked  = p.selected;
    bp.loading  = p.loading;

    // Routing/contexto
    bp.userData = p.userData;
    bp.action   = p.action;

    // Crear base + callbacks
    h.base = Ui::Component::Button::create(parent, s, bp, p.callbacks);

    // Estilo/maquetación y adornos
    apply_layout_column(h.base, s);
    set_hint (h.base, s, h.hintLbl, p.hint);
    set_badge(h.base, s, h.badge,   p.badgeCount, p.showDot);

    Ui::Component::Button::setEnabled(h.base, s, !p.disabled);
    Ui::Component::Button::setChecked (h.base, s,  p.selected);
    return h;
}

bool setText(Handle& h, UiThemeStyles& s, const char* text) {
    Ui::Component::Button::setText(h.base, s, text);
    if (h.hintLbl && h.base.label) {
        lv_obj_move_to_index(h.hintLbl, lv_obj_get_index(h.base.label) + 1);
    }
    return true;
}

bool setIcon(Handle& h, UiThemeStyles& s, const char* iconId) {
    Ui::Component::Button::setIcon(h.base, s, normalize_icon(iconId),
                                   Ui::Component::Button::IconPos::Left);
    apply_layout_column(h.base, s);
    return true;
}

bool setHint(Handle& h, UiThemeStyles& s, const char* hint) {
    set_hint(h.base, s, h.hintLbl, hint);
    return true;
}

bool setBadge(Handle& h, UiThemeStyles& s, int count, bool showDot) {
    set_badge(h.base, s, h.badge, count, showDot);
    return true;
}

bool setSelected(Handle& h, UiThemeStyles& s, bool v) {
    Ui::Component::Button::setChecked(h.base, s, v);
    return true;
}

bool setDisabled(Handle& h, UiThemeStyles& s, bool v) {
    Ui::Component::Button::setEnabled(h.base, s, !v);
    return true;
}

bool setVariant(Handle& h, UiThemeStyles& s, Variant v) {
    if (!h.base.root) return false;
    // ⬇️ Sustitución: aplicar variante con helpers de estilo del tema
    switch (v) {
        case Variant::Primary:     Ui::applyButtonPrimary    (h.base.root, s, false); break;
        case Variant::Secondary:   Ui::applyButtonSecondary  (h.base.root, s, false); break;
        case Variant::Ghost:       Ui::applyButtonGhost      (h.base.root, s, false); break;
        case Variant::Destructive: Ui::applyButtonDestructive(h.base.root, s, false); break;
        case Variant::Success:     Ui::applyButtonSuccess    (h.base.root, s, false); break;
        case Variant::Warning:     Ui::applyButtonWarning    (h.base.root, s, false); break;
    }
    apply_layout_column(h.base, s);
    return true;
}

void focus(Handle& h) { Ui::Component::Button::focus(h.base); }

} // namespace ButtonMenu

/* =============================== ButtonBack ============================== */
namespace ButtonBack {

Handle create(lv_obj_t* parent, UiThemeStyles& s, const Props& p) {
    Ui::themeInitOnce();

    Handle h{};
    Ui::Component::Button::Props bp{};
    bp.text     = p.text;
    bp.icon     = normalize_icon(p.iconId);
    bp.variant  = p.variant;
    bp.iconPos  = Ui::Component::Button::IconPos::Left;
    bp.enabled  = !p.disabled;
    bp.toggle   = false;
    bp.checked  = false;
    bp.loading  = p.loading;

    // Routing/contexto
    bp.userData = p.userData;
    bp.action   = p.action ? p.action : "NAV:/back";

    // Crear base + callbacks
    h.base = Ui::Component::Button::create(parent, s, bp, p.callbacks);

    // Estilo/maquetación recomendada para "Back"
    switch (p.variant) {
        case Variant::Secondary:   Ui::applyButtonSecondary  (h.base.root, s, false); break;
        case Variant::Ghost:       Ui::applyButtonGhost      (h.base.root, s, false); break;
        default: /* se respeta la variante solicitada */                           break;
    }
    apply_layout_row(h.base, s);
    Ui::Component::Button::setEnabled(h.base, s, !p.disabled);
    return h;
}

bool setText(Handle& h, UiThemeStyles& s, const char* text) {
    Ui::Component::Button::setText(h.base, s, text);
    apply_layout_row(h.base, s);
    return true;
}

bool setIcon(Handle& h, UiThemeStyles& s, const char* iconId) {
    Ui::Component::Button::setIcon(h.base, s, normalize_icon(iconId),
                                   Ui::Component::Button::IconPos::Left);
    apply_layout_row(h.base, s);
    return true;
}

bool setDisabled(Handle& h, UiThemeStyles& s, bool v) {
    Ui::Component::Button::setEnabled(h.base, s, !v);
    return true;
}

bool setVariant(Handle& h, UiThemeStyles& s, Variant v) {
    if (!h.base.root) return false;
    // ⬇️ Sustitución: aplicar variante con helpers de estilo del tema
    switch (v) {
        case Variant::Primary:     Ui::applyButtonPrimary    (h.base.root, s, false); break;
        case Variant::Secondary:   Ui::applyButtonSecondary  (h.base.root, s, false); break;
        case Variant::Ghost:       Ui::applyButtonGhost      (h.base.root, s, false); break;
        case Variant::Destructive: Ui::applyButtonDestructive(h.base.root, s, false); break;
        case Variant::Success:     Ui::applyButtonSuccess    (h.base.root, s, false); break;
        case Variant::Warning:     Ui::applyButtonWarning    (h.base.root, s, false); break;
    }
    apply_layout_row(h.base, s);
    return true;
}

void focus(Handle& h) { Ui::Component::Button::focus(h.base); }

} // namespace ButtonBack

} // namespace Ui::Preset
