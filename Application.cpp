#include "Application.h"

#include "Window.h"
#include "GDIplos.h"
#include "Logger.h"

#include "Algos/LSB.h"
#include "Algos/MatriceEmbedding.h"

#include <Windows.h>

#include <string>


static char* messageBuffer = 0;


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
	Logger::logMessage(msg);
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

	Window::getInstance().repaintImages();

	Application::log((std::string("Successfully opened \"") + path + "\"").c_str());
}

void Application::openMessage()
{
	Application::log("Opening message file...");

	HWND hwnd = Window::getInstance().getHwnd();
	char path[MAX_PATH];
	path[0] = '\0';

	OPENFILENAME ofn; ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof(path);
	ofn.lpstrFilter = "All\0*.*\0";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (!GetOpenFileName(&ofn)) {
		Application::log("Open aborted.");
		return;
	}

	FILE* file = fopen(path, "rb");
	if (!file) {
		Application::log("Unknown error while opening file.");
		return;
	}

	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fseek(file, 0, SEEK_SET);

	if (messageBuffer) {
		free(messageBuffer);
		messageBuffer = 0;
	}

	messageBuffer = (char*) malloc(size);
	fread(messageBuffer, size, 1, file);

	fclose(file);

	Application::log((std::string("Successfully opened \"") + path + "\"").c_str());
}

void Application::saveImage()
{
	SaveResult res = _saveImage();
	if (res == SaveResult::OK) {
		return;
	}
	if (res == SaveResult::ABORTED) {
		Application::log(Application::getSaveResultString(SaveResult::ABORTED));
		return;
	}

	std::string msg = "Save failed : ";
	msg += Application::getSaveResultString(res);
	Application::log(msg.c_str());
}


SaveResult Application::_saveImage()
{
	HWND hwnd = Window::getInstance().getHwnd();
	char path[MAX_PATH];
	wchar_t wpath[MAX_PATH];
	path[0] = '\0';

	// No open image.
	Gdiplus::Bitmap* image = GdiPlusManager::getInstance().getGeneratedImage();
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
	char* ext = strchr(path, '.');
	if (!ext) {
		return SaveResult::INVALIDEXT;
	}
	if (!getEncoderByExtension(ext + 1, &encoderID)) {
		return SaveResult::INVALIDEXT;
	}
	mbstowcs(wpath, path, MAX_PATH);
	image->Save(wpath, &encoderID);

	Application::log((std::string("Saved to \"") + (char*)path + "\"").c_str());

	return SaveResult::OK;
}


void Application::encode(EncodeMethod m)
{
	char msg[] = "This is a secret message!";

	switch (m)
	{
	case EncodeMethod::LSB:
		LSB::EmbedMessageInImage(BinaryBuffer(msg, strlen(msg) + 1));
		break;
	case EncodeMethod::MATRIX_EMBEDDING:
		MatriceEmbedding::EmbedMessageInImage("This is a secret message (with matrix embed)!");
		break;
	}
}

void Application::decode(EncodeMethod m)
{
	BinaryBuffer b;

	switch (m)
	{
	case EncodeMethod::LSB:
		b = LSB::DecodeMessageFromImage(GdiPlusManager::getInstance().getImage());
		break;
	case EncodeMethod::MATRIX_EMBEDDING:
		MatriceEmbedding::DecodeMessageFromImage(GdiPlusManager::getInstance().getImage());
		break;
	}

	Application::log((const char*) b.getData());
}


const char* Application::getEncodeMethodString(EncodeMethod r)
{
	switch (r)
	{
	case EncodeMethod::LSB: return "LSB";
	case EncodeMethod::MATRIX_EMBEDDING: return "Matrix Embedding";
	}
}


const char* Application::getSaveResultString(SaveResult r)
{
	switch (r)
	{
	case SaveResult::OK: return "";
	case SaveResult::NOIMAGE: return "There is no image to save";
	case SaveResult::ABORTED: return "Save aborted";
	case SaveResult::INVALIDEXT: return "Invalid file name / file extension";
	case SaveResult::FAILED: return "Unknown error";
	}
}
