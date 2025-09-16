#pragma once
/**
 * @file ui_preset_label.h
 * @brief Presets de Label para usos habituales (Title, Section, Body, Caption, Value, Status).
 * @defgroup ui_preset_label Presets Label
 * @ingroup ui_component_label
 * @{
 *
 * Objetivo:
 * - Proveer atajos semánticos para crear labels típicos de la HMI sin repetir
 *   props en cada vista.
 * - Consumir el componente Ui::Component::Label y UiThemeStyles (no Tokens directos).
 *
 * Notas:
 * - Los presets devuelven el Handle del componente Label.
 * - Se ofrecen overloads que resuelven Ui::getThemeStyles() para el caso simple.
 */

#include "ui/theme/ui_theme_styles.h"
#include "ui/component/ui_component_label.h"
#include "lvgl.h"

namespace Ui::Preset::Label {

// Alias de tipos del componente
using Handle  = Ui::Component::Label::Handle;
using Variant = Ui::Component::Label::Variant;

/* ============================== Presets básicos ============================== */

/** Título de pantalla (grande). */
Handle Title   (lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text,
                lv_text_align_t align = LV_TEXT_ALIGN_LEFT);

/** Cabecera de sección (intermedia; más pequeña que Title). */
Handle Section (lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text,
                lv_text_align_t align = LV_TEXT_ALIGN_LEFT);

/** Texto normal de contenido. */
Handle Body    (lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text,
                lv_text_align_t align = LV_TEXT_ALIGN_LEFT);

/** Texto secundario (caption, atenuado). */
Handle Caption (lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text,
                lv_text_align_t align = LV_TEXT_ALIGN_LEFT);

/** Valor destacado (números, medidas). */
Handle Value   (lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text,
                lv_text_align_t align = LV_TEXT_ALIGN_LEFT);

/* ============================== Preset de estado ============================= */

/** Tipo de estado para Status(). */
enum class StatusKind : uint8_t { Success, Warning, Error };

/**
 * @brief Texto de estado con color semántico (Success/Warning/Error).
 * @note Base tipográfica: Body; color según StatusKind.
 */
Handle Status(lv_obj_t* parent, Ui::UiThemeStyles& s, const char* text, StatusKind kind,
              lv_text_align_t align = LV_TEXT_ALIGN_LEFT);

/* ============================ Overloads sencillos ============================ */

Handle Title   (lv_obj_t* parent, const char* text, lv_text_align_t align = LV_TEXT_ALIGN_LEFT);
Handle Section (lv_obj_t* parent, const char* text, lv_text_align_t align = LV_TEXT_ALIGN_LEFT);
Handle Body    (lv_obj_t* parent, const char* text, lv_text_align_t align = LV_TEXT_ALIGN_LEFT);
Handle Caption (lv_obj_t* parent, const char* text, lv_text_align_t align = LV_TEXT_ALIGN_LEFT);
Handle Value   (lv_obj_t* parent, const char* text, lv_text_align_t align = LV_TEXT_ALIGN_LEFT);

Handle StatusSuccess(lv_obj_t* parent, const char* text, lv_text_align_t align = LV_TEXT_ALIGN_LEFT);
Handle StatusWarning(lv_obj_t* parent, const char* text, lv_text_align_t align = LV_TEXT_ALIGN_LEFT);
Handle StatusError  (lv_obj_t* parent, const char* text, lv_text_align_t align = LV_TEXT_ALIGN_LEFT);

/** @} */ // end of group
} // namespace Ui::Preset::Label
