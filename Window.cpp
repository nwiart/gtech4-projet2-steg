#include "Window.h"

#include "Application.h"
#include "GDIplos.h"

#include <Windows.h>

#include "resource.h"


// Visual styles.
#pragma comment(linker,"\"/manifestdependency:type='win32' \
	name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


static LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);


Window& Window::getInstance()
{
	static Window instance;
	return instance;
}


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

	// Create.
	m_hwnd = CreateWindow(wndClass, title, WS_OVERLAPPEDWINDOW, 20, 20, 800, 600, NULL, 0, 0, 0);
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR) this);

	// Show automatically.
	ShowWindow(m_hwnd, SW_SHOW);
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


static BOOL CALLBACK setFont(HWND hwnd, LPARAM lparam)
{
	SendMessage(hwnd, WM_SETFONT, (WPARAM)lparam, 0);
	return true;
}
static void create(HWND hwnd)
{
	HWND htruc = CreateWindow("STATIC", "This image is\nbig lmao", WS_CHILD | WS_VISIBLE, 10, 10, 180, 180, hwnd, 0, GetModuleHandle(0), 0);

	HWND hwndButton = CreateWindow("BUTTON", "Encode", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 10, 100, 80, 20, hwnd, 0, 0, 0);

	// Set font globally.
	HFONT hFont = CreateFont(16, 0, 0, 0, FW_REGULAR, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
	SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, 0);
	EnumChildWindows(hwnd, &setFont, (LPARAM)hFont);
}

static void paint(HWND hwnd, HDC hdc)
{
	GdiPlusManager::getInstance().DrawImage(hdc, 200, 0);
}


static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	Window* win = (Window*) GetWindowLongPtr(hwnd, GWLP_USERDATA);

	switch (msg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_CREATE:
		create(hwnd);
		break;

	case WM_COMMAND:
		switch (LOWORD(wparam))
		{
		case ID_FILE_OPENIMAGE:
			Application::openImage();
			InvalidateRect(hwnd, 0, false);
			return 0;
		case ID_FILE_SAVEIMAGE:
			Application::saveImage();
			return 0;
		case ID_FILE_EXIT:
			DestroyWindow(hwnd);
			return 0;
		}
		break;

	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		paint(hwnd, hdc);
		EndPaint(hwnd, &ps);
		break;

	default:
		break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}
