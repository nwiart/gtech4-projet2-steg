#include <windows.h>
#include "GdiPlos.h"
#include <commdlg.h>

GdiPlusManager* gdiManager = nullptr;

GdiPlusManager::GdiPlusManager() : loadedImage(nullptr)
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
}

GdiPlusManager::~GdiPlusManager()
{
    if (loadedImage) {
        delete loadedImage;
    }
    Gdiplus::GdiplusShutdown(gdiplusToken);
}

bool GdiPlusManager::LoadImageFromFile(const wchar_t* filePath)
{
    if (loadedImage) {
        delete loadedImage;
        loadedImage = nullptr;
    }

    loadedImage = new Gdiplus::Image(filePath);
    return (loadedImage->GetLastStatus() == Gdiplus::Ok);
}

void GdiPlusManager::DrawImage(HDC hdc, int x, int y)
{
    if (loadedImage) {
        Gdiplus::Graphics graphics(hdc);
        graphics.DrawImage(loadedImage, x, y);
    }
}
