#include "LSB.h"
#include "Application.h"
#include <bitset>

using namespace std;
using namespace Gdiplus;


#define BIT(byte, i) ((byte & (1 << i)) >> i)

void LSB::EmbedMessageInImage(const BinaryBuffer& message) {
    // Load the image
    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(GdiPlusManager::getInstance().getImage()->GetWidth(), GdiPlusManager::getInstance().getImage()->GetHeight());
    if (!bitmap) {
        Application::log("Failed to encode message!");
        return;
    }

    const int numBits = message.getSize() * 8;
    int bitIndex = 0;

    // Iterate over each pixel to encode the message
    for (UINT y = 0; y < bitmap->GetHeight(); y++) {
        for (UINT x = 0; x < bitmap->GetWidth(); x++) {
            Gdiplus::Color pixelColor;
            GdiPlusManager::getInstance().getImage()->GetPixel(x, y, &pixelColor);

            // Modify the LSB of the pixel's RGB values
            BYTE r = (pixelColor.GetR() & ~1) | (bitIndex < numBits ? BIT(message.getData()[bitIndex / 8], bitIndex % 8) : 0); bitIndex++;
            BYTE g = (pixelColor.GetG() & ~1) | (bitIndex < numBits ? BIT(message.getData()[bitIndex / 8], bitIndex % 8) : 0); bitIndex++;
            BYTE b = (pixelColor.GetB() & ~1) | (bitIndex < numBits ? BIT(message.getData()[bitIndex / 8], bitIndex % 8) : 0); bitIndex++;

            Gdiplus::Color newColor(pixelColor.GetA(), r, g, b);
            bitmap->SetPixel(x, y, newColor);
        }
    }

    Application::log((std::string("Encoded ") + std::to_string(message.getSize()) + " bytes.").c_str());

    GdiPlusManager::getInstance().setGeneratedImage(bitmap);
}

BinaryBuffer LSB::DecodeMessageFromImage(Bitmap* bmp) {
    // Load the image
    Gdiplus::Bitmap* bitmap = bmp;

    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        Application::log("Failed to load image");
        return {};
    }

    const int numBits = bmp->GetWidth() * bmp->GetHeight() * 3;
    const int numBytes = numBits / 8;
    unsigned char* data = (unsigned char*) malloc(numBytes);
    memset(data, 0, numBytes);

    int bits = 0;

    // Iterate over each pixel to extract the message
    for (UINT y = 0; y < bitmap->GetHeight(); y++) {
        for (UINT x = 0; x < bitmap->GetWidth(); x++) {
            Gdiplus::Color pixelColor;
            bitmap->GetPixel(x, y, &pixelColor);

            // Extract the LSBs from the RGB values
            data[bits / 8] |= ((pixelColor.GetR() & 1) << (bits % 8)); bits++;
            data[bits / 8] |= ((pixelColor.GetG() & 1) << (bits % 8)); bits++;
            data[bits / 8] |= ((pixelColor.GetB() & 1) << (bits % 8)); bits++;
        }
    }

    Application::log((std::string("Decoded ") + std::to_string(numBytes) + " bytes.").c_str());
    
    return BinaryBuffer(data, numBytes);
}
