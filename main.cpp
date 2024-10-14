#include <Windows.h>

#define SHOW_CONSOLE


static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);


#ifdef SHOW_CONSOLE
int main()
#else
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
#endif
{
	const char* wndClass = "MyWindowClass";

	WNDCLASSEX wcex; ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hbrBackground = (HBRUSH) COLOR_WINDOW;
	wcex.hInstance = GetModuleHandle(0);
	wcex.lpszClassName = wndClass;
	wcex.lpfnWndProc = WindowProc;

	RegisterClassEx(&wcex);

	HWND hwnd = CreateWindow(wndClass, "Title", WS_OVERLAPPEDWINDOW, 20, 20, 800, 600, NULL, 0, 0, 0);
	ShowWindow(hwnd, SW_SHOW);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}


static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}
