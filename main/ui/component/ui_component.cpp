#include "ui_component.h"
#include <esp_log.h>

namespace {
constexpr const char* TAG = "UI.Component";
}

namespace Ui {

// --- Ciclo de vida ----------------------------------------------------------

Component::~Component() {
    // Propiedad clara: si root_ existe, lo eliminamos.
    if (root_) {
        // En LVGL v8, lv_obj_del(nullptr) no se llama; comprobación simple.
        lv_obj_del(root_);
        root_ = nullptr;
    }
}

Component::Component(Component&& other) noexcept {
    root_ = other.root_;
    other.root_ = nullptr;
}

Component& Component::operator=(Component&& other) noexcept {
    if (this != &other) {
        if (root_) {
            lv_obj_del(root_);
        }
        root_ = other.root_;
        other.root_ = nullptr;
    }
    return *this;
}

// --- Hooks por defecto ------------------------------------------------------

void Component::applyTheme() {
    // Por defecto no hace nada.
    // Derivados aplicarán tokens/styles del tema aquí.
}

void Component::onEvent(lv_event_t* /*e*/) {
    // Por defecto, sin manejo. Derivados lo implementan si lo necesitan.
}

// --- Registro de eventos ----------------------------------------------------

void Component::registerAllEvents() {
    if (!root_) {
        ESP_LOGE(TAG, "registerAllEvents() llamado sin root_");
        return;
    }
    lv_obj_add_event_cb(root_, &Component::eventThunk, LV_EVENT_ALL, this);
}

void Component::eventThunk(lv_event_t* e) {
    // user_data contiene el puntero 'this' que pasamos en add_event_cb(...)
    auto* self = static_cast<Component*>(lv_event_get_user_data(e));
    if (!self) return;
    self->onEvent(e);
}

} // namespace Ui
