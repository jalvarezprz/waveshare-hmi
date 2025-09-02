#include "ui/actions/ui_actions.h"
#include "ui_router.h"
#include "ui_menu_nav.h"

// Forward declaration: implementada en ui_view_menu_list.cpp
void ui_show_menu_generic();

namespace Ui {
namespace Actions {

void back_default()
{
    if (!Ui::Menu::ui_menu_nav_path().empty()) {
        Ui::Menu::ui_menu_nav_pop();
    }

    if (Ui::Menu::ui_menu_nav_path().empty()) {
        ui_router_go(UiScreen::MAIN_MENU);
    } else {
        ui_show_menu_generic();  // repinta lista actual
    }
}

} // namespace Actions
} // namespace Ui
