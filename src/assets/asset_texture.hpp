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

#ifndef ASSET_TEXTURE_HPP
#define ASSET_TEXTURE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "asset.hpp"

namespace dravex::assets
{
    class asset_texture final : public asset
    {
        IDirect3DDevice9* device_;
        uint32_t file_type_;
        IDirect3DTexture9* texture_;
        D3DSURFACE_DESC desc_;
        ImVec4 color_;
        float zoom_;

    public:
        /**
         * Constructor and Destructor
         */
        asset_texture(void)
            : device_{nullptr}
            , file_type_{0}
            , texture_{nullptr}
            , desc_{}
            , color_{1.0f, 1.0f, 1.0f, 1.0f}
            , zoom_{1.0f}
        {}
        ~asset_texture(void)
        {
            this->release();
        }

        /**
         * Initializes the asset, preparing it for viewing.
         *
         * @param {IDirect3DDevice9*} device - The Direct3D device pointer.
         * @param {uint32_t} file_type - The asset file type.
         * @param {std::vector&} data - The asset raw data.
         * @return {bool} True on success, false otherwise.
         */
        bool initialize(IDirect3DDevice9* device, const uint32_t file_type, const std::vector<uint8_t>& data)
        {
            this->device_ = device;
            this->device_->AddRef();
            this->file_type_ = file_type;

            // Load the texture from the asset data..
            if (FAILED(::D3DXCreateTextureFromFileInMemory(device, data.data(), data.size(), &this->texture_)))
            {
                dravex::logging::instance().log(dravex::loglevel::error, "[asset::texture] failed to create texture..");
                return false;
            }
            if (FAILED(this->texture_->GetLevelDesc(0, &this->desc_)))
            {
                dravex::logging::instance().log(dravex::loglevel::error, "[asset::texture] failed to obtain texture information..");
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
            if (this->texture_ == nullptr)
                return;

            ImGui::PushItemWidth(ImGui::GetFontSize() * -56);
            {
                // Display information about the texture..
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Width :");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), std::format("{}", this->desc_.Width).c_str());
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "Height:");
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), std::format("{}", this->desc_.Height).c_str());

                // Display property helpers..
                ImGui::Separator();
                ImGui::ColorEdit4("Color Mask", static_cast<float*>(&this->color_.x));
                ImGui::DragFloat("Zoom", &this->zoom_, 0.25f, 0.25f, 25.0f);
                ImGui::Separator();
                ImGui::NewLine();

                // Display the asset..
                ImGui::BeginChild("##dravex_texture", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
                ImGui::Image(this->texture_, ImVec2(this->desc_.Width * this->zoom_, this->desc_.Height * this->zoom_), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), this->color_);
                ImGui::EndChild();
            }
            ImGui::PopItemWidth();

            // Handle zooming while shift is held and the mouse wheel is scrolled..
            if (ImGui::IsKeyDown(ImGuiKey_LeftShift))
            {
                const auto wheel = ImGui::GetIO().MouseWheel;
                if (wheel != 0)
                {
                    if (wheel > 0)
                        this->zoom_ += 0.25f;
                    else
                        this->zoom_ -= 0.25f;

                    this->zoom_ = std::clamp(this->zoom_, 0.25f, 25.0f);
                }
            }
        }
    };

} // namespace dravex::assets

#endif // ASSET_TEXTURE_HPP
