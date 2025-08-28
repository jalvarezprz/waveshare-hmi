#pragma once
#include "lvgl.h"
#include <functional>
#include <string>

class WidgetSlider
{
public:
    // Crea el widget dentro de 'parent' con título, unidad y rango
    WidgetSlider(lv_obj_t* parent,
                 const char* title,
                 const char* unit,
                 int32_t min,
                 int32_t max);

    // Libera recursos gráficos
    ~WidgetSlider();

    // Accesores
    void setValue(int32_t v, lv_anim_enable_t anim = LV_ANIM_OFF);
    int32_t getValue() const;

    void setRange(int32_t min, int32_t max);
    void setTitle(const char* title);
    void setUnit(const char* unit);

    void setWidth(lv_coord_t w);     // ajusta ancho del contenedor
    void setEnabled(bool enabled);   // habilita/deshabilita interacción

    // Callback de cambio de valor (se llama en LV_EVENT_VALUE_CHANGED)
    void onValueChanged(std::function<void(int32_t)> cb);

    // Devuelve el objeto raíz para posicionarlo donde quieras
    lv_obj_t* root() const { return container_; }

private:
    lv_obj_t* container_{nullptr};
    lv_obj_t* titleLabel_{nullptr};
    lv_obj_t* slider_{nullptr};
    lv_obj_t* valueLabel_{nullptr};
    lv_obj_t* unitLabel_{nullptr};

    std::string unitText_;
    std::function<void(int32_t)> valueChangedCb_;

    // Estilo/maquetación
    void applyDefaultStyle_();
    void buildLayout_();
    void updateValueLabel_(int32_t v);

    // Puente estático para eventos de LVGL
    static void eventHandlerThunk_(lv_event_t* e);
    void handleEvent_(lv_event_t* e);
};
