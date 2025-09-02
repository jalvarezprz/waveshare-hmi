#pragma once

#include "lvgl.h"

/**
 * @brief Botón genérico reutilizable (LVGL) con creación en una línea.
 *        Versión mínima para comenzar las pruebas (texto + icono LVGL opcional).
 *
 * Convenciones:
 *  - Clase: PascalCase
 *  - Métodos públicos: camelCase
 *  - Privadas: camelCase_ (con guion bajo final)
 */

class Button
{
public:
    /**
     * @brief Crea un botón en una única llamada.
     * @param parent    Objeto padre.
     * @param text      Texto a mostrar (puede ser nullptr).
     * @param symbol    Símbolo LVGL (ej.: LV_SYMBOL_LEFT) o nullptr para sin icono.
     * @param onClick   Callback C (puede ser nullptr).
     * @param width     Ancho (por defecto 120).
     * @param height    Alto  (por defecto 44).
     * @return          Instancia Button con punteros a lvgl internos.
     */
    static Button create(lv_obj_t* parent,
                         const char* text,
                         const char* symbol,
                         void (*onClick)(void),
                         lv_coord_t width  = 120,
                         lv_coord_t height = 44);

    /// Objeto raíz (lv_btn)
    lv_obj_t* root() const { return btn_; }

    /// Cambiar el texto tras crear el botón
    void setText(const char* txt);

    /// Habilitar / deshabilitar
    void setEnabled(bool enabled);

    /// Acceso a objetos internos por si necesitas ajustes finos
    lv_obj_t* iconLabel() const { return iconLabel_; }   // puede ser nullptr si no hay icono
    lv_obj_t* textLabel() const { return textLabel_; }   // puede ser nullptr si no hay texto

private:
    // Constructor privado: usa Button::create(...)
    Button(lv_obj_t* btn, lv_obj_t* iconLabel, lv_obj_t* textLabel)
        : btn_(btn), iconLabel_(iconLabel), textLabel_(textLabel) {}

    // Estilos compartidos (se inicializan una sola vez)
    static void ensureStyles();
    static bool stylesInited_;

    static lv_style_t styleBtnBase_;
    static lv_style_t styleBtnPressed_;
    static lv_style_t styleBtnFocused_;

    static lv_style_t styleIconLabel_;
    static lv_style_t styleTextLabel_;

private:
    lv_obj_t* btn_        = nullptr;
    lv_obj_t* iconLabel_  = nullptr;
    lv_obj_t* textLabel_  = nullptr;
};
