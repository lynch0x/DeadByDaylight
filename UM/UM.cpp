#include <Windows.h>

#include <d3d11.h>
#include "external/imgui.h"
#include "external/imgui_impl_dx11.h"
#include "external/imgui_impl_win32.h"
#include "dbd.h"
#include "offsets.h"
#include "structs.h"
#include "engine.h"
#include "shared.h"
#include <Uxtheme.h>
#include <dwmapi.h>
#include "cheat.h"
#include <thread>
//#include "external/kdmapper.hpp"
//#include "external/intel_driver.hpp"
//#include "driver.h"
#include "external/fonts.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param) {
	if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param)) {
		return 0L;
	}
  
	if (message == WM_DESTROY) {
		PostQuitMessage(0);
		return 0L;
	}
	return DefWindowProc(window, message, w_param, l_param);
}
int LoadDriver()
{
    const char* driverPath = "CorMem.sys";
    const char* serviceName = "CORMEM";
    char fullPath[MAX_PATH];

    GetFullPathNameA(
        driverPath,
        MAX_PATH,
        fullPath,
        nullptr
    );
    // Open Service Control Manager
    SC_HANDLE scm = OpenSCManagerA(
        nullptr,
        nullptr,
        SC_MANAGER_ALL_ACCESS
    );

    if (!scm) {

        return 1;
    }

    // Create driver service
    auto service = CreateServiceA(
        scm,
        serviceName,
        serviceName,
        SERVICE_ALL_ACCESS,
        SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START,
        SERVICE_ERROR_NORMAL,
        fullPath, // <- tutaj
        nullptr,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    );
    if (!service) {
        DWORD err = GetLastError();

        // Already exists
        if (err == ERROR_SERVICE_EXISTS) {
            service = OpenServiceA(
                scm,
                serviceName,
                SERVICE_ALL_ACCESS
            );
        }
        else {

            CloseServiceHandle(scm);
            return 1;
        }
    }

    // Start driver
    StartServiceA(service, 0, nullptr);



    CloseServiceHandle(service);
    CloseServiceHandle(scm);
    return 0;
}
INT WINAPI WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show)
{
    if (LoadDriver() != 0)
    {
        MessageBoxA(0, "Could not load CorMem.sys.\nMake sure you have administrator rights and the file is in the same directory as executable!", NULL, MB_ICONERROR | MB_OK);
        return 1;
    }
    dbd = new DeadByDaylight();
  /*  if (dbd->handle == INVALID_HANDLE_VALUE)
    {
        if (!intel_driver::Load()) {
            MessageBoxA(0, "Could not load driver!", "ERROR", MB_ICONERROR);
            return 0;
        }
        if (!kdmapper::MapDriver(driverRaw)) {
            intel_driver::Unload();
            MessageBoxA(0, "Failed to map driver!", "ERROR", MB_ICONERROR);
            return 0;
        }
        intel_driver::Unload();
    }*/

    screenWidth = GetSystemMetrics(SM_CXSCREEN);
    screenHeight = GetSystemMetrics(SM_CYSCREEN);

    std::thread(CheatThread).detach();

    WNDCLASSEXW wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = instance;
    wc.lpszClassName = L"KLASS";

    RegisterClassExW(&wc);

    const HWND window = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
        wc.lpszClassName,
        L"Svnth",
        WS_POPUP,
        0, 0,
        screenWidth,
        screenHeight,
        nullptr, nullptr,
        wc.hInstance,
        nullptr
    );

    SetLayeredWindowAttributes(window, RGB(0, 0, 0), 255, LWA_ALPHA);

    {
        RECT client_area = { 0 };
        GetClientRect(window, &client_area);

        RECT window_area = { 0 };
        GetClientRect(window, &window_area);

        POINT diff = { 0 };
        ClientToScreen(window, &diff);

        MARGINS margins = {
            diff.x,
            diff.y,
            client_area.right,
            client_area.bottom
        };

        DwmExtendFrameIntoClientArea(window, &margins);
    }

    DXGI_SWAP_CHAIN_DESC sd = { 0 };
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 4;   // było 1
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2;
    sd.OutputWindow = window;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    constexpr D3D_FEATURE_LEVEL levels[2] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0
    };

    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* device_context = nullptr;
    IDXGISwapChain* swap_chain = nullptr;
    ID3D11RenderTargetView* render_target_view = nullptr;
    D3D_FEATURE_LEVEL level;

    D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        levels,
        2,
        D3D11_SDK_VERSION,
        &sd,
        &swap_chain,
        &device,
        &level,
        &device_context
    );

    ID3D11Texture2D* back_buffer = nullptr;
    swap_chain->GetBuffer(0, IID_PPV_ARGS(&back_buffer));

    if (!back_buffer)
        return 1;

    device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
    back_buffer->Release();

    ShowWindow(window, cmd_show);
    UpdateWindow(window);

    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    {
        ImGuiIO& io = ImGui::GetIO();

        ImFontConfig cfg;
        cfg.OversampleH = 3;
        cfg.OversampleV = 3;
        cfg.PixelSnapH = false;

        // Unicode: PL + CYRILLIC
        static const ImWchar ranges[] =
        {
            0x0020, 0x00FF, // basic + latin extended (PL)
            0x0400, 0x052F, // cyrillic (RU)
            0,
        };

        io.Fonts->AddFontFromMemoryTTF(
            (void*)NotoRegular,
            sizeof(NotoRegular),
            18.0f,
            &cfg,
            ranges
        );
        ImGuiStyle& style = ImGui::GetStyle();
        style.AntiAliasedLines = true;
        style.AntiAliasedLinesUseTex = true;
        style.AntiAliasedFill = true;
        style.FrameRounding = 6.0f;
        style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.191f, 0.64f,1.f, 1.0f);
        style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
    }

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, device_context);

    while (!shouldQuit)
    {
        MSG msg = { 0 };

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                shouldQuit = true;
                break;
            }
        }

        if (shouldQuit)
            break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        RenderBlindable();
        RenderMenu();
        RenderESP();

        ImGui::Render();

        const float color[4] = { 0.f, 0.f, 0.f, 0.f };

        device_context->OMSetRenderTargets(1, &render_target_view, nullptr);
        device_context->ClearRenderTargetView(render_target_view, color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swap_chain->Present(1, 0);
    }
    dbd->ClearMappings();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    if (swap_chain) swap_chain->Release();
    if (device_context) device_context->Release();
    if (device) device->Release();
    if (render_target_view) render_target_view->Release();

    DestroyWindow(window);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}