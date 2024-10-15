#pragma once
#include <windows.h>

#include <gdiplus.h>
#include <vector>
#include <string>

#pragma comment (lib, "Gdiplus.lib")

class GdiPlusManager {
public:

    static GdiPlusManager& getInstance();

public:
    GdiPlusManager();
    ~GdiPlusManager();

    bool LoadImageFromFile(const char* filePath);
    void DrawImage(HDC hdc, int x, int y);

    inline Gdiplus::Bitmap* getImage() const { return loadedImage; }

    inline Gdiplus::Bitmap* getGeneratedImage() const { return generatedImage; }
    inline void setGeneratedImage(Gdiplus::Bitmap* i) { generatedImage = i; }

    void ApplyBlur(int radius);
    void ResizeImage(int newWidth, int newHeight);
    void ApplySepia();

private:
    ULONG_PTR gdiplusToken;
    Gdiplus::Bitmap* loadedImage;
    Gdiplus::Bitmap* generatedImage;
};
