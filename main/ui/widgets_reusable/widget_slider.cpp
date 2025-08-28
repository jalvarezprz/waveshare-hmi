#include "widget_slider.h"
#include <inttypes.h>   // PRId32
#include <utility>

WidgetSlider::WidgetSlider(lv_obj_t* parent,
                           const char* title,
                           const char* unit,
                           int32_t min,
                           int32_t max)
{
    // Contenedor principal
    container_ = lv_obj_create(parent);
    lv_obj_set_size(container_, 260, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(container_, 10, 0);
    lv_obj_set_flex_flow(container_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container_, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // Estilo SOLO del contenedor (no tocar labels aún)
    applyDefaultStyle_();

    // Título (crear antes de aplicar estilos específicos)
    titleLabel_ = lv_label_create(container_);
    lv_label_set_text(titleLabel_, title ? title : "");
    // Si quieres fuente distinta, hazlo aquí (el objeto ya existe)
    // lv_obj_set_style_text_font(titleLabel_, &lv_font_montserrat_16, 0);

    // Fila valor + unidad
    lv_obj_t* row = lv_obj_create(container_);
    lv_obj_remove_style_all(row);
    lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    valueLabel_ = lv_label_create(row);
    lv_label_set_text(valueLabel_, "0");

    unitLabel_ = lv_label_create(row);
    unitText_ = unit ? unit : "";
    lv_label_set_text(unitLabel_, unitText_.c_str());

    // Slider
    slider_ = lv_slider_create(container_);
    lv_obj_set_width(slider_, LV_PCT(100));
    lv_slider_set_range(slider_, min, max);
    lv_slider_set_value(slider_, min, LV_ANIM_OFF);

    // Evento de cambio (user_data = this)
    lv_obj_add_event_cb(slider_, eventHandlerThunk_, LV_EVENT_VALUE_CHANGED, this);

    // Ajuste inicial
    updateValueLabel_(lv_slider_get_value(slider_));
    buildLayout_();
}

WidgetSlider::~WidgetSlider()
{
    if (container_) {
        lv_obj_del(container_); // elimina hijos también
        container_ = nullptr;
        titleLabel_ = nullptr;
        slider_ = nullptr;
        valueLabel_ = nullptr;
        unitLabel_ = nullptr;
    }
}

void WidgetSlider::setValue(int32_t v, lv_anim_enable_t anim)
{
    if (!slider_) return;
    lv_slider_set_value(slider_, v, anim);
    updateValueLabel_(v);
}

int32_t WidgetSlider::getValue() const
{
    return slider_ ? lv_slider_get_value(slider_) : 0;
}

void WidgetSlider::setRange(int32_t min, int32_t max)
{
    if (!slider_) return;
    lv_slider_set_range(slider_, min, max);
    int32_t v = getValue();
    if (v < min) v = min;
    if (v > max) v = max;
    setValue(v, LV_ANIM_OFF);
}

void WidgetSlider::setTitle(const char* title)
{
    if (!titleLabel_) return;
    lv_label_set_text(titleLabel_, title ? title : "");
}

void WidgetSlider::setUnit(const char* unit)
{
    unitText_ = unit ? unit : "";
    if (unitLabel_) lv_label_set_text(unitLabel_, unitText_.c_str());
    updateValueLabel_(getValue());
}

void WidgetSlider::setWidth(lv_coord_t w)
{
    if (!container_) return;
    lv_obj_set_width(container_, w);
    if (slider_) lv_obj_set_width(slider_, LV_PCT(100));
}

void WidgetSlider::setEnabled(bool enabled)
{
    if (!slider_) return;
    if (enabled) {
        lv_obj_clear_state(slider_, LV_STATE_DISABLED);
    } else {
        lv_obj_add_state(slider_, LV_STATE_DISABLED);
    }
}

void WidgetSlider::onValueChanged(std::function<void(int32_t)> cb)
{
    valueChangedCb_ = std::move(cb);
}

void WidgetSlider::applyDefaultStyle_()
{
    // Estilo minimalista del contenedor
    lv_obj_set_style_radius(container_, 12, 0);
    lv_obj_set_style_pad_row(container_, 6, 0);
    lv_obj_set_style_bg_opa(container_, LV_OPA_10, 0);
    lv_obj_set_style_bg_color(container_, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_border_width(container_, 1, 0);
    lv_obj_set_style_border_opa(container_, LV_OPA_20, 0);
}

void WidgetSlider::buildLayout_()
{
    // Punto para afinar tipografías/márgenes si cargas fuentes personalizadas
    // Ej.: if (valueLabel_) lv_obj_set_style_text_font(valueLabel_, &lv_font_montserrat_18, 0);
}

void WidgetSlider::updateValueLabel_(int32_t v)
{
    if (!valueLabel_) return;
    if (unitText_.empty()) {
        lv_label_set_text_fmt(valueLabel_, "%" PRId32, v);
    } else {
        lv_label_set_text_fmt(valueLabel_, "%" PRId32 " %s", v, unitText_.c_str());
    }
}

void WidgetSlider::eventHandlerThunk_(lv_event_t* e)
{
    auto* self = static_cast<WidgetSlider*>(lv_event_get_user_data(e));
    if (self) self->handleEvent_(e);
}

void WidgetSlider::handleEvent_(lv_event_t* e)
{
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED && slider_) {
        int32_t v = lv_slider_get_value(slider_);
        updateValueLabel_(v);
        if (valueChangedCb_) valueChangedCb_(v);
    }
}
