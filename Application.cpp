#include "Application.h"

#include "Window.h"
#include "GDIplos.h"
#include "Logger.h"

#include "Algos/LSB.h"
#include "Algos/LSBExtended.h"
#include "Algos/MatriceEmbedding.h"

#include <Windows.h>

#include <string>


static BinaryBuffer messageBuffer;
static BinaryBuffer decodedBuffer;


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

	if (!messageBuffer.isEmpty()) {
		free(messageBuffer.getData());
		messageBuffer.clear();
	}

	unsigned char* buf = (unsigned char*) malloc(size);
	fread(buf, size, 1, file);
	messageBuffer = BinaryBuffer(buf, size);

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

void Application::saveMessage()
{
	SaveResult res = _saveMessage();
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
	ofn.lpstrFilter = "PNG Image\0*.PNG\0JPEG Image\0*.JPG;*.JPEG\0";

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

SaveResult Application::_saveMessage()
{
	HWND hwnd = Window::getInstance().getHwnd();
	char path[MAX_PATH];
	path[0] = '\0';

	// No data.
	if (decodedBuffer.isEmpty()) {
		return SaveResult::NOIMAGE;
	}

	// Open dialog for save destination.
	OPENFILENAME ofn; ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = path;
	ofn.nMaxFile = sizeof(path);
	ofn.lpstrFilter = "All\0*.*\0";

	if (!GetSaveFileName(&ofn)) {
		return SaveResult::ABORTED;
	}

	// Save.
	FILE* file = fopen(path, "wb");
	fwrite(decodedBuffer.getData(), decodedBuffer.getSize(), 1, file);
	fclose(file);

	Application::log((std::string("Saved to \"") + (char*)path + "\"").c_str());

	return SaveResult::OK;
}


void Application::encode(EncodeMethod m)
{
	if (messageBuffer.isEmpty()) {
		return;
	}

	uint64_t freq, time0, time1;
	QueryPerformanceFrequency((LARGE_INTEGER*) &freq);
	QueryPerformanceCounter((LARGE_INTEGER*) &time0);

	switch (m)
	{
	case EncodeMethod::LSB:
		LSB::EmbedMessageInImage(messageBuffer);
		break;
	case EncodeMethod::LSB_EXTENDED:
		LSBExtended::EmbedMessageInImage(messageBuffer);
		break;
	case EncodeMethod::MATRIX_EMBEDDING:
		MatriceEmbedding::EmbedMessageInImage("This is a secret message (with matrix embed)!");
		break;
	}

	QueryPerformanceCounter((LARGE_INTEGER*) &time1);

	int duration = ceil(((time1 - time0) / (double)freq) * 1000.0);
	Application::log((std::string("Encoded ") + std::to_string(messageBuffer.getSize()) + " bytes in " + std::to_string(duration) + " ms.").c_str());
}

void Application::decode(EncodeMethod m)
{
	if (decodedBuffer.getData()) {
		free(decodedBuffer.getData());
		decodedBuffer = BinaryBuffer();
	}

	uint64_t freq, time0, time1;
	QueryPerformanceFrequency((LARGE_INTEGER*) &freq);
	QueryPerformanceCounter((LARGE_INTEGER*) &time0);

	switch (m)
	{
	case EncodeMethod::LSB:
		decodedBuffer = LSB::DecodeMessageFromImage(GdiPlusManager::getInstance().getImage());
		break;
	case EncodeMethod::LSB_EXTENDED:
		decodedBuffer = LSBExtended::DecodeMessageFromImage(GdiPlusManager::getInstance().getImage());
		break;
	case EncodeMethod::MATRIX_EMBEDDING:
		MatriceEmbedding::DecodeMessageFromImage(GdiPlusManager::getInstance().getImage());
		break;
	}

	QueryPerformanceCounter((LARGE_INTEGER*) &time1);

	int duration = ceil(((time1 - time0) / (double) freq) * 1000.0);
	Application::log((std::string("Decoded ") + std::to_string(decodedBuffer.getSize()) + " bytes in " + std::to_string(duration) + " ms.").c_str());
}


int Application::getMaximumBytes(EncodeMethod m, int width, int height)
{
	switch (m)
	{
	case EncodeMethod::LSB:
		return (width * height * 3) / 8;
	case EncodeMethod::LSB_EXTENDED:
		return (width * height);
	case EncodeMethod::MATRIX_EMBEDDING:
		return 0; // TODO : implement.
	default:
		return 0;
	}
}

const BinaryBuffer& Application::getSelectedMessage()
{
	return messageBuffer;
}

const BinaryBuffer& Application::getDecodedMessage()
{
	return decodedBuffer;
}


const char* Application::getEncodeMethodString(EncodeMethod r)
{
	switch (r)
	{
	case EncodeMethod::LSB: return "LSB";
	case EncodeMethod::LSB_EXTENDED: return "LSB (Extended)";
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
