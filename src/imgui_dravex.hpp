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

#ifndef IMGUI_DRAVEX_HPP
#define IMGUI_DRAVEX_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "defines.hpp"
#include <imconfig.h>
#include <imgui.h>

#define D3DFVF_IMGUIVERTEX (D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1)
#define IMGUI_COL_TO_ARGB(c) ((c & 0xFF00FF00) | ((c & 0x00FF0000) >> 0x10) | ((c & 0x000000FF) << 0x10))

namespace dravex
{
    namespace details
    {
        /**
         * Structure definition for ImGui related rendering information.
         */
        struct renderdata_t
        {
            IDirect3DDevice9* device_;
            IDirect3DVertexBuffer9* vtx_buffer_;
            IDirect3DIndexBuffer9* idx_buffer_;
            IDirect3DTexture9* fnt_texture_;
            int32_t vtx_buffer_size_;
            int32_t idx_buffer_size_;

            renderdata_t(void)
                : device_{nullptr}
                , vtx_buffer_{nullptr}
                , idx_buffer_{nullptr}
                , fnt_texture_{nullptr}
                , vtx_buffer_size_{0}
                , idx_buffer_size_{0}
            {}
        };

        /**
         * Structure definition for ImGui related Win32 window and input information.
         */
        struct win32data_t
        {
            HWND hwnd_;
            HWND hwnd_mouse_;
            bool mouse_tracked_;
            int32_t mouse_buttons_down_;
            int64_t time_;
            int64_t ticks_per_second_;
            ImGuiMouseCursor last_mouse_cursor_;
            bool has_gamepad_;
            bool want_update_has_gamepad_;

            win32data_t(void)
                : hwnd_{nullptr}
                , hwnd_mouse_{nullptr}
                , mouse_tracked_{false}
                , mouse_buttons_down_{0}
                , time_{0}
                , ticks_per_second_{0}
                , last_mouse_cursor_{ImGuiMouseCursor_None}
                , has_gamepad_{false}
                , want_update_has_gamepad_{false}
            {}
        };

        /**
         * Structure definition for ImGui related vertex information.
         */
        struct vertex_t
        {
            float pos_[3];
            D3DCOLOR col_;
            float tuv_[2];
        };

    } // namespace details

    class imguimgr final
    {
        imguimgr(imguimgr const&)            = delete;
        imguimgr(imguimgr&&)                 = delete;
        imguimgr& operator=(imguimgr const&) = delete;
        imguimgr& operator=(imguimgr&&)      = delete;

        imguimgr(void);
        ~imguimgr(void);

        void create_font_texture(void);

    public:
        static imguimgr& instance(void);

        details::renderdata_t* get_render_data(void);
        details::win32data_t* get_win32_data(void);

        bool initialize(const HWND hwnd, IDirect3DDevice9* device);
        void release(void);

        void prereset(void);
        void postreset(void);

        void beginscene(void);
        void endscene(void);
        void render(void);
    };

} // namespace dravex

#endif // IMGUI_DRAVEX_HPP
