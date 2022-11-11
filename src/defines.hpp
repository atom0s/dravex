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

#ifndef DRAVEX_DEFINES_HPP
#define DRAVEX_DEFINES_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Helper Defines
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#define NOMINMAX

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Helper Macros
//
////////////////////////////////////////////////////////////////////////////////////////////////////
// clang-format off
#define SAFE_DELETE(p)         if (p) { delete p; p = nullptr; }
#define SAFE_DELETEARRAY(p)    if (p) { delete[] p; p = nullptr; }
#define SAFE_RELEASE(p)        if (p) { p->Release(); p = nullptr; }
#define SAFE_RELEASED2D(p)     if (p) { p.Release(); p = nullptr; }
#define SAFE_RELEASEDELETE(p)  if (p) { p->Release(); delete p; p = nullptr; }
#define SAFE_CALL(p, c)        if (p) { p->c; }

#define GET(t, n)              inline t get_##n(void) const { return this->##n##_; }
#define SET(t, n)              inline void set_##n(t tt) { this->##n##_ = tt; }
#define GET_SET(t, n)          GET(t, n) SET(t, n)
#define GET_STR(t, n)          inline t get_##n(void) const { return this->##n##_.c_str(); }
#define SET_STR(t, n)          inline void set_##n(t tt) { this->##n##_ = tt; }
#define GET_SET_STR(t, n)      GET_STR(t, n) SET_STR(t, n)
#define GET_O(t, n)            inline t get_##n(void) const override { return this->##n##_; }
#define SET_O(t, n)            inline void set_##n(t tt) override { this->##n##_ = tt; }
#define GET_SET_O(t, n)        GET_O(t, n) SET_O(t, n)
#define GET_O_STR(t, n)        inline t get_##n(void) const override { return this->##n##_.c_str(); }
#define SET_O_STR(t, n)        inline void set_##n(t tt) override { this->##n##_ = tt; }
#define GET_SET_O_STR(t, n)    GET_O_STR(t, n) SET_O_STR(t, n)
#define GET_REF(t, n)          inline t get_##n(void) { return this->##n##_; }
// clang-format on

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Headers and Libraries
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Version.lib")
#pragma comment(lib, "WinMM.lib")

#include <Windows.h>
#include <windowsx.h>
#include <algorithm>
#include <cctype>
#include <codecvt>
#include <csignal>
#include <eh.h>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <future>
#include <iostream>
#include <locale>
#include <map>
#include <mutex>
#include <numeric>
#include <process.h>
#include <Psapi.h>
#include <queue>
#include <ranges>
#include <regex>
#include <ShlObj.h>
#include <sstream>
#include <string>
#include <time.h>
#include <TlHelp32.h>
#include <unordered_map>
#include <utility>
#include <vector>

////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Direct3D & DirectInput Headers and Libraries
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x00000800
#endif

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "DxErr.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dinput8.lib")

#include <atlbase.h>
#include <comdef.h>
#include <wincodec.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <dinput.h>

#endif // DRAVEX_DEFINES_HPP
