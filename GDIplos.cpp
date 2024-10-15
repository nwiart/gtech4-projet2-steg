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

        Logger::logMessage("Applied manual blur with radius: " + std::to_string(radius));
    }
}

void GdiPlusManager::ResizeImage(int newWidth, int newHeight)
{
    if (loadedImage) {
        Gdiplus::Bitmap* resizedImage = new Gdiplus::Bitmap(newWidth, newHeight, loadedImage->GetPixelFormat());

        Gdiplus::Graphics graphics(resizedImage);
        graphics.DrawImage(loadedImage, 0, 0, newWidth, newHeight);

        delete loadedImage;
        loadedImage = resizedImage;

        Logger::logMessage("Resized image to: " + std::to_string(newWidth) + "x" + std::to_string(newHeight));
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

        Logger::logMessage("Applied sepia filter to the image.");
    }
}


