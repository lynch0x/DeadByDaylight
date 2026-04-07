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

//void sendSpaceCommand()
//{
//    INPUT inputs[2] = {};
//
//    inputs[0].type = INPUT_KEYBOARD;
//    inputs[0].ki.wVk = VK_SPACE;
//
//    inputs[1].type = INPUT_KEYBOARD;
//    inputs[1].ki.wVk = VK_SPACE;
//    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;
//
//    SendInput(2, inputs, sizeof(INPUT));
//}
//bool IsProbablyValidAddress(uintptr_t addr)
//{
//	return (addr >= 0x10000 && addr <= 0x00007FFFFFFFFFFF);
//}
//bool IsA(DeadByDaylight& dbd,uintptr_t actor, uintptr_t klass)
//{
//	uintptr_t temp = dbd.Read<uintptr_t>(actor + 0x10);
//		if (temp == klass) {
//			return true;
//		}
//		temp = dbd.Read<uintptr_t>(temp + 0x60);
//		if (temp == klass) {
//			return true;
//		}
//		temp = dbd.Read<uintptr_t>(temp + 0x60);
//		if (temp == klass) {
//			return true;
//		}
//	return false;
//}
//uintptr_t FindSingleActorOfType(DeadByDaylight& dbd, uintptr_t level, uintptr_t klass, unsigned int* index)
//{
//
//	unsigned int actorsCount = dbd.Read<unsigned int>(level + 0x00C0 + 0x8);
//	if (actorsCount > 100 && actorsCount < 3000) {
//		uintptr_t actorsArray = dbd.Read<uintptr_t>(level + 0x00C0);
//		if (index) {
//			if (*index != 0) {
//				uintptr_t actor = dbd.Read<uintptr_t>(actorsArray + (*index * 0x8));
//				if (IsA(dbd, actor, klass)) {
//					return actor;
//				}
//			}
//		}
//		for (unsigned int i = 0; i < actorsCount; i++)
//		{
//
//			uintptr_t actor = dbd.Read<uintptr_t>(actorsArray + i * 0x8);
//
//			if (IsA(dbd, actor, klass)) {
//				if(index)
//					*index = i;
//
//				return actor;
//			}
//		}
//	}
//	return 0;
//}
//
//
//bool IsKillerCarryingMe(DeadByDaylight& dbd, uintptr_t mypawn)
//{
//	uintptr_t carried = dbd.Read<uintptr_t>(killer + 0x1A68);
//	return mypawn == carried;
//}


INT WINAPI WinMain(HINSTANCE instance, HINSTANCE, PSTR, INT cmd_show) 
{
	dbd = new DeadByDaylight();
	if (dbd->handle == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(0, "Make sure you have administrator privileges and the driver is loaded!", "", 0);
		return 0;
	}
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);
	{
		std::thread t(CheatThread);
		t.detach();
	}
	WNDCLASSEXW wc = {0};
	wc.cbSize = sizeof(WNDCLASSEXW);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = window_procedure;
	wc.hInstance = instance;
	wc.lpszClassName = L"KLASS";
	RegisterClassExW(&wc);
	
	const HWND window = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		wc.lpszClassName, L"Svnth", WS_POPUP, 0, 0, screenWidth, screenHeight, 0, 0, wc.hInstance, 0);
	SetLayeredWindowAttributes(window, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);
	{
		RECT client_area={0};
		GetClientRect(window, &client_area);
		RECT window_area={0};
		GetClientRect(window, &window_area);

		POINT diff={0};
		ClientToScreen(window, &diff);

		const MARGINS margins{
			window_area.left + (diff.x - window_area.left),
			window_area.top + (diff.y - window_area.top),
			client_area.right,
			client_area.bottom
		};
		DwmExtendFrameIntoClientArea(window, &margins);

	}
	DXGI_SWAP_CHAIN_DESC sd={0};
	sd.BufferDesc.RefreshRate.Numerator = 60U;
	sd.BufferDesc.RefreshRate.Numerator = 1U;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = 1U;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 2U;
	sd.OutputWindow = window;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	constexpr D3D_FEATURE_LEVEL levels[2]{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_0
	};

	ID3D11Device* device{ 0 };
	ID3D11DeviceContext* device_context{ 0 };
	IDXGISwapChain* swap_chain{ 0 };
	ID3D11RenderTargetView* render_target_view{ 0 };
	D3D_FEATURE_LEVEL level{};
	D3D11CreateDeviceAndSwapChain(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0U, levels, 2U, D3D11_SDK_VERSION, &sd, &swap_chain, &device, &level, &device_context);
	ID3D11Texture2D* back_buffer{ 0 };
	swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));
	if (!back_buffer) {
		return 1;
	}
	device->CreateRenderTargetView(back_buffer, 0, &render_target_view);
	back_buffer->Release();
	ShowWindow(window, cmd_show);
	UpdateWindow(window);

	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(device, device_context);
	while (!shouldQuit) {
		MSG msg = { 0 };
		while (PeekMessage(&msg, 0, 0U, 0U, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT) {
				shouldQuit = true;
			}
		}
		if (shouldQuit)break;

		if (GetAsyncKeyState(VK_INSERT) & 1) {

			showMenu = !showMenu;

			if (showMenu) {
				SetWindowLong(window, GWL_EXSTYLE,
					WS_EX_TOPMOST | WS_EX_LAYERED);
			}
			else {
				SetWindowLong(window, GWL_EXSTYLE,
					WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED);
			}
		}
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		RenderMenu();
		RenderESP();
		ImGui::Render();
		constexpr float color[4]{ 0.f,0.f,0.f,0.f };
		device_context->OMSetRenderTargets(1U, &render_target_view, 0);
		device_context->ClearRenderTargetView(render_target_view, color);

		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		swap_chain->Present(1U, 0U);
	}
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	//ImGui::DestroyContext();
	if (swap_chain)swap_chain->Release();
	if (device_context)device_context->Release();
	if (device)device->Release();
	if (render_target_view)render_target_view->Release();
	DestroyWindow(window);
	UnregisterClassW(wc.lpszClassName, wc.hInstance);
	return 0;

}