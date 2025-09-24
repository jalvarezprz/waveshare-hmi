#pragma once
/**
 * @file component.h
 * @brief Base UI::Component: envoltorio (wrapper) C++ de un lv_obj_t* con
 *        ciclo de vida controlado, registro único de eventos y punto de
 *        extensión para aplicar tema/estilos.
 *
 * Reglas:
 *  - No heredar de lv_obj_t. Composición: esta clase posee un lv_obj_t*.
 *  - Cada componente crea su root en create(parent) y registra eventos.
 *  - El destructor elimina root_ (ownership claro).
 */

#include <lvgl.h>

namespace Ui {

/**
 * @brief Clase base para todos los componentes UI.
 *
 * Patrón:
 *  - Derivados implementan create(parent) y construyen su objeto LVGL.
 *  - onEvent(e) se sobreescribe para manejar eventos.
 *  - applyTheme() se sobreescribe para aplicar tokens/styles del tema.
 */
class Component {
public:
    Component() = default;
    virtual ~Component();

    // No copiable (un objeto LVGL no debe tener dos dueños).
    Component(const Component&) = delete;
    Component& operator=(const Component&) = delete;

    // Movible (transfiere propiedad del root_).
    Component(Component&& other) noexcept;
    Component& operator=(Component&& other) noexcept;

    /**
     * @brief Crea el objeto LVGL de este componente bajo el parent dado.
     * Debe asignar root_ y registrar el callback de eventos.
     */
    virtual void create(lv_obj_t* parent) = 0;

    /** @brief Devuelve el objeto raíz LVGL del componente. */
    inline lv_obj_t* root() const { return root_; }

protected:
    /**
     * @brief Punto de extensión para aplicar tokens/styles del tema.
     * Llamar al final de create(...) tras construir root_.
     */
    virtual void applyTheme();

    /**
     * @brief Manejador virtual de eventos. Derivados lo sobreescriben.
     * @note Se invoca a través del thunk estático eventThunk(...).
     */
    virtual void onEvent(lv_event_t* e);

    /**
     * @brief Registra el callback de eventos para LV_EVENT_ALL en root_.
     * Debe llamarse una vez en create(...) tras crear root_.
     */
    void registerAllEvents();

    /** Objeto raíz LVGL que posee este componente. */
    lv_obj_t* root_ = nullptr;

private:
    /** Adaptador estático C → C++ para reenviar eventos a this->onEvent(...) */
    static void eventThunk(lv_event_t* e);
};

} // namespace Ui
