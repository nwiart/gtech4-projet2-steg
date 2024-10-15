#include "LSB.h"
#include "Application.h"
#include <bitset>

using namespace std;
using namespace Gdiplus;


static vector<bool> GetMessageBits(const string& message) {
    vector<bool> bits;
    for (char c : message) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back((c >> i) & 1);
        }
    }
    return bits;
}

static string BitsToMessage(const vector<bool>& bits) {
    string message;
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


void LSB::EmbedMessageInImage(const string& message) {
    // Load the image
    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(GdiPlusManager::getInstance().getImage()->GetWidth(), GdiPlusManager::getInstance().getImage()->GetHeight());
    if (!bitmap) {
        Application::log("Failed to encode message!");
        return;
    }

    std::vector<bool> messageBits = GetMessageBits(message);
    int bitIndex = 0;

    // Iterate over each pixel to encode the message
    for (UINT y = 0; y < bitmap->GetHeight(); y++) {
        for (UINT x = 0; x < bitmap->GetWidth(); x++) {
            Gdiplus::Color pixelColor;
            GdiPlusManager::getInstance().getImage()->GetPixel(x, y, &pixelColor);

            // Modify the LSB of the pixel's RGB values
            BYTE r = (pixelColor.GetR() & ~1) | (bitIndex < messageBits.size() ? messageBits[bitIndex++] : 0);
            BYTE g = (pixelColor.GetG() & ~1) | (bitIndex < messageBits.size() ? messageBits[bitIndex++] : 0);
            BYTE b = (pixelColor.GetB() & ~1) | (bitIndex < messageBits.size() ? messageBits[bitIndex++] : 0);

            Gdiplus::Color newColor(pixelColor.GetA(), r, g, b);
            bitmap->SetPixel(x, y, newColor);
        }
    }

    Application::log(("Encoded message: " + message).c_str());

    GdiPlusManager::getInstance().setGeneratedImage(bitmap);
}

string LSB::DecodeMessageFromImage(Bitmap* bmp, int messageLength) {
    // Load the image
    Gdiplus::Bitmap* bitmap = bmp;

    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        Application::log("Failed to load image");
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

    Application::log(("decoded message: " + message).c_str());
    return message;
}
