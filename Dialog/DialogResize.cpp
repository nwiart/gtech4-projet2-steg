#include "DialogResize.h"

#include <Windows.h>
#include "resource.h"

#include "Window.h"


static INT_PTR ResizeProc(HWND, UINT, WPARAM, LPARAM);


void DialogResize::create()
{
	DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_RESIZE), Window::getInstance().getHwnd(), ResizeProc);
}


static INT_PTR ResizeProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
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
	}

	return FALSE;
}
