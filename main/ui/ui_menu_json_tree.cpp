#include "ui_menu_json_tree.h"

// Definición del JSON embebido como array de chars
const char ui_menu_json_tree[] = R"json(
{
  "menu": [
    {
      "id": "tend",
      "title": "Punt. Tendencia",
      "items": [
        { 
          "id": "tend_buf",   
          "title": "Buffer Tendenc",
          "widget_type": "button"
        },
        { 
          "id": "tend_flash", 
          "title": "Flash EEPROM",
          "widget_type": "button"
        }
      ]
    },

    { 
      "id": "params", 
      "title": "Parámetros",
      "widget_type": "button"
    },

    {
      "id": "info",
      "title": "Inf. Sistema",
      "items": [

        {
          "id": "T_DEP_ALTA",
          "title": "T_DEP_ALTA",
          "widget_type": "slider",
          "view": "detail",
          "fields": [
            { 
              "id": "histeresis", 
              "label": "Histeresis",
              "unit": "°C", 
              "type": "number", 
              "mock": "2.5", 
              "editable": true, 
              "min": 0, 
              "max": 10, 
              "step": 0.1,
              "widget_type": "slider"
            },
            { 
              "id": "modo",  
              "label": "Modo", 
              "type": "enum", 
              "mock": "AUTO",
              "editable": true, 
              "options": ["AUTO","MANUAL","FORZADO"],
              "widget_type": "dropdown"
            },
            { 
              "id": "ciclo", 
              "label": "Ciclo", 
              "unit": "min", 
              "type": "number", 
              "mock": "15", 
              "editable": true, 
              "min": 0, 
              "max": 60, 
              "step": 1,
              "widget_type": "number"
            },
            { 
              "id": "comentario", 
              "label": "Nota técnica",
              "type": "text", 
              "mock": "Rev.2025",
              "widget_type": "text"
            }
          ]
        },

        {
          "id": "T_DEP_BAJA",
          "title": "T_DEP_BAJA",
          "widget_type": "slider",
          "view": "detail",
          "fields": [
            { 
              "id": "soffset", 
              "label": "Offset", 
              "unit": "°C", 
              "type": "number", 
              "mock": "0.0",
              "editable": true, 
              "min": -10, 
              "max": 10, 
              "step": 0.1,
              "widget_type": "slider"
            },
            { 
              "id": "habtend",
              "label": "Hab. Tend.", 
              "type": "enum", 
              "mock": "OFF",
              "editable": true, 
              "options": ["OFF","ON"],
              "widget_type": "dropdown"
            },
            { 
              "id": "valor", 
              "label": "Valor actual", 
              "unit": "°C", 
              "type": "number", 
              "mock": "26.0",
              "widget_type": "number"
            },
            { 
              "id": "dirtec", 
              "label": "Dir. Tec.", 
              "type": "text", 
              "mock": "010102",
              "widget_type": "text"
            }
          ]
        },

        { 
          "id": "T_IDA_CALD",     
          "title": "T_IDA_CALD", 
          "widget_type": "number",
          "mock": "40.2"
        },
        { 
          "id": "T_IDA_FANCOILS", 
          "title": "T_IDA_FANCOILS", 
          "widget_type": "number",
          "mock": "36.5"
        }
      ]
    },

    { 
      "id": "hw",  
      "title": "Conf. Hardware",
      "widget_type": "button"
    },
    { 
      "id": "ddc", 
      "title": "Ciclos DDC", 
      "widget_type": "button"
    },
    { 
      "id": "bus", 
      "title": "Acceso Buswide", 
      "widget_type": "button"
    }
  ]
}
)json";
