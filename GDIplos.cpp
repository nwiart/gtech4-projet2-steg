#include <windows.h>
#include "GdiPlos.h"
#include <commdlg.h>
#include "Logger.h"
#include "Application.h"

#include <emmintrin.h>


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

void GdiPlusManager::setGeneratedImage(Gdiplus::Bitmap* i)
{
    if (generatedImage) {
        delete generatedImage;
    }

    generatedImage = i;
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

void GdiPlusManager::ApplyResize(int newWidth, int newHeight)
{
    Gdiplus::Bitmap* bmp = ResizeImage(loadedImage, newWidth, newHeight);
    delete loadedImage;
    loadedImage = bmp;
}

void GdiPlusManager::ApplyBlur(int radius)
{
    if (!loadedImage) {
        return;
    }

    int width = loadedImage->GetWidth();
    int height = loadedImage->GetHeight();

    Gdiplus::Bitmap* hblurredImage = new Gdiplus::Bitmap(width, height, loadedImage->GetPixelFormat());
    Gdiplus::Bitmap* blurredImage = new Gdiplus::Bitmap(width, height, loadedImage->GetPixelFormat());

    const int icount = (radius * 2 + 1);
    __m128i count = _mm_set1_epi32(icount);

    Gdiplus::Rect rect(0, 0, width, height);
    Gdiplus::BitmapData imgData, blurData;

    loadedImage->LockBits(&rect, Gdiplus::ImageLockModeRead, loadedImage->GetPixelFormat(), &imgData);
    hblurredImage->LockBits(&rect, Gdiplus::ImageLockModeWrite, hblurredImage->GetPixelFormat(), &blurData);

    for (int y = 0; y < height; y++) {
        for (int x = radius; x < width - radius; x++) {
            __m128i accum = _mm_setzero_si128();

            for (int kx = -radius; kx <= radius; kx++) {
                Gdiplus::Color c(((uint32_t*)imgData.Scan0)[y * width + x+kx]);
                __m128i color = _mm_set_epi32(c.GetB(), c.GetG(), c.GetR(), c.GetA());

                accum = _mm_add_epi32(accum, color);
            }

            accum = _mm_cvtps_epi32(_mm_div_ps(_mm_cvtepi32_ps(accum), _mm_cvtepi32_ps(count)));
            ((uint32_t*) blurData.Scan0)[y * width + x] = Gdiplus::Color::MakeARGB(accum.m128i_i32[0], accum.m128i_i32[1], accum.m128i_i32[2], accum.m128i_i32[3]);
        }
    }

    loadedImage->UnlockBits(&imgData);
    hblurredImage->UnlockBits(&blurData);

    hblurredImage->LockBits(&rect, Gdiplus::ImageLockModeRead, hblurredImage->GetPixelFormat(), &imgData);
    blurredImage->LockBits(&rect, Gdiplus::ImageLockModeWrite, blurredImage->GetPixelFormat(), &blurData);

    for (int y = radius; y < height - radius; y++) {
        for (int x = 0; x < width; x++) {
            __m128i accum = _mm_setzero_si128();

            for (int ky = -radius; ky <= radius; ky++) {
                Gdiplus::Color c(((uint32_t*)imgData.Scan0)[(y + ky) * width + x]);
                __m128i color = _mm_set_epi32(c.GetB(), c.GetG(), c.GetR(), c.GetA());

                accum = _mm_add_epi32(accum, color);
            }

            accum = _mm_cvtps_epi32(_mm_div_ps(_mm_cvtepi32_ps(accum), _mm_cvtepi32_ps(count)));
            ((uint32_t*)blurData.Scan0)[y * width + x] = Gdiplus::Color::MakeARGB(accum.m128i_i32[0], accum.m128i_i32[1], accum.m128i_i32[2], accum.m128i_i32[3]);
        }
    }

    hblurredImage->UnlockBits(&imgData);
    blurredImage->UnlockBits(&blurData);

    delete hblurredImage;

    delete loadedImage;
    loadedImage = blurredImage;

    Application::log(("Applied manual blur with radius: " + std::to_string(radius)).c_str());
}

/*void GdiPlusManager::ApplyBlur(int radius)
{
    if (!loadedImage) {
        return;
    }

    int width = loadedImage->GetWidth();
    int height = loadedImage->GetHeight();

    Gdiplus::Bitmap* hblurredImage = new Gdiplus::Bitmap(width, height, loadedImage->GetPixelFormat());
    Gdiplus::Bitmap* blurredImage = new Gdiplus::Bitmap(width, height, loadedImage->GetPixelFormat());

    for (int y = 0; y < height; y++) {
        for (int x = radius; x < width - radius; x++) {
            int r = 0, g = 0, b = 0;

            for (int kx = -radius; kx <= radius; kx++) {
                Gdiplus::Color color;
                loadedImage->GetPixel(x + kx, y, &color);

                r += color.GetRed();
                g += color.GetGreen();
                b += color.GetBlue();
            }

            int count = (radius * 2 + 1);
            hblurredImage->SetPixel(x, y, Gdiplus::Color(r / count, g / count, b / count));
        }
    }

    for (int y = radius; y < height - radius; y++) {
        for (int x = 0; x < width; x++) {
            int r = 0, g = 0, b = 0;

            for (int ky = -radius; ky <= radius; ky++) {
                Gdiplus::Color color;
                hblurredImage->GetPixel(x, y + ky, &color);

                r += color.GetRed();
                g += color.GetGreen();
                b += color.GetBlue();
            }

            int count = (radius * 2 + 1);
            blurredImage->SetPixel(x, y, Gdiplus::Color(r / count, g / count, b / count));
        }
    }

    delete hblurredImage;

    delete loadedImage;
    loadedImage = blurredImage;

    Application::log(("Applied manual blur with radius: " + std::to_string(radius)).c_str());
}*/

void GdiPlusManager::ApplySepia()
{
    if (!loadedImage) {
        return;
    }

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
