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

#ifndef ASSET_SPLASH_HPP
#define ASSET_SPLASH_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "asset.hpp"
#include "../res/dravex.h"

namespace dravex::assets
{
    class asset_splash final : public asset
    {
        IDirect3DDevice9* device_;
        IDirect3DTexture9* texture_;
        D3DSURFACE_DESC desc_;

    public:
        /**
         * Constructor and Destructor
         */
        asset_splash(void)
            : device_{nullptr}
            , texture_{nullptr}
            , desc_{}
        {}
        ~asset_splash(void)
        {
            this->release();
        }

        /**
         * Initializes the asset, preparing it for viewing.
         *
         * @param {IDirect3DDevice9*} device - The Direct3D device pointer.
         * @param {std::shared_ptr&} entry - The asset entry being loaded.
         * @return {bool} True on success, false otherwise.
         */
        bool initialize(IDirect3DDevice9* device, const std::shared_ptr<dravex::fileentry_t>& entry)
        {
            this->device_ = device;
            this->device_->AddRef();

            if (FAILED(::D3DXCreateTextureFromResourceExA(device, ::GetModuleHandleA(nullptr), MAKEINTRESOURCE(IDI_APPIMAGE), D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, nullptr, nullptr, &this->texture_)))
            {
                dravex::logging::instance().log(dravex::loglevel::error, "[asset::splash] failed to create application image texture..");
                return false;
            }
            if (FAILED(this->texture_->GetLevelDesc(0, &this->desc_)))
            {
                dravex::logging::instance().log(dravex::loglevel::error, "[asset::splash] failed to obtain application image texture information..");
                return false;
            }

            return true;
        }

        /**
         * Releases the asset resources.
         */
        void release(void)
        {
            SAFE_RELEASE(this->texture_);
            SAFE_RELEASE(this->device_);
        }

        /**
         * Renders the asset viewer for this asset type.
         */
        void render(void)
        {
            const auto color1 = ImVec4(0.0f, 0.7f, 1.0f, 1.0f);
            const auto color2 = ImVec4(0.7f, 0.5f, 0.8f, 1.0f);
            const auto width  = ImGui::GetWindowWidth();

            ImGui::SetCursorPosX((width / 2) - (250 / 2));
            ImGui::Image(this->texture_, ImVec2(250, 250));

            ImGui::SetCursorPosX((width / 2) - (ImGui::CalcTextSize("dravex - Dungeon Runners Asset Viewer & Extractor").x / 2));
            ImGui::TextColored(color1, "dravex - Dungeon Runners Asset Viewer & Extractor");
            ImGui::SetCursorPosX((width / 2) - (ImGui::CalcTextSize("by atom0s").x / 2));
            ImGui::TextColored(color1, "by atom0s");

            ImGui::NewLine();
            ImGui::SetCursorPosX((width / 2) - (ImGui::CalcTextSize("Support my work by sponsoring or donating!").x / 2));
            ImGui::TextColored(color2, "Support my work by sponsoring or donating!");
            ImGui::SetCursorPosX((width / 2) - (((150 * 3) + (7 * 3)) / 2));
            if (ImGui::Button(ICON_FA_GITHUB "GitHub", ImVec2(150, 30)))
                dravex::utils::open_url("https://github.com/sponsors/atom0s");
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_PATREON "Patreon", ImVec2(150, 30)))
                dravex::utils::open_url("https://patreon.com/atom0s");
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_PAYPAL "PayPal", ImVec2(150, 30)))
                dravex::utils::open_url("https://paypal.me/atom0s");

            ImGui::NewLine();
            ImGui::SetCursorPosX((width / 2) - (ImGui::CalcTextSize("Project Links").x / 2));
            ImGui::TextColored(color2, "Project Links");
            ImGui::SetCursorPosX((width / 2) - (((150 * 3) + (7 * 3)) / 2));
            if (ImGui::Button(ICON_FA_HOUSE "Homepage", ImVec2(150, 30)))
                dravex::utils::open_url("https://github.com/atom0s/dravex");
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_BUG "Bug Reports", ImVec2(150, 30)))
                dravex::utils::open_url("https://github.com/atom0s/dravex/issues");
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_DOWNLOAD "Latest Release", ImVec2(150, 30)))
                dravex::utils::open_url("https://github.com/atom0s/Steamless/releases/latest");

            ImGui::NewLine();
            ImGui::SetCursorPosX((width / 2) - (ImGui::CalcTextSize("Join my personal Discord for updates and other discussions.").x / 2));
            ImGui::TextColored(color2, "Join my personal Discord for updates and other discussions.");
            ImGui::SetCursorPosX((width / 2) - (150 / 2) - 2);
            if (ImGui::Button(ICON_FA_DISCORD "Join Discord", ImVec2(150, 30)))
                dravex::utils::open_url("https://discord.gg/UmXNvjq");
        }
    };

} // namespace dravex::assets

#endif // ASSET_SPLASH_HPP
