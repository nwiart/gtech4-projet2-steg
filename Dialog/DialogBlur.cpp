#include "DialogBlur.h"

#include "Window.h"
#include "GDIplos.h"

#include <Windows.h>
#include <CommCtrl.h>
#include "resource.h"


static INT_PTR BlurProc(HWND, UINT, WPARAM, LPARAM);


void DialogBlur::create()
{
	DialogBox(GetModuleHandle(0), MAKEINTRESOURCE(IDD_BLUR), Window::getInstance().getHwnd(), BlurProc);
}


static INT_PTR BlurProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	HWND hStaticRadius;
	HWND hSliderRadius;
	char buf[32];

	switch (msg)
	{
	case WM_COMMAND:
		switch (LOWORD(wparam))
		{
		case IDOK:
			hSliderRadius = GetDlgItem(hwnd, IDC_BLURRADIUS);
			GdiPlusManager::getInstance().ApplyBlur(SendMessage(hSliderRadius, TBM_GETPOS, 0, 0));

		case IDCANCEL:
			EndDialog(hwnd, wparam);
			return TRUE;
		}
		break;

	case WM_HSCROLL:
		hStaticRadius = GetDlgItem(hwnd, IDC_STATIC_RADIUS);
		hSliderRadius = GetDlgItem(hwnd, IDC_BLURRADIUS);

		itoa(SendMessage(hSliderRadius, TBM_GETPOS, 0, 0), buf, 10);
		strcat(buf, " px");
		SetWindowText(hStaticRadius, buf);
		break;
	}

	return FALSE;
}
