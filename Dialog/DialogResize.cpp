#include "DialogResize.h"

#include <Windows.h>
#include "resource.h"

#include "Window.h"

#include "GDIplos.h"


static INT_PTR ResizeProc(HWND, UINT, WPARAM, LPARAM);


void DialogResize::create()
{
	DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_RESIZE), Window::getInstance().getHwnd(), ResizeProc);
}


static INT_PTR ResizeProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	HWND hWidth;
	HWND hHeight;
	int width, height;
	char buf[64];

	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wparam))
		{
		case IDOK:
			hWidth = GetDlgItem(hwnd, IDC_WIDTH);
			hHeight = GetDlgItem(hwnd, IDC_HEIGHT);

			GetWindowText(hWidth, buf, sizeof(buf));
			width = atoi(buf);
			GetWindowText(hHeight, buf, sizeof(buf));
			height = atoi(buf);

			GdiPlusManager::getInstance().ApplyResize(width, height);

		case IDCANCEL:
			EndDialog(hwnd, wparam);
			return TRUE;
		}
		break;
	}

	return FALSE;
}
