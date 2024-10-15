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


enum
{
	ID_BTN_OPEN = 801,
	ID_BTN_OPENMESSAGE,
	ID_BTN_SAVE,
	ID_BTN_SAVEMESSAGE,
	ID_BTN_ENCODE,
	ID_BTN_DECODE,
	ID_BTN_CLEAR,
};

static HWND hLog, hBtnClear, hComboMethod, hStats;
static HFONT hFont, hFontMono;


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
	wcex.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wcex.hInstance = hinstance;
	wcex.lpszClassName = wndClass;
	// Load the menu from resources (avoids clogging this file).
	wcex.lpszMenuName = MAKEINTRESOURCE(IDR_MENU_MAINWINDOW);
	wcex.lpfnWndProc = WindowProc;

	RegisterClassEx(&wcex);

	// Create.
	m_hwnd = CreateWindow(wndClass, title, WS_OVERLAPPEDWINDOW, 20, 20, 800, 600, NULL, 0, 0, 0);
	SetWindowLongPtr(m_hwnd, GWLP_USERDATA, (LONG_PTR) this);

	// Register keyboard shortcuts.
	RegisterHotKey(m_hwnd, ID_FILE_OPENIMAGE, MOD_CONTROL, 'O');
	RegisterHotKey(m_hwnd, ID_FILE_SAVEIMAGE, MOD_CONTROL, 'S');

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

void Window::repaintImages()
{
	InvalidateRect(m_hwnd, 0, true);
}


void Window::clearLog()
{
	SetWindowText(hLog, "");
}

void Window::appendLogLine(const char* msg)
{
	int len = SendMessage(hLog, WM_GETTEXTLENGTH, 0, 0);

	char lineFeed[] = "\r\n";
	SendMessage(hLog, EM_SETSEL, len, len);
	SendMessage(hLog, EM_REPLACESEL, false, (LPARAM)msg);
	SendMessage(hLog, EM_REPLACESEL, false, (LPARAM)lineFeed);
}


static BOOL CALLBACK setFont(HWND hwnd, LPARAM lparam)
{
	SendMessage(hwnd, WM_SETFONT, (WPARAM)lparam, 0);
	return true;
}
static void create(HWND hwnd)
{
	// Log panel.
	hLog = CreateWindow("EDIT", "", WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_BORDER, 10, 220, 180, 200, hwnd, 0, 0, 0);
	SendMessage(hLog, EM_LIMITTEXT, 1*1024*1024, 0);

	hBtnClear = CreateWindow("BUTTON", "Clear", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 10, 130, 80, 24, hwnd, (HMENU)ID_BTN_CLEAR, 0, 0);

	// Action buttons.
	CreateWindow("BUTTON", "1. Load image and data", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX, 5,   10,  190, 60, hwnd, 0, 0, 0);
	CreateWindow("BUTTON", "Open Image",             WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,       15,  35,  80,  24, hwnd, (HMENU)ID_BTN_OPEN, 0, 0);
	CreateWindow("BUTTON", "Open File",              WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,       105, 35,  80,  24, hwnd, (HMENU)ID_BTN_OPENMESSAGE, 0, 0);

	CreateWindow("BUTTON", "2. Process hidden data", WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX, 5,   80,  190, 90, hwnd, 0, 0, 0);
	hComboMethod = CreateWindow("COMBOBOX", "",      WS_CHILD | WS_VISIBLE | WS_OVERLAPPED | CBS_DROPDOWN | CBS_HASSTRINGS, 15, 105, 170, 200, hwnd, 0, 0, 0);
	CreateWindow("BUTTON", "Encode",                 WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,       15,  135, 80,  24, hwnd, (HMENU)ID_BTN_ENCODE, 0, 0);
	CreateWindow("BUTTON", "Decode",                 WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,       105, 135, 80,  24, hwnd, (HMENU)ID_BTN_DECODE, 0, 0);

	CreateWindow("BUTTON", "3. Save results",        WS_CHILD | WS_VISIBLE | WS_GROUP | BS_GROUPBOX, 5,   180, 190, 60, hwnd, 0, 0, 0);
	CreateWindow("BUTTON", "Save Image",             WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,       15,  205, 80,  24, hwnd, (HMENU)ID_BTN_SAVE, 0, 0);
	CreateWindow("BUTTON", "Save File",              WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,       105, 205, 80,  24, hwnd, (HMENU)ID_BTN_SAVEMESSAGE, 0, 0);

	// Image & data stats.
	hStats = CreateWindow("STATIC", "No loaded image.", WS_CHILD | WS_VISIBLE, 10, 250, 180, 200, hwnd, 0, 0, 0);

	// Set steganography methods.
	for (int i = 0; i < (int) EncodeMethod::NUM_METHODS; i++) {
		SendMessage(hComboMethod, CB_ADDSTRING, 0, (LPARAM) Application::getEncodeMethodString((EncodeMethod) i));
	}
	SendMessage(hComboMethod, CB_SETCURSEL, 0, 0);

	// Set font globally.
	hFont = CreateFont(16, 0, 0, 0, FW_REGULAR, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Segoe UI");
	SendMessage(hwnd, WM_SETFONT, (WPARAM)hFont, 0);
	EnumChildWindows(hwnd, &setFont, (LPARAM)hFont);

	// Set mono font.
	hFontMono = CreateFont(14, 0, 0, 0, FW_REGULAR, false, false, false, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Consolas");
	SendMessage(hLog, WM_SETFONT, (WPARAM)hFontMono, 0);
}

static bool processMenuCommand(HWND hwnd, int code)
{
	int selectedMethod;

	switch (code)
	{
	case ID_FILE_OPENIMAGE:
		Application::openImage();
		break;
	case ID_FILE_SAVEIMAGE:
		Application::saveImage();
		break;
	case ID_FILE_EXIT:
		DestroyWindow(hwnd);
		break;

	case ID_BTN_OPEN:
		Application::openImage();
		break;
	case ID_BTN_OPENMESSAGE:
		Application::openMessage();
		break;
	case ID_BTN_SAVE:
		Application::saveImage();
		break;
	case ID_BTN_SAVEMESSAGE:
		Application::saveMessage();
		break;
	case ID_BTN_ENCODE:
		selectedMethod = SendMessage(hComboMethod, CB_GETCURSEL, 0, 0);
		Application::encode((EncodeMethod) selectedMethod);
		Window::getInstance().repaintImages();
		break;
	case ID_BTN_DECODE:
		selectedMethod = SendMessage(hComboMethod, CB_GETCURSEL, 0, 0);
		Application::decode((EncodeMethod) selectedMethod);
		break;
	case ID_BTN_CLEAR:
		Window::getInstance().clearLog();
		break;

	default:
		return false;
	}

	return true;
}

static void paint(HWND hwnd, HDC hdc)
{
	RECT rect;
	GetClientRect(hwnd, &rect);

	Gdiplus::Bitmap* srcImage = GdiPlusManager::getInstance().getImage();
	Gdiplus::Bitmap* genImage = GdiPlusManager::getInstance().getGeneratedImage();

	int availableWidth = (rect.right - rect.left) - 200;
	int availableHeight = (rect.bottom - rect.top) - 300 - 24;

	if (srcImage) {
		int width  = availableWidth / 2;
		int height = srcImage->GetHeight() * width / srcImage->GetWidth();
		Gdiplus::Bitmap* resizedImage = GdiPlusManager::getInstance().ResizeImage(srcImage, width, height);

		Gdiplus::Graphics g(hdc);
		g.DrawImage(resizedImage, Gdiplus::RectF(200, 0, width, availableHeight), 0, 0, width, availableHeight, Gdiplus::UnitPixel);

		HGDIOBJ prevFont = SelectObject(hdc, hFont);
		TextOut(hdc, 200, 0, "Original", 8);
		SelectObject(hdc, prevFont);

		delete resizedImage;
	}

	if (genImage) {
		int width  = availableWidth / 2;
		int height = srcImage->GetHeight() * width / srcImage->GetWidth();
		Gdiplus::Bitmap* resizedImage = GdiPlusManager::getInstance().ResizeImage(genImage, width, height);

		Gdiplus::Graphics g(hdc);
		g.DrawImage(resizedImage, Gdiplus::RectF(200 + availableWidth / 2, 0, width, availableHeight), 0, 0, width, availableHeight, Gdiplus::UnitPixel);

		HGDIOBJ prevFont = SelectObject(hdc, hFont);
		TextOut(hdc, 200 + availableWidth / 2, 0, "Stego", 8);
		SelectObject(hdc, prevFont);

		delete resizedImage;
	}
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

	// Resizing.
	case WM_GETMINMAXINFO:
		((MINMAXINFO*) lparam)->ptMinTrackSize.x = 400;
		((MINMAXINFO*) lparam)->ptMinTrackSize.y = 400;
		return 0;
	case WM_SIZE:
		MoveWindow(hLog,      200, HIWORD(lparam) - 300,      LOWORD(lparam) - 200, 300, true);
		MoveWindow(hBtnClear, 200, HIWORD(lparam) - 300 - 24, 80,                   24,  true);
		InvalidateRect(hwnd, 0, true);
		return 0;

	// Keyboard shortcuts and menu.
	case WM_HOTKEY:
	case WM_COMMAND:
		if (processMenuCommand(hwnd, LOWORD(wparam))) {
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
