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

#ifndef ASSET_TEXT_HPP
#define ASSET_TEXT_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "asset.hpp"
#include "imgui_memory_editor.hpp"
#include "texteditor.hpp"

namespace dravex::assets
{
    class asset_text final : public asset
    {
        IDirect3DDevice9* device_;
        std::vector<uint8_t> data_;
        TextEditor editor_;
        TextEditor::LanguageDefinition lang_;

    public:
        /**
         * Constructor and Destructor
         */
        asset_text(void)
            : device_{nullptr}
            , editor_{}
            , lang_{}
        {}
        ~asset_text(void)
        {
            this->release();
        }

        /**
         * Initializes the asset, preparing it for viewing.
         *
         * @param {IDirect3DDevice9*} device - The Direct3D device pointer.
         * @param {std::vector&} data - The asset raw data.
         * @return {bool} True on success, false otherwise.
         */
        bool initialize(IDirect3DDevice9* device, const std::vector<uint8_t>& data)
        {
            this->device_ = device;
            this->device_->AddRef();
            this->data_ = data;

            // Convert the incoming data to chars..
            std::vector<char> str(this->data_.begin(), this->data_.end());
            str.push_back(0x00);

            // Setup the editor..
            this->lang_ = TextEditor::LanguageDefinition::CPlusPlus();
            this->editor_.SetLanguageDefinition(this->lang_);
            this->editor_.SetText(str.data());
            this->editor_.SetReadOnly(true);
            this->editor_.SetShowWhitespaces(false);

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
            if (ImGui::BeginTabBar("##text_asset_tabbar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
            {
                // View: Source Code Editor
                if (ImGui::BeginTabItem("Source Code", nullptr, ImGuiTabItemFlags_NoCloseButton | ImGuiTabItemFlags_NoCloseWithMiddleMouseButton))
                {
                    this->editor_.Render("Source Code");
                    ImGui::EndTabItem();
                }

                // View: Hex Editor
                if (ImGui::BeginTabItem("Hex View", nullptr, ImGuiTabItemFlags_NoCloseButton | ImGuiTabItemFlags_NoCloseWithMiddleMouseButton))
                {
                    static MemoryEditor mem_edit;
                    mem_edit.ReadOnly = true;
                    mem_edit.DrawContents(this->data_.data(), this->data_.size());
                    ImGui::EndTabItem();
                }
            }
            ImGui::EndTabBar();
        }
    };

} // namespace dravex::assets

#endif // ASSET_TEXT_HPP
