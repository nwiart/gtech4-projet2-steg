#include <windows.h>
#include "GdiPlos.h"
#include <commdlg.h>
#include "Logger.h"


GdiPlusManager& GdiPlusManager::getInstance()
{
    static GdiPlusManager instance;
    return instance;
}

GdiPlusManager::GdiPlusManager() : loadedImage(nullptr)
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    Logger::logMessage("GDI+ started successfully.");
}

GdiPlusManager::~GdiPlusManager()
{
    if (loadedImage) {
        delete loadedImage;
    }
    Gdiplus::GdiplusShutdown(gdiplusToken);
    Logger::logMessage("GDI+ shutdown.");
}

bool GdiPlusManager::LoadImageFromFile(const char* filePath)
{
    wchar_t wpath[MAX_PATH];
    mbstowcs(wpath, filePath, MAX_PATH);

    if (loadedImage) {
        delete loadedImage;
        loadedImage = nullptr;
    }

    loadedImage = new Gdiplus::Bitmap(wpath);

    Gdiplus::Color col;
    loadedImage->GetPixel(0, 0, &col);

    if (loadedImage->GetLastStatus() == Gdiplus::Ok) {
        Logger::logMessage(std::string("Image loaded successfully from: ") + filePath);
        return true;
    }
    else {
        Logger::logMessage(std::string("Failed to load image from: ") + filePath);
        return false;
    }
}

void GdiPlusManager::DrawImage(HDC hdc, int x, int y)
{
    if (loadedImage) {
        Gdiplus::Graphics graphics(hdc);
        graphics.DrawImage(loadedImage, x, y);

        Logger::logMessage("Image drawn at position: (" + std::to_string(x) + ", " + std::to_string(y) + ")");
    }
    else {
        Logger::logMessage("Attempt to draw image failed. No image loaded.");
    }
}
