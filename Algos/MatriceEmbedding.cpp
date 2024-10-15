#include "MatriceEmbedding.h"
#include "Application.h"
#include <bitset>

using namespace std;
using namespace Gdiplus;


static string StringToBinary(const string& message) {
    string binary = "";
    for (char const& c : message) {
        binary += bitset<8>(c).to_string();
    }
    return binary;
}

void MatriceEmbedding::EmbedMessageInImage(const string& message) {
    string binaryMessage = StringToBinary(message);
    int dataIndex = 0;
    int dataLength = binaryMessage.length();
    bool complete = false;

    Bitmap* bmp = GdiPlusManager::getInstance().getImage();
    Bitmap* dest = new Gdiplus::Bitmap(bmp->GetWidth(), bmp->GetHeight());

    for (UINT y = 0; y < bmp->GetHeight(); ++y) {
        for (UINT x = 0; x < bmp->GetWidth(); ++x) {

            Color pixelColor;
            bmp->GetPixel(x, y, &pixelColor);

            BYTE red = pixelColor.GetR();
            BYTE green = pixelColor.GetG();
            BYTE blue = pixelColor.GetB();

            if (dataIndex < dataLength) {
                red = (red & 0xFE) | (binaryMessage[dataIndex] - '0');
                dataIndex++;
            }
            if (dataIndex < dataLength) {
                green = (green & 0xFE) | (binaryMessage[dataIndex] - '0');
                dataIndex++;
            }
            if (dataIndex < dataLength) {
                blue = (blue & 0xFE) | (binaryMessage[dataIndex] - '0');
                dataIndex++;
            }

            Color newColor(pixelColor.GetA(), red, green, blue);
            dest->SetPixel(x, y, newColor);

            if (dataIndex >= dataLength) {
                complete = true;
            }
        }
    }

    if (dataIndex < dataLength) {
        Application::log("Image too small to hide the whole message");
    }
    else {
        Application::log(("Message hidden perfectly :" + message).c_str());
    }

    GdiPlusManager::getInstance().setGeneratedImage(dest);
}

string MatriceEmbedding::DecodeMessageFromImage(Bitmap* bmp, int messageLength) {
    string binaryMessage = "";
    int bitCount = 0;

    for (UINT y = 0; y < bmp->GetHeight(); ++y) {
        for (UINT x = 0; x < bmp->GetWidth(); ++x) {
            Color pixelColor;
            bmp->GetPixel(x, y, &pixelColor);

            BYTE red = pixelColor.GetR();
            BYTE green = pixelColor.GetG();
            BYTE blue = pixelColor.GetB();

            binaryMessage += (red & 0x01) ? '1' : '0';
            ++bitCount;
            if (bitCount >= messageLength * 8) break;

            binaryMessage += (green & 0x01) ? '1' : '0';
            ++bitCount;
            if (bitCount >= messageLength * 8) break;

            binaryMessage += (blue & 0x01) ? '1' : '0';
            ++bitCount;
            if (bitCount >= messageLength * 8) break;
        }

        if (bitCount >= messageLength * 8) break;
    }

    string decodedMessage = "";
    for (size_t i = 0; i < binaryMessage.size(); i += 8) {
        bitset<8> byte(binaryMessage.substr(i, 8));
        decodedMessage += static_cast<char>(byte.to_ulong());
    }

    Application::log(("decoded message Matrix: " + decodedMessage).c_str());

    return decodedMessage;
}
