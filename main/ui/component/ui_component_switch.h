#pragma once
/**
 * @file ui_switch.h
 * @brief Componente Ui::Switch. Envuelve lv_switch como clase C++.
 */

#include "ui_component.h"

namespace Ui {

/**
 * @brief Interruptor on/off basado en lv_switch.
 *
 * Características:
 *  - API simple: setState/getState.
 *  - Callback onToggle para reaccionar al cambio.
 *  - Usa tokens/styles en applyTheme().
 */
class Switch : public Component {
public:
    Switch() = default;
    ~Switch() override = default;

    /** Crea el switch como hijo de parent. */
    void create(lv_obj_t* parent) override;

    /** Fija el estado lógico del switch. */
    void setState(bool on);

    /** Devuelve el estado lógico actual. */
    bool getState() const;

    /** Registra callback para cambios de estado. */
    void setOnToggle(void (*cb)(bool, void*), void* user_data = nullptr);

protected:
    void applyTheme() override;
    void onEvent(lv_event_t* e) override;

private:
    void (*onToggle_)(bool, void*) = nullptr;
    void* userData_ = nullptr;
};

} // namespace Ui
