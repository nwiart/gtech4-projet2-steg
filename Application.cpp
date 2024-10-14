#include "Application.h"

#include "Window.h"
#include "GDIplos.h"

#include <Windows.h>


static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}

	free(pImageCodecInfo);
	return -1;  // Failure
}

static bool getEncoderByExtension(const char* ext, CLSID* id)
{
	if (strcmp(ext, "png") == 0) {
		return GetEncoderClsid(L"image/png", id) != -1;
	}
	else if (strcmp(ext, "jpg") == 0 || strcmp(ext, "jpeg") == 0) {
		return GetEncoderClsid(L"image/jpeg", id) != -1;
	}
	return false;
}


void Application::log(const char* msg)
{
	Window::getInstance().appendLogLine(msg);
}

void Application::openImage()
{
	Application::log("Opening image...");

	HWND hwnd = Window::getInstance().getHwnd();
	char path[MAX_PATH];
	path[0] = '\0';

	OPENFILENAME ofn; ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof(path);
	ofn.lpstrFilter = "Images\0*.BMP;*.PNG;*.JPG;*.JPEG\0";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileName(&ofn)) {
		Application::log("Open aborted.");
		return;
	}

	if (!GdiPlusManager::getInstance().LoadImageFromFile(path)) {
		MessageBox(hwnd, "There was a problem loading the image.", "Oops...", MB_OK);
		Application::log("Unknown error while opening an image.");
		return;
	}

	Application::log("Successfully opened image!");
}


SaveResult Application::saveImage()
{
	HWND hwnd = Window::getInstance().getHwnd();
	char path[MAX_PATH];
	wchar_t wpath[MAX_PATH];
	path[0] = '\0';

	// No open image.
	Gdiplus::Bitmap* image = GdiPlusManager::getInstance().getImage();
	if (!image) {
		return SaveResult::NOIMAGE;
	}

	// Open dialog for save destination.
	OPENFILENAME ofn; ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof(path);
	ofn.lpstrFilter = "Images\0*.BMP;*.PNG;*.JPG;*.JPEG\0";

	if (!GetSaveFileName(&ofn)) {
		return SaveResult::ABORTED;
	}

	// Save.
	CLSID encoderID;
	if (!getEncoderByExtension(strchr(path, '.') + 1, &encoderID)) {
		return SaveResult::INVALIDEXT;
	}
	mbstowcs(wpath, path, MAX_PATH);
	image->Save(wpath, &encoderID);

	return SaveResult::OK;
}
