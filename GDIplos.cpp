#include <windows.h>
#include "GdiPlos.h"
#include <commdlg.h>
#include "Logger.h"
#include "Application.h"


GdiPlusManager& GdiPlusManager::getInstance()
{
    static GdiPlusManager instance;
    return instance;
}

GdiPlusManager::GdiPlusManager() : loadedImage(nullptr), generatedImage(nullptr)
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    Application::log("GDI+ started successfully.");
}

GdiPlusManager::~GdiPlusManager()
{
    if (loadedImage) {
        delete loadedImage;
    }
    Gdiplus::GdiplusShutdown(gdiplusToken);
    Application::log("GDI+ shutdown.");
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
        std::string logMessage = "Image loaded successfully from: " + std::string(filePath);
        Application::log(logMessage.c_str());
        return true;
    }
    else {
        std::string logMessage = "Failed to load image from: " + std::string(filePath);
        Application::log(logMessage.c_str());
        return false;
    }

}

void GdiPlusManager::DrawImage(HDC hdc, int x, int y)
{
    if (!loadedImage) {
        return;
    }
    Gdiplus::Graphics graphics(hdc);
    graphics.DrawImage(loadedImage, x, y);
}


Gdiplus::Bitmap* GdiPlusManager::ResizeImage(Gdiplus::Bitmap* bmp, int newWidth, int newHeight)
{
    if (!bmp) {
        return 0;
    }

    Gdiplus::Bitmap* resizedImage = new Gdiplus::Bitmap(newWidth, newHeight, bmp->GetPixelFormat());

    Gdiplus::Graphics graphics(resizedImage);
    graphics.DrawImage(bmp, 0, 0, newWidth, newHeight);

    return resizedImage;
}

void GdiPlusManager::ApplyBlur(int radius)
{
    if (loadedImage) {
        int width = loadedImage->GetWidth();
        int height = loadedImage->GetHeight();

        Gdiplus::Bitmap* blurredImage = new Gdiplus::Bitmap(width, height, loadedImage->GetPixelFormat());

        for (int y = radius; y < height - radius; y++) {
            for (int x = radius; x < width - radius; x++) {
                int r = 0, g = 0, b = 0, count = 0;

                for (int ky = -radius; ky <= radius; ky++) {
                    for (int kx = -radius; kx <= radius; kx++) {
                        Gdiplus::Color color;
                        loadedImage->GetPixel(x + kx, y + ky, &color);

                        r += color.GetRed();
                        g += color.GetGreen();
                        b += color.GetBlue();
                        count++;
                    }
                }

                r /= count;
                g /= count;
                b /= count;

                Gdiplus::Color avgColor(r, g, b);
                blurredImage->SetPixel(x, y, avgColor);
            }
        }

        delete loadedImage;
        loadedImage = blurredImage;

        Application::log(("Applied manual blur with radius: " + std::to_string(radius)).c_str());
    }
}

void GdiPlusManager::ApplySepia()
{
    if (loadedImage) {
        int width = loadedImage->GetWidth();
        int height = loadedImage->GetHeight();

        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                Gdiplus::Color color;
                loadedImage->GetPixel(x, y, &color);

                int r = color.GetRed();
                int g = color.GetGreen();
                int b = color.GetBlue();

                int tr = (int)(0.393 * r + 0.769 * g + 0.189 * b);
                int tg = (int)(0.349 * r + 0.686 * g + 0.168 * b);
                int tb = (int)(0.272 * r + 0.534 * g + 0.131 * b);

                tr = (tr > 255) ? 255 : tr;
                tg = (tg > 255) ? 255 : tg;
                tb = (tb > 255) ? 255 : tb;

                Gdiplus::Color sepiaColor(tr, tg, tb);

                loadedImage->SetPixel(x, y, sepiaColor);
            }
        }

        Application::log("Applied sepia filter to the image.");
    }
}


