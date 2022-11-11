/**
 * dravex - Copyright (c) 2022 atom0s [atom0s@live.com]
 *
 * Contact: https://www.atom0s.com/
 * Contact: https://discord.gg/UmXNvjq
 * Contact: https://github.com/atom0s
 * Support: https://paypal.me/atom0s
 * Support: https://patreon.com/atom0s
 * Support: https://github.com/sponsors/atom0s
 *
 * This file is part of dravex.
 *
 * dravex is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dravex is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with dravex.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "defines.hpp"
#include "imgui_dravex.hpp"
#include "window.hpp"

/**
 * Callback invoked when the scene is being rendered.
 */
void __stdcall on_render(void)
{
    dravex::imguimgr::instance().render();
}

/**
 * Callback invoked when the graphics device is being reset.
 *
 * @param {bool} is_pre_reset - Flag set if the call is happening before the device has been reset.
 * @return {bool} True on success, false otherwise.
 */
bool __stdcall on_reset(const bool is_pre_reset)
{
    if (is_pre_reset)
        dravex::imguimgr::instance().prereset();
    else
        dravex::imguimgr::instance().postreset();

    return true;
}

/**
 * Callback invoked when the scene is being updated.
 */
void __stdcall on_update(void)
{
    dravex::imguimgr::instance().beginscene();

    ImGui::ShowDemoWindow();

    dravex::imguimgr::instance().endscene();
}

/**
 * Application entry point.
 *
 * @param {int32_t} argc - The argument count passed to the application.
 * @param {char*[]} argv - The argument array passed to the application.
 * @return {int32_t} 0 on success, 1 otherwise.
 */
int32_t __cdecl main(int32_t argc, char* argv[])
{
    std::cout << "dravex - Dungeon Runners Asset Viewer & Extractor" << std::endl;
    std::cout << "(c) 2022 atom0s [atom0s@live.com]"
              << std::endl
              << std::endl
              << "Discord: https://discord.gg/UmXNvjq"
              << std::endl
              << std::endl;
    std::cout << "Support Me via Donations:" << std::endl;
    std::cout << "  >> PayPal : https://paypal.me/atom0s" << std::endl;
    std::cout << "  >> Patreon: https://patreon.com/atom0s" << std::endl;
    std::cout << "  >> GitHub : https://github.com/sponsors/atom0s"
              << std::endl
              << std::endl;

    // Initialize the window..
    const auto window = std::make_shared<dravex::window>();
    if (!window->initialize())
        return 0;

    // Setup window callbacks..
    window->set_render_callback(on_render);
    window->set_reset_callback(on_reset);
    window->set_update_callback(on_update);

    // Initialize ImGui..
    if (!dravex::imguimgr::instance().initialize(window->get_hwnd(), window->get_d3d9dev()))
    {
        window->release();
        return 0;
    }

    // Run the application..
    window->run();

    // Cleanup..
    dravex::imguimgr::instance().release();

    // Cleanup the window..
    window->release();

    return 0;
}
