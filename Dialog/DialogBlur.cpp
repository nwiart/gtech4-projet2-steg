#include "DialogBlur.h"

#include <Windows.h>
#include "resource.h"

#include "Window.h"


static INT_PTR BlurProc(HWND, UINT, WPARAM, LPARAM);


void DialogBlur::create()
{
	DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_BLUR), Window::getInstance().getHwnd(), BlurProc);
}


static INT_PTR BlurProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wparam))
		{
		case IDOK:

		case IDCANCEL:
			EndDialog(hwnd, wparam);
			return TRUE;
		}
		break;

	case WM_NOTIFY:
		
		break;
	}

	return FALSE;
}
