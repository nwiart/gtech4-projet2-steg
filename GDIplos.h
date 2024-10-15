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
    static std::vector<bool> GetMessageBits(const std::string& message);
    static Gdiplus::Bitmap* EncodeMessage(const std::string& message);
    static std::string BitsToMessage(const std::vector<bool>& bits);
    static std::string DecodeMessage(Gdiplus::Bitmap* image);

    static int TestMain();

    inline Gdiplus::Bitmap* getImage() const { return loadedImage; }

    inline Gdiplus::Bitmap* getImage() const { return loadedImage; }
    void ApplyBlur(int radius);
    void ResizeImage(int newWidth, int newHeight);
    void ApplySepia();

private:
    ULONG_PTR gdiplusToken;
    Gdiplus::Bitmap* loadedImage;
};
