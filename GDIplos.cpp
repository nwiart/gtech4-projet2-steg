#include <windows.h>
#include "GdiPlos.h"
#include <commdlg.h>
#include <iostream>
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

std::vector<bool> GdiPlusManager::GetMessageBits(const std::string& message) {
    std::vector<bool> bits;
    for (char c : message) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back((c >> i) & 1);
        }
    }
    return bits;
}


Gdiplus::Bitmap* GdiPlusManager::EncodeMessage(const std::string& message) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Load the image
    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(GdiPlusManager::getInstance().getImage()->GetWidth(), GdiPlusManager::getInstance().getImage()->GetHeight());
    if (!bitmap) {
        std::cout << "Failed to load image!" << std::endl;
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return bitmap;
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
    
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return bitmap;
}

std::string GdiPlusManager::BitsToMessage(const std::vector<bool>& bits) {
    std::string message;
    for (size_t i = 0; i < bits.size(); i += 8) {
        char c = 0;
        for (int j = 0; j < 8; ++j) {
            c |= bits[i + j] << (7 - j);
        }
        if (c == '\0') {
            break; // End of message
        }
        message += c;
    }
    return message;
}

std::string GdiPlusManager::DecodeMessage(Gdiplus::Bitmap* image) {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    // Load the image
    Gdiplus::Bitmap* bitmap = image;

    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        std::cout << "Failed to load image!" << std::endl;
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return "";
    }

    std::vector<bool> messageBits;

    // Iterate over each pixel to extract the message
    for (UINT y = 0; y < bitmap->GetHeight(); y++) {
        for (UINT x = 0; x < bitmap->GetWidth(); x++) {
            Gdiplus::Color pixelColor;
            bitmap->GetPixel(x, y, &pixelColor);

            // Extract the LSBs from the RGB values
            messageBits.push_back(pixelColor.GetR() & 1);
            messageBits.push_back(pixelColor.GetG() & 1);
            messageBits.push_back(pixelColor.GetB() & 1);

            // Check if the message contains a multiple of 8 bits
            if (messageBits.size() % 8 == 0) {
                // Try to detect the end of the message
                std::string currentMessage = BitsToMessage(messageBits);
                if (!currentMessage.empty() && currentMessage.back() == '\0') {
                    messageBits.resize(messageBits.size() - 8); // Remove the delimiter
                    break;
                }
            }
        }
    }

    // Decode the bits into a message
    std::string message = BitsToMessage(messageBits);

    // Cleanup
    delete bitmap;
    Gdiplus::GdiplusShutdown(gdiplusToken);
    return message;
}

int GdiPlusManager::TestMain() {
    // Example usage for encoding
    std::string message = "Hello, this is a secret message!";

    // Example usage for decoding
    std::string decodedMessage = GdiPlusManager::DecodeMessage(GdiPlusManager::EncodeMessage(message + '\0'));
    if (!decodedMessage.empty()) {
        std::cout << "Decoded message: " << decodedMessage << std::endl;
    }
    else {
        std::cout << "Failed to decode the message." << std::endl;
    }

    return 0;
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
