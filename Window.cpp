#include "Window.h"

#include <Windows.h>

#include "resource.h"


static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);


Window::Window()
	: m_hwnd(0)
{

}

void Window::init(const char* title)
{
	const char* wndClass = "MainWindowClass";

	HINSTANCE hinstance = GetModuleHandle(0);

	// Create the main window's class.
	WNDCLASSEX wcex; ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.hInstance = hinstance;
	wcex.lpszClassName = wndClass;
	// Load the menu from resources (avoids clogging this file).
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_MAINWINDOW);
	wcex.lpfnWndProc = WindowProc;

	RegisterClassEx(&wcex);

	// Create and show automatically.
	HWND hwnd = CreateWindow(wndClass, title, WS_OVERLAPPEDWINDOW, 20, 20, 800, 600, NULL, 0, 0, 0);
	ShowWindow(hwnd, SW_SHOW);
}

void Window::run()
{
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
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

	case WM_COMMAND:
		switch (LOWORD(wparam))
		{
		case ID_FILE_EXIT:
			DestroyWindow(hwnd);
			return 0;
		}
		break;

	default:
		break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}