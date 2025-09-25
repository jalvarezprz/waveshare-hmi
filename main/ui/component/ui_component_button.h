#pragma once
/**
 * @file ui_component_button.h
 * @brief Componente Ui::Button. Envuelve lv_btn + label interno.
 */
#include "ui_component.h"

namespace Ui {

class Button : public Component {
public:
    Button() = default;
    ~Button() override = default;

    void create(lv_obj_t* parent) override;

    void setText(const char* txt);
    void setEnabled(bool en);
    bool isEnabled() const;

    // onClick simple (click corto)
    void setOnClick(void (*cb)(void*), void* user_data = nullptr);

protected:
    void applyTheme() override;
    void onEvent(lv_event_t* e) override;

private:
    lv_obj_t* label_ = nullptr;
    void (*onClick_)(void*) = nullptr;
    void* userData_ = nullptr;
};

} // namespace Ui
