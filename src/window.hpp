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

#ifndef WINDOW_HPP
#define WINDOW_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "defines.hpp"

namespace dravex
{
    class window final
    {
        HWND hwnd_;
        HINSTANCE instance_;

        D3DPRESENT_PARAMETERS present_params_;
        IDirect3D9* d3d9_;
        IDirect3DDevice9* d3d9dev_;

        std::function<void(void)> on_update_;
        std::function<void(void)> on_render_;
        std::function<bool(const bool)> on_reset_;

    public:
        window(void);
        ~window(void);

        static LRESULT WINAPI window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
        LRESULT WINAPI on_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

        bool initialize(void);
        void release(void);
        void run();

        bool reset_device(void);

        void set_render_callback(std::function<void(void)> callback);
        void set_reset_callback(std::function<bool(bool)> callback);
        void set_update_callback(std::function<void(void)> callback);

        GET_SET(HWND, hwnd);
        GET_SET(IDirect3D9*, d3d9);
        GET_SET(IDirect3DDevice9*, d3d9dev);
    };

} // namespace dravex

#endif // WINDOW_HPP
