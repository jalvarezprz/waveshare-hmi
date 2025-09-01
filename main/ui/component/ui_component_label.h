#pragma once
#include "ui_component.h"

namespace Ui {

enum class LabelVariant { Title, Body, Caption };

class UiLabel : public UiComponent {
public:
    UiLabel(const char* text, LabelVariant v = LabelVariant::Body);
    lv_obj_t* create(lv_obj_t* parent) override;

private:
    const char*  text_;
    LabelVariant variant_;
};

} // namespace Ui
