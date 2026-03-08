#include "display.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

	const wchar_t CLASS_NAME[] = L"MonitorTrackerClass";

	WNDCLASS wc = {};
	wc.lpfnWndProc = DisplayCtrl::WndProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	HWND hWnd = CreateWindowEx(0, CLASS_NAME, L"MonitorTracker", 0, 0, 0, 0, 0,
		HWND_MESSAGE, NULL, hInstance, NULL);

	if (!hWnd || !DisplayCtrl::Initialize(hWnd)) {
		MessageBox(NULL, L"Failed to register hotkeys or power listener. Another app might be using them.", L"Error", MB_ICONERROR);
		return 1;
	}

	MSG msg = {0};
	// GetMessage will still retrieve thread-level messages
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}