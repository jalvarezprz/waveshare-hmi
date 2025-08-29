#include "ui_menu_mcr50_json.h"

// Nota: UTF-8 sin BOM. Evita comillas sin escapar dentro de las claves/valores.
// Si usas el símbolo °, asegúrate de que el fichero esté en UTF-8.
const char ui_menu_mcr50_json[] = R"json(
{
  "menu": [
    {
      "id": "tend",
      "title": "Punt. Tendencia",
      "items": [
        { "id": "tend_buf",   "title": "Buffer Tendenc" },
        { "id": "tend_flash", "title": "Flash EEPROM"   }
      ]
    },

    { "id": "params", "title": "Parámetros" },

    {
      "id": "info",
      "title": "Inf. Sistema",
      "items": [
        {
          "id": "ain",
          "title": "Ent. Analóg",
          "items": [
            { "id": "T_IDA_SUELO",     "title": "T_IDA_SUELO" },
            { "id": "T_RET_MAQUINA1",  "title": "T_RET_MAQUINA1" },
            { "id": "Z_MCX4_1",        "title": "Z_MCX4_1" },
            { "id": "Z_MCX4_2",        "title": "Z_MCX4_2" },

            {
              "id": "T_DEP_ALTA",
              "title": "T_DEP_ALTA",
              "view": "detail",
              "fields": [
                { "id": "histeresis", "label": "Histeresis Tend.", "unit": "°C",  "type": "number", "mock": "2.5" },
                { "id": "ciclo",      "label": "Ciclo Tend.",      "unit": "min", "type": "number", "mock": "15" },
                { "id": "output",     "label": "Output",                          "type": "enum",   "mock": "ON" },
                { "id": "input",      "label": "Input",                           "type": "enum",   "mock": "OFF" },
                { "id": "dirtec",     "label": "Dir. Tec.",                        "type": "text",   "mock": "010101" },
                { "id": "habtend",    "label": "Hab. Tend.",                       "type": "enum",   "mock": "OFF" },
                { "id": "maxlim",     "label": "Max lim1/lim2",   "unit": "°C",    "type": "text",   "mock": "145/145" },
                { "id": "minlim",     "label": "Min lim1/lim2",   "unit": "°C",    "type": "text",   "mock": "45/45" },
                { "id": "soffset",    "label": "S. Offset",       "unit": "°C",    "type": "number", "mock": "0.0" },
                { "id": "suprimalm",  "label": "Suprim. Alm.",                    "type": "enum",   "mock": "NO" },
                { "id": "valor",      "label": "Valor actual",    "unit": "°C",    "type": "number", "mock": "25.2" }
              ]
            },

            {
              "id": "T_DEP_BAJA",
              "title": "T_DEP_BAJA",
              "view": "detail",
              "fields": [
                { "id": "histeresis", "label": "Histeresis Tend.", "unit": "°C",  "type": "number", "mock": "0.0" },
                { "id": "ciclo",      "label": "Ciclo Tend.",      "unit": "min", "type": "number", "mock": "0" },
                { "id": "output",     "label": "Output",                          "type": "enum",   "mock": "—" },
                { "id": "input",      "label": "Input",                           "type": "enum",   "mock": "—" },
                { "id": "dirtec",     "label": "Dir. Tec.",                        "type": "text",   "mock": "010102" },
                { "id": "habtend",    "label": "Hab. Tend.",                       "type": "enum",   "mock": "OFF" },
                { "id": "maxlim",     "label": "Max lim1/lim2",   "unit": "°C",    "type": "text",   "mock": "145.0/145.0" },
                { "id": "minlim",     "label": "Min lim1/lim2",   "unit": "°C",    "type": "text",   "mock": "45.0/45.0" },
                { "id": "soffset",    "label": "S. Offset",       "unit": "°C",    "type": "number", "mock": "0.0" },
                { "id": "suprimalm",  "label": "Suprim. Alm.",                    "type": "enum",   "mock": "NO" },
                { "id": "valor",      "label": "Valor actual",    "unit": "°C",    "type": "number", "mock": "26.0" },
                { "id": "modo",       "label": "Modo",                            "type": "enum",   "mock": "AUTO" }
              ]
            },

            { "id": "T_IDA_CALD",     "title": "T_IDA_CALD" },
            { "id": "T_IDA_FANCOILS", "title": "T_IDA_FANCOILS" }
          ]
        },

        { "id": "aout",  "title": "Sal. Analóg"  },
        { "id": "din",   "title": "Ent. Digital" },
        { "id": "dout",  "title": "Sal. Digital" },
        { "id": "tot",   "title": "Totalizador"  },
        { "id": "hours", "title": "Horas Functo."}
      ]
    },

    { "id": "hw",  "title": "Conf. Hardware" },
    { "id": "ddc", "title": "Ciclos DDC"     },
    { "id": "bus", "title": "Acceso Buswide" }
  ]
}
)json";
