#pragma once
#include <string>
#include <vector>
#include <cstddef>

/**
 * ElementSpecification: descripción de un elemento interactivo.
 *  - Sin dependencias de LVGL ni del parser: solo datos.
 */
struct ElementSpecification {
    std::string id;        // slug único dentro de la pantalla
    std::string title;     // texto visible
    std::string icon;      // nombre de icono (opcional)
    std::string action;    // "NAV:/..." | "DO:/..." (o derivado de objeto de acción)
    bool        enabled = true;
    bool        visible = true;
    bool        confirm = false;   // si requiere confirmación previa (para DO:/)
    std::string badgeText;         // badge.text si existe

    // Subniveles opcionales (para mockups). Profundidad máxima recomendada: 1.
    std::vector<ElementSpecification> children;
};

/**
 * ScreenSpecification: descripción de una pantalla.
 */
struct ScreenSpecification {
    int         schemaVersion = 0; // requerido
    std::string id;                // recomendado
    std::string view;              // requerido: "menu_grid" | "menu_list" | "detail"
    std::string title;             // opcional
    std::vector<ElementSpecification> elements; // elementos (ordenados)
};

/**
 * @brief Carga una pantalla desde texto JSON *de esa pantalla*.
 *        Reglas (v1):
 *          - Requeridos: schemaVersion (number), view (string)
 *          - Alias aceptados por transición:
 *              * raíz: specVersion → schemaVersion
 *              * lista: elements | items | tiles
 *              * elemento: elementId→id, elementTitle→title, elementIcon→icon, elementAction→action
 *          - action: string o objeto { "type":"NAV|DO", "to":"/ruta" } → se normaliza a "NAV:/ruta" | "DO:/ruta"
 *          - children: se parsea hasta 1 nivel (si hay)
 * @param text    JSON UTF-8 (null-terminated)
 * @param out     salida
 * @param errBuf  buffer para mensaje de error (puede ser nullptr)
 * @param errLen  tamaño de errBuf
 * @return true si OK; false en error (errBuf con mensaje si se proporciona)
 */
bool loadScreen(const char* text, ScreenSpecification& out, char* errBuf, size_t errLen);
