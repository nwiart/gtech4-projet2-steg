#pragma once
#include <windows.h>

#include <gdiplus.h>
#include <vector>
#include <string>

#pragma comment (lib, "Gdiplus.lib")

class GdiPlusManager {
public:
    GdiPlusManager();
    ~GdiPlusManager();

    bool LoadImageFromFile(const wchar_t* filePath);
    void DrawImage(HDC hdc, int x, int y);
    static std::vector<bool> GetMessageBits(const std::string& message);
    static bool EncodeMessage(const wchar_t* inputImagePath, const wchar_t* outputImagePath, const std::string& message);
    static std::string BitsToMessage(const std::vector<bool>& bits);
    static std::string DecodeMessage(const wchar_t* imagePath);

    static int TestMain();

private:
    ULONG_PTR gdiplusToken;
    Gdiplus::Image* loadedImage;
};
