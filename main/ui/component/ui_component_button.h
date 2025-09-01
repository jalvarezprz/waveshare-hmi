#pragma once
#include "ui_component.h"

namespace Ui {

enum class ButtonRole { Primary, Secondary, Ghost };

class UiButton : public UiComponent {
public:
    using Callback = void(*)(lv_event_t* e);

    UiButton(const char* text, ButtonRole role = ButtonRole::Primary, Callback onClick = nullptr);
    lv_obj_t* create(lv_obj_t* parent) override;

private:
    const char* text_;
    ButtonRole  role_;
    Callback    onClick_;
};

} // namespace Ui
