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

#ifndef ASSET_OGG_HPP
#define ASSET_OGG_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "asset.hpp"
#include "imgui.h"

#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

#undef STB_VORBIS_HEADER_ONLY
#include "stb_vorbis.c"

namespace dravex::assets
{
    /**
     * Miniaudio data callback used to request the next set of sample data.
     *
     * @param {ma_device*} device - The audio device object.
     * @param {void*} output - The output buffer to hold the sample data.
     * @param {void*} input - The input buffer.
     * @param {ma_uint32} frame_count - The frame count being requested.
     */
    void data_callback(ma_device* device, void* output, const void* input, ma_uint32 frame_count)
    {
        const auto vorbis = (stb_vorbis*)device->pUserData;
        if (!vorbis || vorbis->channels > 2)
            return;

        stb_vorbis_get_samples_short_interleaved(vorbis, vorbis->channels, (short*)output, frame_count * vorbis->channels);

        // Stop the player and reset to the start..
        if (stb_vorbis_get_sample_offset(vorbis) == stb_vorbis_stream_length_in_samples(vorbis))
            stb_vorbis_seek_start(vorbis);
    }

    class asset_ogg final : public asset
    {
        IDirect3DDevice9* device_;
        uint32_t file_type_;
        std::vector<uint8_t> data_;
        stb_vorbis* vorbis_;
        ma_device madevice_;

    public:
        /**
         * Constructor and Destructor
         */
        asset_ogg(void)
            : device_{nullptr}
            , file_type_{0}
            , vorbis_{nullptr}
            , madevice_{}
        {}
        ~asset_ogg(void)
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
            this->data_      = data;

            // Load the vorbis file from memory..
            this->vorbis_ = stb_vorbis_open_memory(this->data_.data(), this->data_.size(), nullptr, nullptr);
            if (this->vorbis_ == nullptr)
                return false;

            // Obtain the audio information..
            const auto info = stb_vorbis_get_info(this->vorbis_);

            // Prepare the device configuration..
            ma_device_config cfg{};
            cfg                   = ma_device_config_init(ma_device_type_playback);
            cfg.playback.format   = ma_format_s16;
            cfg.playback.channels = info.channels;
            cfg.sampleRate        = info.sample_rate;
            cfg.dataCallback      = dravex::assets::data_callback;
            cfg.pUserData         = this->vorbis_;

            // Initialize the device..
            if (ma_device_init(nullptr, &cfg, &this->madevice_) != MA_SUCCESS)
            {
                stb_vorbis_close(this->vorbis_);
                return false;
            }

            return true;
        }

        /**
         * Releases the asset resources.
         */
        void release(void)
        {
            if (this->vorbis_)
            {
                stb_vorbis_close(this->vorbis_);
                this->vorbis_ = nullptr;
            }

            ma_device_uninit(&this->madevice_);

            SAFE_RELEASE(this->device_);
        }

        /**
         * Renders the asset viewer for this asset type.
         */
        void render(void)
        {
            if (this->vorbis_ == nullptr)
            {
                ImGui::Text("Failed to load ogg file information!");
                return;
            }

            const auto display_info = [](const std::string& title, const std::string& value) -> void {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), title.c_str());
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 0.9f, 1.0f, 1.0f), value.c_str());
            };

            const auto get_timestamp = [](const uint32_t seconds) -> std::string {
                const auto s = (seconds % 60);
                const auto m = (seconds % 3600) / 60;
                const auto h = (seconds % 86400) / 3600;
                const auto d = (seconds % (86400 * 30)) / 86400;
                return std::format("{:02d}:{:02d}:{:02d}:{:02d}", d, h, m, s);
            };

            const auto info     = stb_vorbis_get_info(this->vorbis_);
            const auto soffset  = stb_vorbis_get_sample_offset(this->vorbis_);
            const auto foffset  = stb_vorbis_get_file_offset(this->vorbis_);
            const auto lsamples = stb_vorbis_stream_length_in_samples(this->vorbis_);
            const auto lseconds = stb_vorbis_stream_length_in_seconds(this->vorbis_);

            // Display information about the sound file..
            display_info("     Channels :", std::format("{}", info.channels));
            display_info("  Sample Rate :", std::format("{}", info.sample_rate));
            display_info("Total Samples :", std::format("{}", lsamples));

            ImGui::Separator();

            // Display a mini-player
            if (ImGui::Button(ICON_FA_PLAY "Play", ImVec2(110, 30)))
                ma_device_start(&this->madevice_);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_STOP "Stop", ImVec2(110, 30)))
                ma_device_stop(&this->madevice_);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FA_RECYCLE "Reset", ImVec2(110, 30)))
                stb_vorbis_seek_start(this->vorbis_);

            // Display the current and total times..
            const auto curr = get_timestamp(soffset / this->vorbis_->sample_rate);
            const auto full = get_timestamp(lseconds);

            ImGui::PushItemWidth(ImGui::GetFontSize() * -56);
            ImGui::Text(curr.c_str());
            ImGui::SameLine();

            int32_t player_offset = soffset;
            if (ImGui::SliderInt("##ogg_position", &player_offset, 0, lsamples))
                stb_vorbis_seek(this->vorbis_, player_offset);

            ImGui::SameLine();
            ImGui::Text(full.c_str());
            ImGui::PopItemWidth();
        }
    };

} // namespace dravex::assets

#endif // ASSET_OGG_HPP
