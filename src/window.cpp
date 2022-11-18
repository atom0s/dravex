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

#include "window.hpp"
#include "logging.hpp"
#include "../res/dravex.h"

/**
 * Constructor and Destructor
 */
dravex::window::window(void)
    : hwnd_{nullptr}
    , instance_{::GetModuleHandleA(nullptr)}
    , present_params_{}
    , d3d9_{nullptr}
    , d3d9dev_{nullptr}
{}
dravex::window::~window(void)
{}

/**
 * Window message handler. (static)
 *
 * @param {HWND} hwnd - The window handle of the message.
 * @param {UINT} msg - The message being handled.
 * @param {WPARAM} wparam - The message wparam value.
 * @param {LPARAM} lparam - The message lparam value.
 * @return {LRESULT} Message specific return value.
 */
LRESULT WINAPI dravex::window::window_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    dravex::window* window = nullptr;

    if (msg == WM_CREATE)
    {
        // Set the window user data to the window object..
        window = reinterpret_cast<dravex::window*>(reinterpret_cast<LPCREATESTRUCTA>(lparam)->lpCreateParams);
        ::SetWindowLongPtrA(hwnd, GWL_USERDATA, reinterpret_cast<LONG_PTR>(window));
    }
    else
        window = reinterpret_cast<dravex::window*>(::GetWindowLongA(hwnd, GWL_USERDATA));

    return window == nullptr
               ? ::DefWindowProcA(hwnd, msg, wparam, lparam)
               : window->on_message(hwnd, msg, wparam, lparam);
}

/**
 * Window message handler. (static)
 *
 * @param {HWND} hwnd - The window handle of the message.
 * @param {UINT} msg - The message being handled.
 * @param {WPARAM} wparam - The message wparam value.
 * @param {LPARAM} lparam - The message lparam value.
 * @return {LRESULT} Message specific return value.
 */
LRESULT WINAPI dravex::window::on_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    switch (msg)
    {
        case WM_CREATE:
        {
            // Initialize Direct3D..
            if ((this->d3d9_ = ::Direct3DCreate9(D3D_SDK_VERSION)) == nullptr)
            {
                std::cout << "[!] Error: Failed to create Direct3D9." << std::endl;
                return -1;
            }

            // Prepare the present parameters..
            std::memset(&this->present_params_, 0x00, sizeof(D3DPRESENT_PARAMETERS));
            this->present_params_.BackBufferWidth            = 0;
            this->present_params_.BackBufferHeight           = 0;
            this->present_params_.BackBufferFormat           = D3DFMT_UNKNOWN;
            this->present_params_.BackBufferCount            = 1;
            this->present_params_.MultiSampleType            = D3DMULTISAMPLE_TYPE::D3DMULTISAMPLE_NONE;
            this->present_params_.MultiSampleQuality         = 0;
            this->present_params_.SwapEffect                 = D3DSWAPEFFECT::D3DSWAPEFFECT_DISCARD;
            this->present_params_.hDeviceWindow              = hwnd;
            this->present_params_.Windowed                   = TRUE;
            this->present_params_.EnableAutoDepthStencil     = TRUE;
            this->present_params_.AutoDepthStencilFormat     = D3DFMT_D16;
            this->present_params_.Flags                      = 0;
            this->present_params_.FullScreen_RefreshRateInHz = 0;
            this->present_params_.PresentationInterval       = D3DPRESENT_INTERVAL_ONE;

            // Create the Direct3D device..
            if (FAILED(this->d3d9_->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &this->present_params_, &this->d3d9dev_)))
            {
                std::cout << "[!] Error: Failed to create Direct3D9 device." << std::endl;
                return -1;
            }

            break;
        }

        case WM_DESTROY:
        {
            ::PostQuitMessage(0);
            return 0;
        }

        case WM_SIZE:
        {
            if (this->d3d9_ && wparam != SIZE_MINIMIZED)
            {
                this->present_params_.BackBufferWidth  = LOWORD(lparam);
                this->present_params_.BackBufferHeight = HIWORD(lparam);
                this->reset_device();
            }

            break;
        }
    }

    return ::DefWindowProcA(hwnd, msg, wparam, lparam);
}

/**
 * Initializes the window, preparing it for use.
 *
 * @return {bool} True on success, false otherwise.
 */
bool dravex::window::initialize(void)
{
    // Prepare the window class..
    WNDCLASSEXA wndcls{};
    wndcls.cbSize        = sizeof(WNDCLASSEXA);
    wndcls.style         = CS_CLASSDC | CS_HREDRAW | CS_VREDRAW;
    wndcls.lpfnWndProc   = reinterpret_cast<WNDPROC>(window_proc);
    wndcls.cbClsExtra    = 0L;
    wndcls.cbWndExtra    = 0L;
    wndcls.hInstance     = this->instance_;
    wndcls.hIcon         = ::LoadIconA(this->instance_, MAKEINTRESOURCEA(IDI_ICON));
    wndcls.hCursor       = 0L;
    wndcls.hbrBackground = 0;
    wndcls.lpszMenuName  = 0;
    wndcls.lpszClassName = "cls_wnddravex";
    wndcls.hIconSm       = ::LoadIconA(this->instance_, MAKEINTRESOURCEA(IDI_ICON));

    // Register the window class..
    if (!::RegisterClassExA(&wndcls))
    {
        std::cout << "[!] Error: Failed to register window class." << std::endl;
        return false;
    }

    // Create the window..
    this->hwnd_ = ::CreateWindowExA(0, "cls_wnddravex", "Dungeon Runners Asset Viewer & Extractor - by atom0s", WS_OVERLAPPEDWINDOW, 100, 100, 1600, 900, nullptr, nullptr, this->instance_, this);
    if (!this->hwnd_)
    {
        std::cout << "[!] Error: Failed to create application window." << std::endl;
        ::UnregisterClassA("cls_wnddravex", ::GetModuleHandleA(nullptr));
        return false;
    }

    // Show the window..
    ::ShowWindow(this->hwnd_, SW_SHOWDEFAULT);
    ::UpdateWindow(this->hwnd_);

    return true;
}

/**
 * Releases the window resources.
 */
void dravex::window::release(void)
{
    SAFE_RELEASE(this->d3d9dev_);
    SAFE_RELEASE(this->d3d9_);

    if (this->hwnd_)
        ::DestroyWindow(this->hwnd_);
    ::UnregisterClassA("cls_wnd_dravex", this->instance_);

    this->hwnd_     = nullptr;
    this->instance_ = nullptr;
}

/**
 * Runs the window until it has been terminated.
 */
void dravex::window::run()
{
    if (this->d3d9_ == nullptr || this->d3d9dev_ == nullptr)
        return;

    auto running = true;

    while (running)
    {
        // Handle the pending window messages..
        MSG msg{};
        while (::PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessageA(&msg);

            if (msg.message == WM_QUIT)
                running = false;
        }

        if (!running)
            break;

        // Sleep to not kill the CPU..
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1ms);

        // Update the application..
        if (this->on_update_)
            this->on_update_();

        // Prepare the scene states..
        this->d3d9dev_->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        this->d3d9dev_->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        this->d3d9dev_->SetRenderState(D3DRS_ZENABLE, FALSE);

        // Clear the buffer..
        const auto color = D3DCOLOR_ARGB(255, 30, 30, 30);
        this->d3d9dev_->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, color, 1.0f, 0);

        // Render the scene..
        if (SUCCEEDED(this->d3d9dev_->BeginScene()))
        {
            if (this->on_render_)
                this->on_render_();

            this->d3d9dev_->EndScene();
        }

        // Present the scene..
        const auto res = this->d3d9dev_->Present(nullptr, nullptr, nullptr, nullptr);
        if (res == D3DERR_DEVICELOST || res == D3DERR_DEVICENOTRESET)
        {
            if (!this->reset_device())
            {
                running = false;
                break;
            }
        }
    }
}

/**
 * Resets the Direct3D device and resources.
 *
 * @return {bool} True on success, false otherwise.
 */
bool dravex::window::reset_device(void)
{
    dravex::logging::instance().log(dravex::loglevel::debug, "[graphics] graphics device was reset; reinitializing objects..");

    // Call reset callback (pre)..
    if (this->on_reset_)
    {
        if (!this->on_reset_(true))
            return false;
    }

    // Reset the graphics device..
    if (FAILED(this->d3d9dev_->Reset(&this->present_params_)))
        return false;

    // Call reset callback (post)..
    if (this->on_reset_)
    {
        if (!this->on_reset_(false))
            return false;
    }

    return true;
}

/**
 * Sets the window render callback.
 *
 * @param {std::function} callback - The function to invoke when the window is rendering the Direct3D scene.
 */
void dravex::window::set_render_callback(std::function<void(void)> callback)
{
    this->on_render_ = callback;
}

/**
 * Sets the window reset callback.
 *
 * @param {std::function} callback - The function to invoke when the window is resetting the Direct3D device..
 */
void dravex::window::set_reset_callback(std::function<bool(bool)> callback)
{
    this->on_reset_ = callback;
}

/**
 * Sets the window update callback.
 *
 * @param {std::function} callback - The function to invoke when window is updating, per-frame.
 */
void dravex::window::set_update_callback(std::function<void(void)> callback)
{
    this->on_update_ = callback;
}
