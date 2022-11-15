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

#include "imgui_dravex.hpp"
#include "imgui_fontawesome.hpp"
#include "imgui_fontawesome_brands.hpp"

#ifndef GWL_WNDPROC
#define GWL_WNDPROC -4
#endif
#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif
#ifndef DBT_DEVNODES_CHANGED
#define DBT_DEVNODES_CHANGED 0x0007
#endif

#define IM_VK_KEYPAD_ENTER (VK_RETURN + 256)

namespace dravex
{
    WNDPROC hook_wndproc = nullptr;

    /**
     * Updates the current cursor icon based on the current ImGui status.
     *
     * @return {bool} True if changed, false otherwise.
     */
    bool update_mouse_cursor(void)
    {
        auto& io = ImGui::GetIO();

        if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
            return false;

        const auto cursor = ImGui::GetMouseCursor();
        if (cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
            ::SetCursor(nullptr);
        else
        {
            auto win32_cursor = IDC_ARROW;

            switch (cursor)
            {
                case ImGuiMouseCursor_Arrow:
                    win32_cursor = IDC_ARROW;
                    break;
                case ImGuiMouseCursor_TextInput:
                    win32_cursor = IDC_IBEAM;
                    break;
                case ImGuiMouseCursor_ResizeAll:
                    win32_cursor = IDC_SIZEALL;
                    break;
                case ImGuiMouseCursor_ResizeEW:
                    win32_cursor = IDC_SIZEWE;
                    break;
                case ImGuiMouseCursor_ResizeNS:
                    win32_cursor = IDC_SIZENS;
                    break;
                case ImGuiMouseCursor_ResizeNESW:
                    win32_cursor = IDC_SIZENESW;
                    break;
                case ImGuiMouseCursor_ResizeNWSE:
                    win32_cursor = IDC_SIZENWSE;
                    break;
                case ImGuiMouseCursor_Hand:
                    win32_cursor = IDC_HAND;
                    break;
                case ImGuiMouseCursor_NotAllowed:
                    win32_cursor = IDC_NO;
                    break;
                default:
                    break;
            }

            ::SetCursor(::LoadCursor(nullptr, win32_cursor));
        }

        return true;
    }

    /**
     * Returns if the given virtual key code is down.
     *
     * @param {int32_t} vk - The virtual key code to test.
     * @return {bool} True if the key is down, false otherwise.
     */
    bool is_vk_down(const int32_t vk)
    {
        return (::GetKeyState(vk) & 0x8000) != 0;
    }

    /**
     * Injects a key event into the current ImGui key cache.
     *
     * @param {ImGuiKey} key - The ImGui key code.
     * @param {bool} down - Flag stating if the key is pressed down or not.
     * @param {int32_t} native_keycode - The native key code of the key.
     * @param {int32_t} native_scancode - The native scan code of the key.
     */
    void add_key_event(const ImGuiKey key, const bool down, const int32_t native_keycode, const int32_t native_scancode = -1)
    {
        auto& io = ImGui::GetIO();

        io.AddKeyEvent(key, down);
        io.SetKeyEventNativeData(key, native_keycode, native_scancode);
    }

    /**
     * Injects special key presses for the current frame, if needed. (Shift, Windows)
     */
    void process_key_events(void)
    {
        if (ImGui::IsKeyDown(ImGuiKey_LeftShift) && !is_vk_down(VK_LSHIFT))
            add_key_event(ImGuiKey_LeftShift, false, VK_LSHIFT);

        if (ImGui::IsKeyDown(ImGuiKey_RightShift) && !is_vk_down(VK_RSHIFT))
            add_key_event(ImGuiKey_RightShift, false, VK_RSHIFT);

        if (ImGui::IsKeyDown(ImGuiKey_LeftSuper) && !is_vk_down(VK_LWIN))
            add_key_event(ImGuiKey_LeftSuper, false, VK_LWIN);

        if (ImGui::IsKeyDown(ImGuiKey_RightSuper) && !is_vk_down(VK_RWIN))
            add_key_event(ImGuiKey_RightSuper, false, VK_RWIN);
    }

    /**
     * Injects the current modifier key states for the current frame. (Control, Shift, Alt, Apps)
     */
    void update_key_modifiers(void)
    {
        auto& io = ImGui::GetIO();

        io.AddKeyEvent(ImGuiKey_ModCtrl, is_vk_down(VK_CONTROL));
        io.AddKeyEvent(ImGuiKey_ModShift, is_vk_down(VK_SHIFT));
        io.AddKeyEvent(ImGuiKey_ModAlt, is_vk_down(VK_MENU));
        io.AddKeyEvent(ImGuiKey_ModSuper, is_vk_down(VK_APPS));
    }

    /**
     * Updates the mouse cursor position for the current frame.
     */
    void update_mouse_data(void)
    {
        const auto wdata = dravex::imguimgr::instance().get_win32_data();
        if (wdata == nullptr)
            return;

        auto& io = ImGui::GetIO();

        if (::GetForegroundWindow() == wdata->hwnd_)
        {
            if (io.WantSetMousePos)
            {
                POINT pos{static_cast<int32_t>(io.MousePos.x), static_cast<int32_t>(io.MousePos.y)};

                if (::ClientToScreen(wdata->hwnd_, &pos))
                    ::SetCursorPos(pos.x, pos.y);
            }

            if (!io.WantSetMousePos && !wdata->mouse_tracked_)
            {
                POINT pos{};
                if (::GetCursorPos(&pos) && ::ScreenToClient(wdata->hwnd_, &pos))
                    io.AddMousePosEvent(static_cast<float>(pos.x), static_cast<float>(pos.y));
            }
        }
    }

    /**
     * Returns the ImGui key id for the given virtual key code.
     *
     * @param {WPARAM} wparam - The WPARAM value holding the virtual key code.
     * @return {ImGuiKey} The converted ImGuiKey value.
     */
    ImGuiKey vk_to_imgui_key(const WPARAM wparam)
    {
        switch (wparam)
        {
            case VK_TAB:
                return ImGuiKey_Tab;
            case VK_LEFT:
                return ImGuiKey_LeftArrow;
            case VK_RIGHT:
                return ImGuiKey_RightArrow;
            case VK_UP:
                return ImGuiKey_UpArrow;
            case VK_DOWN:
                return ImGuiKey_DownArrow;
            case VK_PRIOR:
                return ImGuiKey_PageUp;
            case VK_NEXT:
                return ImGuiKey_PageDown;
            case VK_HOME:
                return ImGuiKey_Home;
            case VK_END:
                return ImGuiKey_End;
            case VK_INSERT:
                return ImGuiKey_Insert;
            case VK_DELETE:
                return ImGuiKey_Delete;
            case VK_BACK:
                return ImGuiKey_Backspace;
            case VK_SPACE:
                return ImGuiKey_Space;
            case VK_RETURN:
                return ImGuiKey_Enter;
            case VK_ESCAPE:
                return ImGuiKey_Escape;
            case VK_OEM_7:
                return ImGuiKey_Apostrophe;
            case VK_OEM_COMMA:
                return ImGuiKey_Comma;
            case VK_OEM_MINUS:
                return ImGuiKey_Minus;
            case VK_OEM_PERIOD:
                return ImGuiKey_Period;
            case VK_OEM_2:
                return ImGuiKey_Slash;
            case VK_OEM_1:
                return ImGuiKey_Semicolon;
            case VK_OEM_PLUS:
                return ImGuiKey_Equal;
            case VK_OEM_4:
                return ImGuiKey_LeftBracket;
            case VK_OEM_5:
                return ImGuiKey_Backslash;
            case VK_OEM_6:
                return ImGuiKey_RightBracket;
            case VK_OEM_3:
                return ImGuiKey_GraveAccent;
            case VK_CAPITAL:
                return ImGuiKey_CapsLock;
            case VK_SCROLL:
                return ImGuiKey_ScrollLock;
            case VK_NUMLOCK:
                return ImGuiKey_NumLock;
            case VK_SNAPSHOT:
                return ImGuiKey_PrintScreen;
            case VK_PAUSE:
                return ImGuiKey_Pause;
            case VK_NUMPAD0:
                return ImGuiKey_Keypad0;
            case VK_NUMPAD1:
                return ImGuiKey_Keypad1;
            case VK_NUMPAD2:
                return ImGuiKey_Keypad2;
            case VK_NUMPAD3:
                return ImGuiKey_Keypad3;
            case VK_NUMPAD4:
                return ImGuiKey_Keypad4;
            case VK_NUMPAD5:
                return ImGuiKey_Keypad5;
            case VK_NUMPAD6:
                return ImGuiKey_Keypad6;
            case VK_NUMPAD7:
                return ImGuiKey_Keypad7;
            case VK_NUMPAD8:
                return ImGuiKey_Keypad8;
            case VK_NUMPAD9:
                return ImGuiKey_Keypad9;
            case VK_DECIMAL:
                return ImGuiKey_KeypadDecimal;
            case VK_DIVIDE:
                return ImGuiKey_KeypadDivide;
            case VK_MULTIPLY:
                return ImGuiKey_KeypadMultiply;
            case VK_SUBTRACT:
                return ImGuiKey_KeypadSubtract;
            case VK_ADD:
                return ImGuiKey_KeypadAdd;
            case IM_VK_KEYPAD_ENTER:
                return ImGuiKey_KeypadEnter;
            case VK_LSHIFT:
                return ImGuiKey_LeftShift;
            case VK_LCONTROL:
                return ImGuiKey_LeftCtrl;
            case VK_LMENU:
                return ImGuiKey_LeftAlt;
            case VK_LWIN:
                return ImGuiKey_LeftSuper;
            case VK_RSHIFT:
                return ImGuiKey_RightShift;
            case VK_RCONTROL:
                return ImGuiKey_RightCtrl;
            case VK_RMENU:
                return ImGuiKey_RightAlt;
            case VK_RWIN:
                return ImGuiKey_RightSuper;
            case VK_APPS:
                return ImGuiKey_Menu;
            case '0':
                return ImGuiKey_0;
            case '1':
                return ImGuiKey_1;
            case '2':
                return ImGuiKey_2;
            case '3':
                return ImGuiKey_3;
            case '4':
                return ImGuiKey_4;
            case '5':
                return ImGuiKey_5;
            case '6':
                return ImGuiKey_6;
            case '7':
                return ImGuiKey_7;
            case '8':
                return ImGuiKey_8;
            case '9':
                return ImGuiKey_9;
            case 'A':
                return ImGuiKey_A;
            case 'B':
                return ImGuiKey_B;
            case 'C':
                return ImGuiKey_C;
            case 'D':
                return ImGuiKey_D;
            case 'E':
                return ImGuiKey_E;
            case 'F':
                return ImGuiKey_F;
            case 'G':
                return ImGuiKey_G;
            case 'H':
                return ImGuiKey_H;
            case 'I':
                return ImGuiKey_I;
            case 'J':
                return ImGuiKey_J;
            case 'K':
                return ImGuiKey_K;
            case 'L':
                return ImGuiKey_L;
            case 'M':
                return ImGuiKey_M;
            case 'N':
                return ImGuiKey_N;
            case 'O':
                return ImGuiKey_O;
            case 'P':
                return ImGuiKey_P;
            case 'Q':
                return ImGuiKey_Q;
            case 'R':
                return ImGuiKey_R;
            case 'S':
                return ImGuiKey_S;
            case 'T':
                return ImGuiKey_T;
            case 'U':
                return ImGuiKey_U;
            case 'V':
                return ImGuiKey_V;
            case 'W':
                return ImGuiKey_W;
            case 'X':
                return ImGuiKey_X;
            case 'Y':
                return ImGuiKey_Y;
            case 'Z':
                return ImGuiKey_Z;
            case VK_F1:
                return ImGuiKey_F1;
            case VK_F2:
                return ImGuiKey_F2;
            case VK_F3:
                return ImGuiKey_F3;
            case VK_F4:
                return ImGuiKey_F4;
            case VK_F5:
                return ImGuiKey_F5;
            case VK_F6:
                return ImGuiKey_F6;
            case VK_F7:
                return ImGuiKey_F7;
            case VK_F8:
                return ImGuiKey_F8;
            case VK_F9:
                return ImGuiKey_F9;
            case VK_F10:
                return ImGuiKey_F10;
            case VK_F11:
                return ImGuiKey_F11;
            case VK_F12:
                return ImGuiKey_F12;
            default:
                break;
        }

        return ImGuiKey_None;
    }

    /**
     * Window message hook callback used to allow ImGui to process window messages first before the application.
     *
     * @param {HWND} hwnd - The window handle that caused the message.
     * @param {uint32_t} msg - The message being processed.
     * @param {WPARAM} wparam - The WPARAM value of the message.
     * @param {LPARAM} lparam - The LPARAM value of the message.
     * @return {LRESULT} The message result.
     */
    LRESULT __stdcall ImGui_WindowProc(const HWND hwnd, const uint32_t msg, const WPARAM wparam, const LPARAM lparam)
    {
        const auto wdata = dravex::imguimgr::instance().get_win32_data();
        if (!wdata)
            return 0;

        auto& io = ImGui::GetIO();

        switch (msg)
        {
            case WM_MOUSEMOVE:
            {
                wdata->hwnd_mouse_ = hwnd;

                if (!wdata->mouse_tracked_)
                {
                    TRACKMOUSEEVENT tme{sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, 0};
                    ::TrackMouseEvent(&tme);
                    wdata->mouse_tracked_ = true;
                }

                io.AddMousePosEvent(static_cast<float>(GET_X_LPARAM(lparam)), static_cast<float>(GET_Y_LPARAM(lparam)));
                break;
            }

            case WM_MOUSELEAVE:
            {
                if (wdata->hwnd_mouse_ == hwnd)
                    wdata->hwnd_mouse_ = nullptr;

                wdata->mouse_tracked_ = false;

                io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
                break;
            }

            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
            case WM_MBUTTONDOWN:
            case WM_XBUTTONDOWN:
            case WM_LBUTTONDBLCLK:
            case WM_RBUTTONDBLCLK:
            case WM_MBUTTONDBLCLK:
            case WM_XBUTTONDBLCLK:
            {
                int32_t button = 0;

                if (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONDBLCLK)
                    button = 0;
                if (msg == WM_RBUTTONDOWN || msg == WM_RBUTTONDBLCLK)
                    button = 1;
                if (msg == WM_MBUTTONDOWN || msg == WM_MBUTTONDBLCLK)
                    button = 2;
                if (msg == WM_XBUTTONDOWN || msg == WM_XBUTTONDBLCLK)
                    button = (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) ? 3 : 4;

                if (wdata->mouse_buttons_down_ == 0 && ::GetCapture() == nullptr)
                    ::SetCapture(hwnd);

                wdata->mouse_buttons_down_ |= 1 << button;
                io.AddMouseButtonEvent(button, true);
                return 0;
            }

            case WM_LBUTTONUP:
            case WM_RBUTTONUP:
            case WM_MBUTTONUP:
            case WM_XBUTTONUP:
            {
                int32_t button = 0;

                if (msg == WM_LBUTTONUP)
                    button = 0;
                if (msg == WM_RBUTTONUP)
                    button = 1;
                if (msg == WM_MBUTTONUP)
                    button = 2;
                if (msg == WM_XBUTTONUP)
                    button = (GET_XBUTTON_WPARAM(wparam) == XBUTTON1) ? 3 : 4;

                wdata->mouse_buttons_down_ &= ~(1 << button);

                if (wdata->mouse_buttons_down_ == 0 && ::GetCapture() == hwnd)
                    ::ReleaseCapture();

                io.AddMouseButtonEvent(button, false);
                return 0;
            }

            case WM_MOUSEWHEEL:
                io.AddMouseWheelEvent(0.0f, static_cast<float>(GET_WHEEL_DELTA_WPARAM(wparam)) / static_cast<float>(WHEEL_DELTA));
                return 0;

            case WM_MOUSEHWHEEL:
                io.AddMouseWheelEvent(static_cast<float>(GET_WHEEL_DELTA_WPARAM(wparam)) / static_cast<float>(WHEEL_DELTA), 0.0f);
                return 0;

            case WM_KEYDOWN:
            case WM_KEYUP:
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            {
                const auto is_down = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
                if (wparam < 256)
                {
                    update_key_modifiers();

                    auto vk = static_cast<int32_t>(wparam);
                    if ((wparam == VK_RETURN) && (HIWORD(lparam) & KF_EXTENDED))
                        vk = IM_VK_KEYPAD_ENTER;

                    const auto key      = vk_to_imgui_key(vk);
                    const auto scancode = static_cast<int32_t>(LOBYTE(HIWORD(lparam)));

                    if (key != ImGuiKey_None)
                        add_key_event(key, is_down, vk, scancode);

                    if (vk == VK_SHIFT)
                    {
                        if (is_vk_down(VK_LSHIFT) == is_down)
                            add_key_event(ImGuiKey_LeftShift, is_down, VK_LSHIFT, scancode);
                        if (is_vk_down(VK_RSHIFT) == is_down)
                            add_key_event(ImGuiKey_RightShift, is_down, VK_RSHIFT, scancode);
                    }
                    else if (vk == VK_CONTROL)
                    {
                        if (is_vk_down(VK_LCONTROL) == is_down)
                            add_key_event(ImGuiKey_LeftCtrl, is_down, VK_LCONTROL, scancode);
                        if (is_vk_down(VK_RCONTROL) == is_down)
                            add_key_event(ImGuiKey_RightCtrl, is_down, VK_RCONTROL, scancode);
                    }
                    else if (vk == VK_MENU)
                    {
                        if (is_vk_down(VK_LMENU) == is_down)
                            add_key_event(ImGuiKey_LeftAlt, is_down, VK_LMENU, scancode);
                        if (is_vk_down(VK_RMENU) == is_down)
                            add_key_event(ImGuiKey_RightAlt, is_down, VK_RMENU, scancode);
                    }
                }

                return 0;
            }

            case WM_SETFOCUS:
            case WM_KILLFOCUS:
                io.AddFocusEvent(msg == WM_SETFOCUS);
                return 0;

            case WM_CHAR:
            {
                if (wparam > 0 && wparam < 0x10000)
                    io.AddInputCharacterUTF16(static_cast<uint16_t>(wparam));

                return 0;
            }

            case WM_SETCURSOR:
            {
                if (LOWORD(lparam) == HTCLIENT && update_mouse_cursor())
                    return 1;

                return 0;
            }

            case WM_DEVICECHANGE:
            {
                if (static_cast<uint32_t>(wparam) == DBT_DEVNODES_CHANGED)
                    wdata->want_update_has_gamepad_ = true;

                return 0;
            }

            default:
                break;
        }

        return 0;
    }

    /**
     * Window message hook callback used to allow ImGui to process window messages first before the application.
     *
     * @param {HWND} hwnd - The window handle that caused the message.
     * @param {uint32_t} msg - The message being processed.
     * @param {WPARAM} wparam - The WPARAM value of the message.
     * @param {LPARAM} lparam - The LPARAM value of the message.
     * @return {LRESULT} The message result.
     */
    LRESULT __stdcall WindowProc(const HWND hwnd, const uint32_t msg, const WPARAM wparam, const LPARAM lparam)
    {
        if (ImGui::GetCurrentContext() == nullptr)
            return ::CallWindowProcA(hook_wndproc, hwnd, msg, wparam, lparam);

        if (ImGui_WindowProc(hwnd, msg, wparam, lparam))
            return 1;

        auto& io = ImGui::GetIO();

        if (msg >= WM_KEYFIRST && msg <= WM_KEYLAST)
        {
            if (io.WantCaptureKeyboard || io.WantTextInput || ImGui::IsAnyItemActive())
                return 1;
        }

        if ((msg >= WM_MOUSEFIRST && msg <= WM_MOUSELAST) || msg == WM_INPUT)
        {
            if (io.WantCaptureMouse)
                return 1;
        }

        return ::CallWindowProcA(hook_wndproc, hwnd, msg, wparam, lparam);
    }

    /**
     * Sets up the render state for ImGui to render properly.
     *
     * @param {ImDrawData*} draw_data - The ImGui draw data information being rendered.
     */
    void setup_render_state(const ImDrawData* draw_data)
    {
        const auto data = dravex::imguimgr::instance().get_render_data();
        if (data == nullptr)
            return;

        D3DVIEWPORT9 view{};
        view.X      = 0;
        view.Y      = 0;
        view.Width  = static_cast<uint32_t>(draw_data->DisplaySize.x);
        view.Height = static_cast<uint32_t>(draw_data->DisplaySize.y);
        view.MinZ   = 0.0f;
        view.MaxZ   = 1.0f;

        data->device_->SetViewport(&view);
        data->device_->SetPixelShader(nullptr);
        data->device_->SetVertexShader(nullptr);
        data->device_->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
        data->device_->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
        data->device_->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
        data->device_->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        data->device_->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        data->device_->SetRenderState(D3DRS_ZENABLE, FALSE);
        data->device_->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        data->device_->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        data->device_->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        data->device_->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        data->device_->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
        data->device_->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
        data->device_->SetRenderState(D3DRS_DESTBLENDALPHA, D3DBLEND_INVSRCALPHA);
        data->device_->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
        data->device_->SetRenderState(D3DRS_FOGENABLE, FALSE);
        data->device_->SetRenderState(D3DRS_RANGEFOGENABLE, FALSE);
        data->device_->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
        data->device_->SetRenderState(D3DRS_STENCILENABLE, FALSE);
        data->device_->SetRenderState(D3DRS_CLIPPING, TRUE);
        data->device_->SetRenderState(D3DRS_LIGHTING, FALSE);
        data->device_->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
        data->device_->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        data->device_->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
        data->device_->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
        data->device_->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
        data->device_->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
        data->device_->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
        data->device_->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        data->device_->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        data->device_->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

        {
            const float L = draw_data->DisplayPos.x + 0.5f;
            const float R = draw_data->DisplayPos.x + draw_data->DisplaySize.x + 0.5f;
            const float T = draw_data->DisplayPos.y + 0.5f;
            const float B = draw_data->DisplayPos.y + draw_data->DisplaySize.y + 0.5f;

            // clang-format off
            D3DMATRIX mat_identity = {{{
                1.0f, 0.0f, 0.0f, 0.0f,
                0.0f, 1.0f, 0.0f, 0.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f
            }}};
            D3DMATRIX mat_projection = {{{
                2.0f / (R - L),     0.0f,               0.0f,   0.0f,
                0.0f,               2.0f / (T - B),     0.0f,   0.0f,
                0.0f,               0.0f,               0.5f,   0.0f,
                (L + R) / (L - R),  (T + B) / (B - T),  0.5f,   1.0f
            }}};
            // clang-format on

            data->device_->SetTransform(D3DTS_WORLD, &mat_identity);
            data->device_->SetTransform(D3DTS_VIEW, &mat_identity);
            data->device_->SetTransform(D3DTS_PROJECTION, &mat_projection);
        }
    }

    /**
     * Constructor and Destructor
     */
    imguimgr::imguimgr(void)
    {}
    imguimgr::~imguimgr(void)
    {}

    /**
     * Returns the singleton instance of this class.
     *
     * @return {imguimgr&} The singleton instance of this class.
     */
    imguimgr& imguimgr::instance(void)
    {
        static dravex::imguimgr mgr;
        return mgr;
    }

    /**
     * Returns the current render data context.
     *
     * @return {renderdata_t*} The render data context if valid, nullptr otherwise.
     */
    details::renderdata_t* imguimgr::get_render_data(void)
    {
        return ImGui::GetCurrentContext() ? static_cast<details::renderdata_t*>(ImGui::GetIO().BackendRendererUserData) : nullptr;
    }

    /**
     * Returns the current window and input data context.
     *
     * @return {win32data_t*} The window and input data context if valid, nullptr otherwise.
     */
    details::win32data_t* imguimgr::get_win32_data(void)
    {
        return ImGui::GetCurrentContext() ? static_cast<details::win32data_t*>(ImGui::GetIO().BackendPlatformUserData) : nullptr;
    }

    /**
     * Initializes the ImGui manager, preparing it for use.
     *
     * @param {HWND} hwnd - The main application window handle.
     * @param {IDirect3DDevice9*} device - The Direct3D device object pointer.
     * @return {bool} True on success, false otherwise.
     */
    bool imguimgr::initialize(const HWND hwnd, IDirect3DDevice9* device)
    {
        if (!device)
            return false;

        if (ImGui::GetCurrentContext() != nullptr)
            return true;

        D3DVIEWPORT9 view{};
        if (FAILED(device->GetViewport(&view)))
            return false;

        int64_t freq    = 0;
        int64_t counter = 0;

        ::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&freq));
        ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&counter));

        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGui::GetMainViewport()->PlatformHandleRaw = static_cast<void*>(hwnd);

        auto& io       = ImGui::GetIO();
        io.IniFilename = nullptr;

        const auto rdata = IM_NEW(details::renderdata_t)();
        const auto wdata = IM_NEW(details::win32data_t)();

        rdata->device_ = device;
        rdata->device_->AddRef();

        wdata->hwnd_                    = hwnd;
        wdata->last_mouse_cursor_       = ImGuiMouseCursor_COUNT;
        wdata->ticks_per_second_        = freq;
        wdata->time_                    = counter;
        wdata->want_update_has_gamepad_ = false;

        io.BackendRendererName     = "imgui_dravex_d3d9";
        io.BackendPlatformName     = "imgui_dravex_win32";
        io.BackendRendererUserData = static_cast<void*>(rdata);
        io.BackendPlatformUserData = static_cast<void*>(wdata);

        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        io.ConfigInputTextCursorBlink   = true;
        io.ConfigInputTrickleEventQueue = true;
        //io.ConfigViewportsNoDecoration  = true;
        io.ConfigWindowsResizeFromEdges = true;

        io.DisplaySize.x = static_cast<float>(view.Width);
        io.DisplaySize.y = static_cast<float>(view.Height);

        hook_wndproc = reinterpret_cast<WNDPROC>(::SetWindowLongPtrA(hwnd, GWL_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc)));

        // Initialize the default ImGui style..
        auto& style = ImGui::GetStyle();

        style.WindowPadding            = ImVec2(8.0f, 8.0f);
        style.FramePadding             = ImVec2(4.0f, 3.0f);
        style.CellPadding              = ImVec2(4.0f, 2.0f);
        style.ItemSpacing              = ImVec2(8.0f, 4.0f);
        style.ItemInnerSpacing         = ImVec2(4.0f, 4.0f);
        style.TouchExtraPadding        = ImVec2(0.0f, 0.0f);
        style.IndentSpacing            = 20.0f;
        style.ScrollbarSize            = 14.0f;
        style.GrabMinSize              = 12.0f;
        style.WindowBorderSize         = 1.0f;
        style.ChildBorderSize          = 1.0f;
        style.PopupBorderSize          = 1.0f;
        style.FrameBorderSize          = 1.0f;
        style.TabBorderSize            = 0.0f;
        style.WindowRounding           = 2.0f;
        style.ChildRounding            = 1.0f;
        style.FrameRounding            = 1.0f;
        style.PopupRounding            = 1.0f;
        style.ScrollbarRounding        = 1.0f;
        style.GrabRounding             = 2.0f;
        style.LogSliderDeadzone        = 1.0f;
        style.TabRounding              = 0.0f;
        style.WindowTitleAlign         = ImVec2(0.0f, 0.50f);
        style.WindowMenuButtonPosition = ImGuiDir_Left;
        style.ColorButtonPosition      = ImGuiDir_Right;
        style.ButtonTextAlign          = ImVec2(0.50f, 0.50f);
        style.SelectableTextAlign      = ImVec2(0.0f, 0.0f);
        style.DisplaySafeAreaPadding   = ImVec2(3.0f, 3.0f);

        const auto colors                      = style.Colors;
        colors[ImGuiCol_Text]                  = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.94f, 0.94f, 0.94f, 0.29f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.18f, 0.20f, 0.23f, 0.96f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.22f, 0.24f, 0.27f, 0.96f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.05f, 0.05f, 0.10f, 0.90f);
        colors[ImGuiCol_Border]                = ImVec4(0.05f, 0.05f, 0.10f, 0.80f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.14f, 0.14f, 0.14f, 0.78f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.52f, 0.32f, 0.63f, 0.69f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.52f, 0.32f, 0.63f, 0.50f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.12f, 0.13f, 0.17f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.12f, 0.13f, 0.17f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.52f, 0.32f, 0.63f, 0.69f);
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_CheckMark]             = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_SliderGrab]            = ImVec4(0.52f, 0.32f, 0.63f, 0.69f);
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.52f, 0.32f, 0.63f, 0.78f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_Header]                = ImVec4(0.52f, 0.32f, 0.63f, 0.78f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_Separator]             = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.05f, 0.05f, 0.05f, 0.69f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.05f, 0.05f, 0.05f, 0.69f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_TabUnfocused]          = ImVec4(0.05f, 0.05f, 0.05f, 0.69f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.05f, 0.05f, 0.05f, 0.69f);
        colors[ImGuiCol_PlotLines]             = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.81f, 0.81f, 0.81f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.05f, 0.05f, 0.05f, 0.69f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.52f, 0.32f, 0.63f, 0.50f);
        colors[ImGuiCol_DragDropTarget]        = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(1.00f, 1.00f, 0.00f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.52f, 0.32f, 0.63f, 1.00f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.00f, 0.00f, 0.00f, 0.60f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.05f, 0.05f, 0.05f, 0.78f);
        colors[ImGuiCol_DockingPreview]        = ImVec4(0.05f, 0.05f, 0.05f, 0.69f);

        // Setup the glyph ranges for the FontAwesome fonts..
        static const ImWchar fa_ranges[]{ICON_MIN_FA, ICON_MAX_FA, 0};
        static const ImWchar fa_ranges_brands[]{ICON_MIN_FAB, ICON_MAX_FAB, 0};

        ImFontConfig cfg_default{};
        ImFontConfig cfg_regular{};
        ImFontConfig cfg_brands{};
        ImFontConfig cfg_solid{};

        const auto font_size = 16.0f;

        // Load the default Agave font..
        cfg_default.MergeMode  = false;
        cfg_default.PixelSnapH = true;
        io.Fonts->AddFontFromFileTTF("fonts/agave-regular.ttf", font_size, &cfg_default);

        // Attach the regular FontAwesome glyphs..
        cfg_regular.MergeMode         = true;
        cfg_regular.PixelSnapH        = true;
        cfg_regular.GlyphOffset       = ImVec2(0.0f, 1.0f);
        cfg_regular.GlyphExtraSpacing = ImVec2(4.0f, 0.0f);
        io.Fonts->AddFontFromFileTTF("fonts/fa-regular-400.ttf", font_size, &cfg_regular, fa_ranges);

        // Attach the solid FontAwesome glyphs..
        cfg_solid.MergeMode         = true;
        cfg_solid.PixelSnapH        = true;
        cfg_solid.GlyphOffset       = ImVec2(0.0f, 1.0f);
        cfg_solid.GlyphExtraSpacing = ImVec2(4.0f, 0.0f);
        io.Fonts->AddFontFromFileTTF("fonts/fa-solid-900.ttf", font_size, &cfg_solid, fa_ranges);

        // Attach the brand FontAwesome glyphs..
        cfg_brands.MergeMode         = true;
        cfg_brands.PixelSnapH        = true;
        cfg_brands.GlyphOffset       = ImVec2(0.0f, 1.0f);
        cfg_brands.GlyphExtraSpacing = ImVec2(4.0f, 0.0f);
        io.Fonts->AddFontFromFileTTF("fonts/fa-brands-400.ttf", font_size, &cfg_brands, fa_ranges_brands);

        return true;
    }

    /**
     * Releases the ImGui manager, cleaning up its resources.
     */
    void imguimgr::release(void)
    {
        auto& io   = ImGui::GetIO();
        auto rdata = this->get_render_data();
        auto wdata = this->get_win32_data();

        this->prereset();

        SAFE_RELEASE(rdata->device_);

        io.BackendRendererName     = nullptr;
        io.BackendRendererUserData = nullptr;
        io.BackendPlatformName     = nullptr;
        io.BackendPlatformUserData = nullptr;

        IM_DELETE(rdata);
        IM_DELETE(wdata);

        ImGui::DestroyContext();
    }

    /**
     * Direct3D callback invoked when the device is about to reset.
     */
    void imguimgr::prereset(void)
    {
        const auto data = this->get_render_data();
        if (!data || !data->device_)
            return;

        SAFE_RELEASE(data->vtx_buffer_);
        SAFE_RELEASE(data->idx_buffer_);
        SAFE_RELEASE(data->fnt_texture_);

        ImGui::GetIO().Fonts->SetTexID(nullptr);
    }

    /**
     * Direct3D callback invoked when the device has finished resetting.
     */
    void imguimgr::postreset(void)
    {
        const auto data = this->get_render_data();
        if (!data || !data->device_)
            return;

        this->create_font_texture();
    }

    /**
     * Begins the new ImGui frame for rendering.
     */
    void imguimgr::beginscene(void)
    {
        if (const auto rdata = this->get_render_data(); rdata != nullptr)
        {
            if (!rdata->fnt_texture_)
                this->postreset();
        }

        if (const auto wdata = this->get_win32_data(); wdata != nullptr)
        {
            auto& io = ImGui::GetIO();

            RECT rect{};
            ::GetClientRect(wdata->hwnd_, &rect);

            int64_t counter = 0;
            ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&counter));

            io.DisplaySize = ImVec2(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));
            io.DeltaTime   = static_cast<float>(counter - wdata->time_) / wdata->ticks_per_second_;
            wdata->time_   = counter;

            update_mouse_data();
            process_key_events();

            const auto cursor = io.MouseDrawCursor ? ImGuiMouseCursor_None : ImGui::GetMouseCursor();
            if (wdata->last_mouse_cursor_ != cursor)
            {
                wdata->last_mouse_cursor_ = cursor;
                update_mouse_cursor();
            }
        }

        ImGui::NewFrame();
    }

    /**
     * Ends the current ImGui frame and renders it to the screen.
     */
    void imguimgr::endscene(void)
    {
        ImGui::EndFrame();
        ImGui::Render();
    }

    /**
     * Creates the base font texture used with ImGui.
     */
    void imguimgr::create_font_texture(void)
    {
        const auto data = this->get_render_data();
        auto& io        = ImGui::GetIO();

        SAFE_RELEASE(data->fnt_texture_);

        uint8_t* pixels = nullptr;
        int32_t width   = 0;
        int32_t height  = 0;
        int32_t bpp     = 0;

        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bpp);

        if (FAILED(data->device_->CreateTexture(width, height, 1, D3DUSAGE_DYNAMIC, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &data->fnt_texture_, nullptr)))
            return;

        D3DLOCKED_RECT lrect{};
        if (FAILED(data->fnt_texture_->LockRect(0, &lrect, nullptr, 0)))
            return;

        for (int32_t y = 0; y < height; y++)
            std::memcpy(static_cast<uint8_t*>(lrect.pBits) + static_cast<std::size_t>(lrect.Pitch) * y, pixels + static_cast<std::size_t>(width) * bpp * y, static_cast<std::size_t>(width) * bpp);

        data->fnt_texture_->UnlockRect(0);

        io.Fonts->SetTexID(static_cast<ImTextureID>(data->fnt_texture_));
    }

    /**
     * Renders the current vertex cache to the screen.
     */
    void imguimgr::render(void)
    {
        const auto draw_data = ImGui::GetDrawData();
        const auto data      = this->get_render_data();

        if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
            return;

        if (!data->vtx_buffer_ || data->vtx_buffer_size_ < draw_data->TotalVtxCount)
        {
            SAFE_RELEASE(data->vtx_buffer_);

            data->vtx_buffer_size_ = draw_data->TotalVtxCount + 5000;

            if (FAILED(data->device_->CreateVertexBuffer(data->vtx_buffer_size_ * sizeof(details::vertex_t), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, D3DFVF_IMGUIVERTEX, D3DPOOL_DEFAULT, &data->vtx_buffer_, nullptr)))
                return;
        }

        if (!data->idx_buffer_ || data->idx_buffer_size_ < draw_data->TotalIdxCount)
        {
            SAFE_RELEASE(data->idx_buffer_);

            data->idx_buffer_size_ = draw_data->TotalIdxCount + 10000;

            if (FAILED(data->device_->CreateIndexBuffer(data->idx_buffer_size_ * sizeof(ImDrawIdx), D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY, sizeof(ImDrawIdx) == 2 ? D3DFMT_INDEX16 : D3DFMT_INDEX32, D3DPOOL_DEFAULT, &data->idx_buffer_, nullptr)))
                return;
        }

        IDirect3DStateBlock9* state_block = nullptr;
        if (FAILED(data->device_->CreateStateBlock(D3DSBT_ALL, &state_block)))
            return;

        if (FAILED(state_block->Capture()))
        {
            state_block->Release();
            return;
        }

        D3DMATRIX world{};
        D3DMATRIX view{};
        D3DMATRIX projection{};

        data->device_->GetTransform(D3DTS_WORLD, &world);
        data->device_->GetTransform(D3DTS_VIEW, &view);
        data->device_->GetTransform(D3DTS_PROJECTION, &projection);

        details::vertex_t* vtx = nullptr;
        ImDrawIdx* idx         = nullptr;

        if (data->vtx_buffer_->Lock(0, static_cast<uint32_t>(draw_data->TotalVtxCount * sizeof(details::vertex_t)), reinterpret_cast<void**>(&vtx), D3DLOCK_DISCARD))
        {
            state_block->Release();
            return;
        }

        if (data->idx_buffer_->Lock(0, static_cast<uint32_t>(draw_data->TotalIdxCount * sizeof(ImDrawIdx)), reinterpret_cast<void**>(&idx), D3DLOCK_DISCARD))
        {
            data->vtx_buffer_->Unlock();
            state_block->Release();
            return;
        }

        for (int32_t n = 0; n < draw_data->CmdListsCount; n++)
        {
            auto cmd_lst = draw_data->CmdLists[n];
            auto vtx_src = cmd_lst->VtxBuffer.Data;

            for (int32_t i = 0; i < cmd_lst->VtxBuffer.Size; i++)
            {
                vtx->pos_[0] = vtx_src->pos.x;
                vtx->pos_[1] = vtx_src->pos.y;
                vtx->pos_[2] = 0.0f;
                vtx->col_    = IMGUI_COL_TO_ARGB(vtx_src->col);
                vtx->tuv_[0] = vtx_src->uv.x;
                vtx->tuv_[1] = vtx_src->uv.y;
                vtx++;
                vtx_src++;
            }

            std::memcpy(idx, cmd_lst->IdxBuffer.Data, cmd_lst->IdxBuffer.Size * sizeof(ImDrawIdx));
            idx += cmd_lst->IdxBuffer.Size;
        }

        data->vtx_buffer_->Unlock();
        data->idx_buffer_->Unlock();

        data->device_->SetStreamSource(0, data->vtx_buffer_, 0, sizeof(details::vertex_t));
        data->device_->SetIndices(data->idx_buffer_);
        data->device_->SetFVF(D3DFVF_IMGUIVERTEX);

        setup_render_state(draw_data);

        int32_t vtx_offset = 0;
        int32_t idx_offset = 0;

        ImVec2 clip = draw_data->DisplayPos;
        for (int32_t n = 0; n < draw_data->CmdListsCount; n++)
        {
            const auto cmd_lst = draw_data->CmdLists[n];
            for (int32_t cmd_i = 0; cmd_i < cmd_lst->CmdBuffer.Size; cmd_i++)
            {
                const auto pcmd = &cmd_lst->CmdBuffer[cmd_i];
                if (pcmd->UserCallback != nullptr)
                {
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState)
                        setup_render_state(draw_data);
                    else
                        pcmd->UserCallback(cmd_lst, pcmd);
                }
                else
                {
                    ImVec2 clip_min(pcmd->ClipRect.x - clip.x, pcmd->ClipRect.y - clip.y);
                    ImVec2 clip_max(pcmd->ClipRect.z - clip.x, pcmd->ClipRect.w - clip.y);

                    if (clip_max.x <= clip_min.x || clip_max.y <= clip_min.y)
                        continue;

                    const RECT rect{static_cast<LONG>(clip_min.x), static_cast<LONG>(clip_min.y), static_cast<LONG>(clip_max.x), static_cast<LONG>(clip_max.y)};

                    data->device_->SetTexture(0, static_cast<IDirect3DTexture9*>(pcmd->GetTexID()));
                    data->device_->SetScissorRect(&rect);
                    data->device_->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, pcmd->VtxOffset + vtx_offset, 0, static_cast<uint32_t>(cmd_lst->VtxBuffer.Size), pcmd->IdxOffset + idx_offset, pcmd->ElemCount / 3);
                }
            }

            idx_offset += cmd_lst->IdxBuffer.Size;
            vtx_offset += cmd_lst->VtxBuffer.Size;
        }

        data->device_->SetTransform(D3DTS_WORLD, &world);
        data->device_->SetTransform(D3DTS_VIEW, &view);
        data->device_->SetTransform(D3DTS_PROJECTION, &projection);

        state_block->Apply();
        state_block->Release();
    }

} // namespace dravex
