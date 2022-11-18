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

#ifndef ASSET_FONT_HPP
#define ASSET_FONT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "asset.hpp"
#include "utils.hpp"
#include "imgui.h"

namespace dravex::assets
{
    /**
     * Font cache.
     */
    std::map<uint32_t, std::tuple<ImFont*, ImFont*, ImFont*, ImFont*, ImFont*, ImFont*, ImFont*>> fonts;

    class asset_font final : public asset
    {
        IDirect3DDevice9* device_;
        ImFont* font12_;
        ImFont* font18_;
        ImFont* font24_;
        ImFont* font36_;
        ImFont* font48_;
        ImFont* font60_;
        ImFont* font72_;

    public:
        /**
         * Constructor and Destructor
         */
        asset_font(void)
            : device_{nullptr}
            , font12_{nullptr}
            , font18_{nullptr}
            , font24_{nullptr}
            , font36_{nullptr}
            , font48_{nullptr}
            , font60_{nullptr}
            , font72_{nullptr}
        {}
        ~asset_font(void)
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

            const auto data     = dravex::package::instance().get_entry_data(entry->index_);
            const auto checksum = dravex::utils::adler32(data.data(), data.size());
            const auto iter     = dravex::assets::fonts.find(checksum);

            if (iter != dravex::assets::fonts.end())
            {
                this->font12_ = std::get<0>(iter->second);
                this->font18_ = std::get<1>(iter->second);
                this->font24_ = std::get<2>(iter->second);
                this->font36_ = std::get<3>(iter->second);
                this->font48_ = std::get<4>(iter->second);
                this->font60_ = std::get<5>(iter->second);
                this->font72_ = std::get<6>(iter->second);
            }
            else
            {
                ImFontConfig cfg{};
                cfg.FontDataOwnedByAtlas = false;

                this->font12_ = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(data.data()), data.size(), 12.0f, &cfg);
                this->font18_ = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(data.data()), data.size(), 18.0f, &cfg);
                this->font24_ = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(data.data()), data.size(), 24.0f, &cfg);
                this->font36_ = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(data.data()), data.size(), 36.0f, &cfg);
                this->font48_ = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(data.data()), data.size(), 48.0f, &cfg);
                this->font60_ = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(data.data()), data.size(), 60.0f, &cfg);
                this->font72_ = ImGui::GetIO().Fonts->AddFontFromMemoryTTF(const_cast<uint8_t*>(data.data()), data.size(), 72.0f, &cfg);

                ImGui::GetIO().Fonts->Build();

                dravex::assets::fonts[checksum] = std::make_tuple(
                    this->font12_,
                    this->font18_,
                    this->font24_,
                    this->font36_,
                    this->font48_,
                    this->font60_,
                    this->font72_);

                // Force the font texture to rebuild..
                dravex::imguimgr::instance().prereset();
                dravex::imguimgr::instance().postreset();
            }

            return true;
        }

        /**
         * Releases the asset resources.
         */
        void release(void)
        {
            SAFE_RELEASE(this->device_);
        }

        /**
         * Renders the asset viewer for this asset type.
         */
        void render(void)
        {
            ImGui::PushFont(this->font24_);
            ImGui::Text("abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ");
            ImGui::Text("1234567890.:,; ' \" (!?) +-*/=");
            ImGui::PopFont();
            ImGui::Separator();

            const auto render_example_text = [](ImFont* font) -> void {
                ImGui::PushFont(font);
                ImGui::Text("The quick brown fox jumped over the lazy dog.");
                ImGui::PopFont();
            };

            render_example_text(this->font12_);
            render_example_text(this->font18_);
            render_example_text(this->font24_);
            render_example_text(this->font36_);
            render_example_text(this->font48_);
            render_example_text(this->font60_);
            render_example_text(this->font72_);
        }
    };

} // namespace dravex::assets

#endif // ASSET_FONT_HPP
