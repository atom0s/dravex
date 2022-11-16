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
#include "imgui_internal.h"
#include "imgui_dravex.hpp"
#include "imgui_fontawesome.hpp"
#include "imgui_fontawesome_brands.hpp"
#include "package/package.hpp"
#include "window.hpp"

#include "assets/asset_font.hpp"
#include "assets/asset_ogg.hpp"
#include "assets/asset_splash.hpp"
#include "assets/asset_text.hpp"
#include "assets/asset_texture.hpp"
#include "assets/asset_unknown.hpp"

/**
 * Globals
 */
std::shared_ptr<dravex::window> g_window;
std::shared_ptr<dravex::assets::asset> g_asset;
bool g_has_pending_asset       = false;
int32_t g_selected_asset_index = -1;
int32_t g_pending_asset_type   = -1;
int32_t g_pending_asset_index  = -1;

/**
 * Resets the various asset variables.
 */
void reset_asset_variables(void)
{
    g_selected_asset_index = -1;
    g_has_pending_asset    = false;
    g_pending_asset_type   = -1;
    g_pending_asset_index  = -1;

    if (g_asset)
    {
        g_asset->release();
        g_asset = nullptr;
    }
}

/**
 * Extracts the currently selected asset to disk.
 */
void extract_asset(void)
{
    // Obtain the selected asset entry information..
    const auto& entry = dravex::package::instance().get_entry(g_selected_asset_index);
    if (entry == nullptr)
        return;

    const auto& data = dravex::package::instance().get_entry_data(g_selected_asset_index);
    const auto& name = dravex::package::instance().get_string(entry->string_offset_);
    auto ext         = dravex::package::instance().get_extension(entry->file_type_);

    // Prepare the default file name..
    char file_name[MAX_PATH]{};
    ::sprintf_s(file_name, MAX_PATH, "%s%s", name == nullptr ? "(unknown)" : name, (ext == nullptr || ::strlen(ext) == 0) ? ".raw" : ext);

    // Display the save as dialog..
    OPENFILENAMEA ofn{};
    ofn.lStructSize     = sizeof(OPENFILENAMEA);
    ofn.hwndOwner       = g_window->get_hwnd();
    ofn.hInstance       = ::GetModuleHandleA(nullptr);
    ofn.lpstrFilter     = nullptr;
    ofn.nFilterIndex    = 0;
    ofn.lpstrFile       = file_name;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrFileTitle  = nullptr;
    ofn.nMaxFileTitle   = 0;
    ofn.lpstrInitialDir = nullptr;
    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt     = (ext == nullptr || ::strlen(ext) == 0) ? "raw" : ++ext;

    if (::GetSaveFileNameA(&ofn))
    {
        // Save the asset..
        FILE* f = nullptr;
        if (::fopen_s(&f, file_name, "wb") == ERROR_SUCCESS)
        {
            ::fwrite(data.data(), data.size(), 1, f);
            ::fclose(f);
        }
    }
}

/**
 * Extracts all assets to disk.
 */
void extract_assets(void)
{
    char file_path[MAX_PATH]{};

    BROWSEINFOA binfo{};
    binfo.hwndOwner      = g_window->get_hwnd();
    binfo.pszDisplayName = file_path;
    binfo.iImage         = -1;

    // Display the folder selection dialog..
    const auto ret = ::SHBrowseForFolderA(&binfo);
    if (ret == nullptr)
        return;

    // Obtain the selected export path..
    ::SHGetPathFromIDListA(ret, file_path);
    ::CoTaskMemFree(ret);

    // Prepare the root path..
    std::error_code ec{};
    std::filesystem::path root = file_path;

    // Ensure the path exists..
    if (!std::filesystem::exists(root, ec))
        std::filesystem::create_directories(root, ec);

    // Extract each asset..
    const auto entry_count = dravex::package::instance().get_entry_count();
    for (auto x = 0; x < entry_count; x++)
    {
        // Obtain the asset entry information..
        const auto& entry = dravex::package::instance().get_entry(x);
        if (entry == nullptr)
            continue;

        const auto& data = dravex::package::instance().get_entry_data(x);
        const auto& name = dravex::package::instance().get_string(entry->string_offset_);
        auto ext         = dravex::package::instance().get_extension(entry->file_type_);

        // Prepare the full path to the file..
        auto fpath = root;
        fpath /= std::format("{}{}", name, ext);

        // Ensure the path to the file exists..
        if (!std::filesystem::exists(fpath.parent_path(), ec))
            std::filesystem::create_directories(fpath.parent_path(), ec);

        // Save the asset..
        FILE* f = nullptr;
        if (::fopen_s(&f, fpath.string().c_str(), "wb") == ERROR_SUCCESS)
        {
            ::fwrite(data.data(), data.size(), 1, f);
            ::fclose(f);
        }
    }
}

/**
 * Loads an asset from its file data to be viewed.
 *
 * @param {uint32_t} file_type - The file type of the asset being loaded.
 * @param {std::vector} data - The file data of the asset.
 */
void load_asset(const uint32_t file_type, const std::vector<uint8_t>& data)
{
    switch (file_type)
    {
        case 0: // tga
        case 1: // bmp
        case 2: // dds
            g_asset = std::make_shared<dravex::assets::asset_texture>();
            g_asset->initialize(g_window->get_d3d9dev(), file_type, data);
            break;

        case 3: // ttf
            g_asset = std::make_shared<dravex::assets::asset_font>();
            g_asset->initialize(g_window->get_d3d9dev(), file_type, data);
            break;

        case 8: // ogg
            g_asset = std::make_shared<dravex::assets::asset_ogg>();
            g_asset->initialize(g_window->get_d3d9dev(), file_type, data);
            break;

        case 10: // msc
        case 11: // mig
        case 12: // dict
        case 13: // gc
        case 14: // tile
        case 15: // world
        case 16: // zone
        case 18: // fx
        case 19: // cfg
        case 20: // txt
            g_asset = std::make_shared<dravex::assets::asset_text>();
            g_asset->initialize(g_window->get_d3d9dev(), file_type, data);
            break;

        case 4:  // cobj
        case 5:  // d3d
        case 6:  // dre
        case 7:  // wav
        case 9:  // win
        case 17: // dat
        case 21: // (undefined)
        default:
            g_asset = std::make_shared<dravex::assets::asset_unknown>();
            g_asset->initialize(g_window->get_d3d9dev(), file_type, data);
            break;
    }
}

/**
 * Renders the assets list view.
 */
void render_view_assets(void)
{
    const auto entry_count = dravex::package::instance().get_entry_count();
    if (entry_count == 0)
        return;

    ImGui::BeginChild("##entry_list", ImVec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);

    ImGuiListClipper clipper;
    clipper.Begin(entry_count);

    while (clipper.Step())
    {
        for (auto x = clipper.DisplayStart; x < clipper.DisplayEnd; x++)
        {
            const auto e    = dravex::package::instance().get_entry(x);
            const auto& s   = dravex::package::instance().get_string(e->string_offset_);
            const auto& ext = dravex::package::get_extension(e->file_type_);

            if (ImGui::Selectable(std::format("{}{}##entry_{}", s == nullptr ? "(unknown)" : s, ext, x).c_str(), x == g_selected_asset_index))
            {
                if (g_selected_asset_index != x)
                {
                    reset_asset_variables();

                    g_selected_asset_index = x;

                    g_has_pending_asset   = true;
                    g_pending_asset_type  = e->file_type_;
                    g_pending_asset_index = x;
                }
            }

            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text(std::format("{}{}", s == nullptr ? "(unknown)" : s, ext).c_str());
                ImGui::Separator();
                ImGui::Text(std::format("       Compressed : {}", e->is_compressed_ ? "True" : "False").c_str());
                ImGui::Text(std::format("  Size Compressed : {}", e->size_compressed_).c_str());
                ImGui::Text(std::format("Size Uncompressed : {}", e->size_uncompressed_).c_str());
                ImGui::Text(std::format("         Checksum : {:08X} (Adler32)", e->checksum_).c_str());

                if (e->is_compressed_)
                {
                    const auto ratio = (1.0f - (static_cast<float>(e->size_compressed_) / static_cast<float>(e->size_uncompressed_))) * 100.0f;
                    ImGui::NewLine();
                    ImGui::Text(std::format("Compression Ratio : {:.2f}%%", ratio).c_str());
                }

                ImGui::EndTooltip();
            }
        }
    }

    ImGui::EndChild();
}

/**
 * Renders the logging view.
 */
void render_view_logging(void)
{
}

/**
 * Renders the main view.
 */
void render_view_main(void)
{
    if (g_asset)
        g_asset->render();
}

/**
 * Callback invoked when the scene is being rendered.
 */
void __stdcall on_render(void)
{
    // Render ImGui..
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
    // Reset ImGui resources..
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
    // Load pending asset prior to ImGui frame..
    if (g_has_pending_asset)
    {
        load_asset(g_pending_asset_type, dravex::package::instance().get_entry_data(g_pending_asset_index));
        g_has_pending_asset = false;
    }

    dravex::imguimgr::instance().beginscene();
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem(ICON_FA_FOLDER_OPEN "Open Package"))
                {
                    char file_path[MAX_PATH]{};

                    OPENFILENAMEA ofn{};
                    ofn.lStructSize     = sizeof(OPENFILENAMEA);
                    ofn.hwndOwner       = g_window->get_hwnd();
                    ofn.hInstance       = ::GetModuleHandleA(nullptr);
                    ofn.lpstrFilter     = "game.pki\0game.pki\0";
                    ofn.nFilterIndex    = 1;
                    ofn.lpstrFile       = file_path;
                    ofn.lpstrFile[0]    = '\0';
                    ofn.nMaxFile        = MAX_PATH;
                    ofn.lpstrFileTitle  = nullptr;
                    ofn.nMaxFileTitle   = 0;
                    ofn.lpstrInitialDir = nullptr;
                    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                    if (::GetOpenFileNameA(&ofn))
                    {
                        reset_asset_variables();

                        dravex::package::instance().close();
                        dravex::package::instance().open(file_path);
                    }
                }
                if (ImGui::MenuItem(ICON_FA_FOLDER_CLOSED "Close"))
                {
                    reset_asset_variables();

                    dravex::package::instance().close();
                }
                ImGui::Separator();
                if (ImGui::MenuItem(ICON_FA_ANGLE_DOWN "Extract Selected Asset"))
                    extract_asset();
                if (ImGui::MenuItem(ICON_FA_ANGLES_DOWN "Extract All Assets"))
                    extract_assets();
                ImGui::Separator();
                if (ImGui::MenuItem(ICON_FA_RECTANGLE_XMARK "Exit"))
                    ::PostQuitMessage(0);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem(ICON_FA_GITHUB "GitHub Repo"))
                    dravex::utils::open_url("https://github.com/atom0s/dravex");
                if (ImGui::MenuItem(ICON_FA_DOWNLOAD "Latest Releases"))
                    dravex::utils::open_url("https://github.com/atom0s/dravex/releases/latest");
                if (ImGui::MenuItem(ICON_FA_BUG "Bug Reports"))
                    dravex::utils::open_url("https://github.com/atom0s/dravex/issues");
                ImGui::Separator();
                if (ImGui::MenuItem(ICON_FA_CIRCLE_INFO "About dravex"))
                {
                    g_asset = std::make_shared<dravex::assets::asset_splash>();
                    g_asset->initialize(g_window->get_d3d9dev(), 0, {});
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // Prepare the dockspace..
        auto dock_window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        dock_window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        dock_window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        auto main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(main_viewport->Pos);
        ImGui::SetNextWindowSize(main_viewport->Size);
        ImGui::SetNextWindowViewport(main_viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::Begin("###dravex_main_window", nullptr, dock_window_flags);
        ImGui::PopStyleVar(3);

        // Build the dockspace default layout..
        if (ImGui::DockBuilderGetNode(ImGui::GetID("###dravex_dockspace")) == nullptr)
        {
            auto dock_id = ImGui::GetID("###dravex_dockspace");
            ImGui::DockBuilderRemoveNode(dock_id);
            ImGui::DockBuilderAddNode(dock_id);

            auto dock_loc_main = dock_id;
            auto dock_loc_left = ImGui::DockBuilderSplitNode(dock_loc_main, ImGuiDir_Left, 0.20f, nullptr, &dock_loc_main);
            auto dock_loc_down = ImGui::DockBuilderSplitNode(dock_loc_main, ImGuiDir_Down, 0.20f, nullptr, &dock_loc_main);

            ImGui::DockBuilderDockWindow("###view_assets", dock_loc_left);
            ImGui::DockBuilderDockWindow("###view_main", dock_loc_main);
            ImGui::DockBuilderDockWindow("###view_log", dock_loc_down);
            ImGui::DockBuilderFinish(dock_id);
        }

        ImGui::DockSpace(ImGui::GetID("###dravex_dockspace"));
        ImGui::End();

        // Render the asset list view..
        ImGui::Begin(ICON_FA_PUZZLE_PIECE "Assets List###view_assets");
        render_view_assets();
        ImGui::End();

        // Render the asset viewer view..
        ImGui::Begin(ICON_FA_SHARE_NODES "Asset Viewer###view_main");
        render_view_main();
        ImGui::End();

        // Render the logging view..
        ImGui::Begin(ICON_FA_LIST "Log###view_log");
        render_view_logging();
        ImGui::End();
    }
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

    /**
     * Runs the application.
     *
     * @return {bool} True on success, false otherwise.
     */
    const auto run_application = [](void) -> bool {
        // Initialize the window..
        g_window = std::make_shared<dravex::window>();
        if (!g_window->initialize())
            return false;

        // Setup window callbacks..
        g_window->set_render_callback(on_render);
        g_window->set_reset_callback(on_reset);
        g_window->set_update_callback(on_update);

        // Initialize ImGui..
        if (!dravex::imguimgr::instance().initialize(g_window->get_hwnd(), g_window->get_d3d9dev()))
            return false;

        // Default the initial asset to the splash screen..
        g_asset = std::make_shared<dravex::assets::asset_splash>();
        g_asset->initialize(g_window->get_d3d9dev(), 0, {});

        // Run the window..
        g_window->run();

        return true;
    };

    // Run the application..
    const auto ret = run_application();

    // Cleanup..
    dravex::imguimgr::instance().release();
    dravex::package::instance().close();

    if (g_asset)
        g_asset->release();

    g_window->release();

    return !ret;
}
