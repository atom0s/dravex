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
#include "logging.hpp"
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
int32_t g_pending_asset_index  = -1;

/**
 * Globals (Extraction Overlay)
 */
int32_t g_extract_modal_index              = 0;
std::filesystem::path g_extract_modal_path = "";
bool g_extract_modal_show                  = false;
std::thread g_extract_modal_thread;

/**
 * Resets the various asset variables.
 */
void reset_asset_variables(void)
{
    g_selected_asset_index = -1;
    g_has_pending_asset    = false;
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
        else
            dravex::logging::instance().log(dravex::loglevel::error, std::format("[extract] failed to extract asset to path: {}", file_name));
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
    binfo.ulFlags        = BIF_NEWDIALOGSTYLE;

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

    // Mark the extraction overlay to display..
    g_extract_modal_index = 0;
    g_extract_modal_path  = root;
    g_extract_modal_show  = true;

    // Start the extract thread..
    g_extract_modal_thread = std::thread([]() {
        std::vector<int32_t> failed_index;
        std::vector<std::filesystem::path> failed_paths;

        while (g_extract_modal_show)
        {
            if (g_extract_modal_index >= dravex::package::instance().get_entry_count())
                break;

            // Obtain the asset entry information..
            const auto& entry = dravex::package::instance().get_entry(g_extract_modal_index);
            if (entry == nullptr)
            {
                failed_index.push_back(g_extract_modal_index);

                g_extract_modal_index++;
                continue;
            }

            // Obtain the entry information..
            const auto& data = dravex::package::instance().get_entry_data(g_extract_modal_index);
            const auto& name = dravex::package::instance().get_string(entry->string_offset_);
            auto ext         = dravex::package::instance().get_extension(entry->file_type_);

            std::error_code ec{};

            // Prepare the full path to the file..
            auto fpath = g_extract_modal_path;
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
            else
                failed_paths.push_back(fpath);

            // Step to the next asset..
            g_extract_modal_index++;
        }

        dravex::logging::instance().log(dravex::loglevel::info, "[extract] extract all assets completed.");

        if (failed_index.size() > 0)
        {
            for (const auto& i : failed_index)
                dravex::logging::instance().log(dravex::loglevel::error, std::format("[extract] failed to extract asset at index: {}", i));
        }

        if (failed_paths.size() > 0)
        {
            for (const auto& p : failed_paths)
                dravex::logging::instance().log(dravex::loglevel::error, std::format("[extract] failed to extract asset to path: {}", p.string()));
        }
    });
}

/**
 * Displays the asset extraction overlay.
 */
void extract_assets_overlay(void)
{
    if (g_extract_modal_show)
        ImGui::OpenPopup("###dravex_extract_overlay");

    const auto size   = ImGui::GetIO().DisplaySize;
    const auto center = ImVec2(size.x * 0.5f, size.y * 0.5f);

    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(-1, -1), ImGuiCond_Always);

    if (ImGui::BeginPopupModal("Extracting..###dravex_extract_overlay", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        const auto entry_count = dravex::package::instance().get_entry_count();

        ImGui::Text(std::format("Extracting asset {} of {}..", g_extract_modal_index, entry_count).c_str());
        ImGui::ProgressBar(static_cast<float>(g_extract_modal_index) / static_cast<float>(entry_count));
        ImGui::Separator();

        if (ImGui::Button("Cancel") || g_extract_modal_index >= entry_count)
        {
            g_extract_modal_index = 0;
            g_extract_modal_path  = "";
            g_extract_modal_show  = false;
            g_extract_modal_thread.join();

            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

/**
 * Loads the selected asset to be viewed.
 */
void load_selected_asset(void)
{
    const auto& e = dravex::package::instance().get_entry(g_pending_asset_index);
    if (e == nullptr)
        return;

    switch (e->file_type_)
    {
        case 0: // tga
        case 1: // bmp
        case 2: // dds
            g_asset = std::make_shared<dravex::assets::asset_texture>();
            g_asset->initialize(g_window->get_d3d9dev(), e);
            break;

        case 3: // ttf
            g_asset = std::make_shared<dravex::assets::asset_font>();
            g_asset->initialize(g_window->get_d3d9dev(), e);
            break;

        case 8: // ogg
            g_asset = std::make_shared<dravex::assets::asset_ogg>();
            g_asset->initialize(g_window->get_d3d9dev(), e);
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
            g_asset->initialize(g_window->get_d3d9dev(), e);
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
            g_asset->initialize(g_window->get_d3d9dev(), e);
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
                    g_has_pending_asset    = true;
                    g_pending_asset_index  = x;
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
    dravex::logging::instance().render();
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
        load_selected_asset();
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
                    g_asset->initialize(g_window->get_d3d9dev(), nullptr);
                }
                ImGui::EndMenu();
            }

            ImGui::EndMainMenuBar();
        }

        // Display extraction overlay..
        extract_assets_overlay();

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
            auto dock_loc_down = ImGui::DockBuilderSplitNode(dock_loc_main, ImGuiDir_Down, 0.21f, nullptr, &dock_loc_main);

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
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin(ICON_FA_LIST "Log###view_log");
        render_view_logging();
        ImGui::End();
        ImGui::PopStyleVar();
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
    std::cout << "Support Me via Donations:"
              << std::endl
              << "  >> PayPal : https://paypal.me/atom0s"
              << std::endl
              << "  >> Patreon: https://patreon.com/atom0s"
              << std::endl
              << "  >> GitHub : https://github.com/sponsors/atom0s"
              << std::endl
              << std::endl;

    dravex::logging::instance().log(dravex::loglevel::info, "[info] dravex - Dungeon Runners Asset Viewer & Extractor - (c) 2022 atom0s [atom0s@live.com]");

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
        {
            std::cout << "[!] Error: Failed to initialize ImGui." << std::endl;
            return false;
        }

        // Default the initial asset to the splash screen..
        g_asset = std::make_shared<dravex::assets::asset_splash>();
        g_asset->initialize(g_window->get_d3d9dev(), nullptr);

        // Run the window..
        g_window->run();

        return true;
    };

    // Run the application..
    const auto ret = run_application();

    // Stop the extraction thread if running..
    if (g_extract_modal_show || g_extract_modal_thread.joinable())
    {
        g_extract_modal_show = false;
        g_extract_modal_thread.join();
    }

    // Cleanup..
    dravex::imguimgr::instance().release();
    dravex::package::instance().close();

    if (g_asset)
        g_asset->release();

    g_window->release();

    return !ret;
}
