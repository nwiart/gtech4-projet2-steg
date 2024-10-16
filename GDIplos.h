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

    inline Gdiplus::Bitmap* getImage() const { return loadedImage; }

    inline Gdiplus::Bitmap* getGeneratedImage() const { return generatedImage; }
    void setGeneratedImage(Gdiplus::Bitmap* i);

    Gdiplus::Bitmap* ResizeImage(Gdiplus::Bitmap* bmp, int newWidth, int newHeight);
    void ApplyResize(int newWidth, int newHeight);
    void ApplyBlur(int radius);
    void ApplySepia();

private:
    ULONG_PTR gdiplusToken;
    Gdiplus::Bitmap* loadedImage;
    Gdiplus::Bitmap* generatedImage;
};
