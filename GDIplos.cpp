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

std::vector<bool> GdiPlusManager::GetMessageBits(const std::string& message) {
    std::vector<bool> bits;
    for (char c : message) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back((c >> i) & 1);
        }
    }
    return bits;
}


bool EncodeMessage(const wchar_t* inputImagePath, const wchar_t* outputImagePath, const std::string& message) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Load the image
    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(inputImagePath);
    if (!bitmap) {
        //cout << "Failed to load image!" << endl;
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return false;
    }

    std::vector<bool> messageBits = GdiPlusManager::GetMessageBits(message);
    int bitIndex = 0;

    // Iterate over each pixel to encode the message
    for (UINT y = 0; y < bitmap->GetHeight(); y++) {
        for (UINT x = 0; x < bitmap->GetWidth(); x++) {
            if (bitIndex >= messageBits.size()) {
                break; // Exit if the entire message has been encoded
            }

            Gdiplus::Color pixelColor;
            bitmap->GetPixel(x, y, &pixelColor);

            // Modify the LSB of the pixel's RGB values
            BYTE r = (pixelColor.GetR() & ~1) | messageBits[bitIndex++];
            BYTE g = (pixelColor.GetG() & ~1) | (bitIndex < messageBits.size() ? messageBits[bitIndex++] : 0);
            BYTE b = (pixelColor.GetB() & ~1) | (bitIndex < messageBits.size() ? messageBits[bitIndex++] : 0);

            Gdiplus::Color newColor(pixelColor.GetA(), r, g, b);
            bitmap->SetPixel(x, y, newColor);
        }
    }
    // Cleanup
    delete bitmap;
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return true;
}