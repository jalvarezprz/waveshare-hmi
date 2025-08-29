#pragma once
#include <functional>

// Muestra el diálogo "Introducir clave ****".
// expected_pin: PIN válido (p.ej. 2410, más adelante NVS).
// on_done(ok): callback cuando el usuario pulsa SEGUIR (true si el PIN coincide).
void ui_show_pin_dialog(int expected_pin, std::function<void(bool ok)> on_done);
