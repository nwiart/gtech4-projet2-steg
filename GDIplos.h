#pragma once
#include <windows.h>

#include <gdiplus.h>

#pragma comment (lib, "Gdiplus.lib")

class GdiPlusManager {
public:
    GdiPlusManager();
    ~GdiPlusManager();

    bool LoadImageFromFile(const wchar_t* filePath);
    void DrawImage(HDC hdc, int x, int y);

private:
    ULONG_PTR gdiplusToken;
    Gdiplus::Image* loadedImage;
};
