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
    static bool EncodeMessage(const wchar_t* inputImagePath, const wchar_t* outputImagePath, const std::string& message);
    static std::string BitsToMessage(const std::vector<bool>& bits);
    static std::string DecodeMessage(const wchar_t* imagePath);

    static int TestMain();

    inline Gdiplus::Bitmap* getImage() const { return loadedImage; }

private:
    ULONG_PTR gdiplusToken;
    Gdiplus::Bitmap* loadedImage;
};
