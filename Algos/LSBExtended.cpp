#include "LSBExtended.h"
#include "Application.h"
#include <bitset>

using namespace std;
using namespace Gdiplus;


void LSBExtended::EmbedMessageInImage(const BinaryBuffer& message)
{
    // Clone the source image.
    Gdiplus::Bitmap* src = GdiPlusManager::getInstance().getImage();

    Gdiplus::RectF rect;
    Unit unit;
    src->GetBounds(&rect, &unit);
    Gdiplus::Bitmap* bitmap = src->Clone(rect, src->GetPixelFormat());

    if (!bitmap) {
        Application::log("Failed to encode message!");
        return;
    }

    for (UINT i = 0; i < message.getSize(); i++) {
        UINT x = (i % src->GetWidth());
        UINT y = (i / src->GetWidth());

        Gdiplus::Color color;
        bitmap->GetPixel(x, y, &color);

        BYTE byte = message.getData()[i];
        BYTE r = (color.GetR() & 0xF8) | ((byte & 0x07));
        BYTE g = (color.GetG() & 0xFC) | ((byte & 0x18) >> 3);
        BYTE b = (color.GetB() & 0xF8) | ((byte & 0xE0) >> 5);

        bitmap->SetPixel(x, y, Gdiplus::Color(255, r, g, b));
    }

    GdiPlusManager::getInstance().setGeneratedImage(bitmap);
}

BinaryBuffer LSBExtended::DecodeMessageFromImage(Bitmap* bmp)
{
    if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok) {
        Application::log("Failed to load image");
        return {};
    }

    const int numBytes = bmp->GetWidth() * bmp->GetHeight();
    unsigned char* data = (unsigned char*) malloc(numBytes);
    memset(data, 0, numBytes);

    // Iterate over each pixel to extract the message
    unsigned char* cur = data;
    for (UINT y = 0; y < bmp->GetHeight(); y++) {
        for (UINT x = 0; x < bmp->GetWidth(); x++) {
            Gdiplus::Color color;
            bmp->GetPixel(x, y, &color);

            // Extract the LSBs from the RGB values
            *cur = (color.GetR() & 0x07) | ((color.GetG() & 0x03) << 3) | ((color.GetB() & 0x07) << 5);
            cur++;
        }
    }
    
    return BinaryBuffer(data, numBytes);
}
