#include "ui/menu/ui_menu_loader.h"
#include "ui/menu/ui_menu_tree.h"
#include "esp_log.h"
#include <cstring>

static const char* TAG = "UI_MENU_SMOKE";

static void assert_true(bool cond, const char* msg) {
    if (!cond) ESP_LOGE(TAG, "ASSERT FAIL: %s", msg);
    else       ESP_LOGI(TAG, "OK: %s", msg);
}

static void dump_screen(const ScreenSpecification& s) {
    ESP_LOGI(TAG, "Screen: id='%s' view='%s' title='%s' schemaVersion=%d elements=%u",
             s.id.c_str(), s.view.c_str(), s.title.c_str(), s.schemaVersion,
             (unsigned)s.elements.size());
    for (size_t i = 0; i < s.elements.size(); ++i) {
        const auto& e = s.elements[i];
        ESP_LOGI(TAG, "  [%u] id='%s' title='%s' icon='%s' action='%s' enabled=%d visible=%d confirm=%d children=%u",
                 (unsigned)i, e.id.c_str(), e.title.c_str(), e.icon.c_str(),
                 e.action.c_str(), (int)e.enabled, (int)e.visible, (int)e.confirm,
                 (unsigned)e.children.size());
    }
}

/**
 * @brief Test mínimo de carga:
 *  1) Carga la pantalla MAIN desde ui_menu_tree.
 *  2) Valida campos básicos.
 *  3) Hace un par de pruebas negativas rápidas.
 */
extern "C" void ui_menu_loader_run_smoke_test(void) {
    ESP_LOGI(TAG, "=== ui_menu_loader SMOKE TEST ===");

    // 1) Obtener el JSON SOLO de la pantalla pedida ("main")
    const char* json = ui_menu_tree_get(UI_SCREEN_ID_MAIN);
    assert_true(json != nullptr, "ui_menu_tree_get('main') devuelve JSON");

    // 2) Parsear con el loader
    ScreenSpecification scr; char err[128];
    bool ok = loadScreen(json, scr, err, sizeof(err));
    assert_true(ok, "loadScreen(main) retorna true");
    if (!ok) {
        ESP_LOGE(TAG, "Error: %s", err);
        return;
    }

    // 3) Validaciones básicas
    assert_true(scr.schemaVersion >= 1, "schemaVersion válido");
    assert_true(!scr.view.empty(),      "view no vacío");
    assert_true(scr.view == "menu_grid","view == 'menu_grid'");
    assert_true(scr.elements.size() >= 1, "hay elementos");
    dump_screen(scr);

    // 4) Caso negativo: falta schemaVersion
    {
        const char* bad = R"json({"id":"bad","view":"menu_grid","title":"Bad"})json";
        ScreenSpecification tmp;
        ok = loadScreen(bad, tmp, err, sizeof(err));
        assert_true(!ok, "falta schemaVersion → falla");
        if (!ok) ESP_LOGI(TAG, "Mensaje esperado: %s", err);
    }

    // 5) Compatibilidad: alias antiguos (specVersion + items + elementTitle/elementAction)
    {
        const char* legacy = R"json(
        {
          "specVersion": 1,
          "id": "legacy",
          "view": "menu_list",
          "items": [
            { "elementId": "a", "elementTitle": "A", "elementIcon": "info", "elementAction": "NAV:/screen/info" }
          ]
        })json";
        ScreenSpecification tmp;
        ok = loadScreen(legacy, tmp, err, sizeof(err));
        assert_true(ok, "alias legacy (specVersion+items) soportados");
        if (ok) dump_screen(tmp);
        else ESP_LOGE(TAG, "Error legacy: %s", err);
    }

    ESP_LOGI(TAG, "=== FIN SMOKE TEST ===");
}
