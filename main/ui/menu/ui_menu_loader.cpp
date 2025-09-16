#include "ui_menu_loader.h"
#include "esp_log.h"
#include <cstring>

extern "C" {
#include "cJSON.h"
}

// ====== Parámetros de validación ======
#ifndef UI_SCHEMA_VERSION_SUPPORTED
#define UI_SCHEMA_VERSION_SUPPORTED 1
#endif

#ifndef UI_SCREEN_MAX_ELEMENTS
#define UI_SCREEN_MAX_ELEMENTS 24   // guía para 800x480; ajusta si lo necesitas
#endif

#ifndef UI_MAX_CHILD_DEPTH
#define UI_MAX_CHILD_DEPTH 1        // profundidad máxima de children
#endif

static const char* TAG = "SCREEN_LOADER";

static void set_err(char* b, size_t n, const char* m) {
    if (b && n) {
        std::snprintf(b, n, "%s", m ? m : "");
        b[n ? n - 1 : 0] = '\0';
    }
}

static cJSON* get2(cJSON* obj, const char* a, const char* b) {
    if (!obj) return nullptr;
    cJSON* x = cJSON_GetObjectItemCaseSensitive(obj, a);
    return x ? x : cJSON_GetObjectItemCaseSensitive(obj, b);
}

static bool parse_action_node(cJSON* node, std::string& out) {
    // action como string
    if (cJSON_IsString(node)) {
        out = node->valuestring;
        return true;
    }
    // action como objeto { "type": "NAV"|"DO", "to": "/screen/..." }
    if (cJSON_IsObject(node)) {
        cJSON* type = get2(node, "type", "kind");
        cJSON* to   = get2(node, "to", "path");
        if (cJSON_IsString(type) && cJSON_IsString(to)) {
            const char* t = type->valuestring;
            const char* p = to->valuestring;
            if (std::strcmp(t, "NAV") == 0) { out = std::string("NAV:") + p; return true; }
            if (std::strcmp(t, "DO")  == 0) { out = std::string("DO:")  + p; return true; }
        }
    }
    // sin action o desconocida → out queda vacío (válido pero sin efecto)
    out.clear();
    return true;
}

static void parse_elements_array(cJSON* arr,
                                 std::vector<ElementSpecification>& out_vec,
                                 int depth, int max_depth) {
    if (!cJSON_IsArray(arr)) return;
    int n = cJSON_GetArraySize(arr);
    if (n < 0) n = 0;
    // Límite de seguridad (no hard-fail para no romper mockups: se recorta)
    if (n > UI_SCREEN_MAX_ELEMENTS) {
        ESP_LOGW(TAG, "elements=%d > max=%d, recortando", n, UI_SCREEN_MAX_ELEMENTS);
        n = UI_SCREEN_MAX_ELEMENTS;
    }
    out_vec.reserve(static_cast<size_t>(n));

    for (int i = 0; i < n; ++i) {
        cJSON* it = cJSON_GetArrayItem(arr, i);
        if (!cJSON_IsObject(it)) continue;

        ElementSpecification e;

        // Campos con alias
        cJSON* eid = get2(it, "id", "elementId");
        cJSON* ett = get2(it, "title", "elementTitle");
        cJSON* eic = get2(it, "icon", "elementIcon");
        cJSON* eac = get2(it, "action", "elementAction");
        cJSON* een = get2(it, "enabled", "isEnabled");
        cJSON* evi = get2(it, "visible", "isVisible");
        cJSON* eco = get2(it, "confirm", "needsConfirm");
        cJSON* ebd = cJSON_GetObjectItemCaseSensitive(it, "badge");
        cJSON* ech = get2(it, "children", "items"); // permitimos "items" como alias en children

        if (cJSON_IsString(eid)) e.id     = eid->valuestring;
        if (cJSON_IsString(ett)) e.title  = ett->valuestring;
        if (cJSON_IsString(eic)) e.icon   = eic->valuestring;
        if (cJSON_IsBool(een))   e.enabled= cJSON_IsTrue(een);
        if (cJSON_IsBool(evi))   e.visible= cJSON_IsTrue(evi);
        if (cJSON_IsBool(eco))   e.confirm= cJSON_IsTrue(eco);

        // badge: { text: "…" }
        if (cJSON_IsObject(ebd)) {
            cJSON* txt = get2(ebd, "text", "label");
            if (cJSON_IsString(txt)) e.badgeText = txt->valuestring;
        }

        // action (string u objeto)
        if (eac) (void)parse_action_node(eac, e.action);

        // children (solo si depth < max_depth)
        if (ech && depth < max_depth) {
            parse_elements_array(ech, e.children, depth + 1, max_depth);
        }

        out_vec.push_back(std::move(e));
    }
}

bool loadScreen(const char* text, ScreenSpecification& out, char* err, size_t errLen) {
    out = {};
    if (!text) { set_err(err, errLen, "text=null"); return false; }

    cJSON* root = cJSON_Parse(text);
    if (!root) { set_err(err, errLen, "JSON inválido"); return false; }

    // schemaVersion (requerido) con alias specVersion
    cJSON* v = get2(root, "schemaVersion", "specVersion");
    if (!cJSON_IsNumber(v)) {
        cJSON_Delete(root);
        set_err(err, errLen, "Falta 'schemaVersion' (number)");
        return false;
    }
    out.schemaVersion = v->valueint;
    if (out.schemaVersion <= 0) {
        cJSON_Delete(root);
        set_err(err, errLen, "schemaVersion inválido");
        return false;
    }
    if (out.schemaVersion > UI_SCHEMA_VERSION_SUPPORTED) {
        cJSON_Delete(root);
        set_err(err, errLen, "schemaVersion no soportado");
        return false;
    }

    // id (opcional)
    cJSON* id = cJSON_GetObjectItemCaseSensitive(root, "id");
    if (cJSON_IsString(id)) out.id = id->valuestring;

    // view (requerido)
    cJSON* vw = cJSON_GetObjectItemCaseSensitive(root, "view");
    if (!cJSON_IsString(vw)) {
        cJSON_Delete(root);
        set_err(err, errLen, "Falta 'view' (string)");
        return false;
    }
    out.view = vw->valuestring;

    // title (opcional)
    cJSON* tt = cJSON_GetObjectItemCaseSensitive(root, "title");
    if (cJSON_IsString(tt)) out.title = tt->valuestring;

    // elements (acepta alias items/tiles)
    cJSON* arr = get2(root, "elements", "items");
    if (!cJSON_IsArray(arr)) arr = cJSON_GetObjectItemCaseSensitive(root, "tiles");

    if (cJSON_IsArray(arr)) {
        parse_elements_array(arr, out.elements, /*depth=*/0, UI_MAX_CHILD_DEPTH);
    }

    cJSON_Delete(root);
    set_err(err, errLen, "");
    return true;
}
