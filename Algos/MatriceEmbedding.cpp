#include "MatriceEmbedding.h"
#include "Application.h"
#include <bitset>

using namespace std;
using namespace Gdiplus;

#include <vector>
#include <stdexcept>

std::vector<int> encode_hamming_7_4(const std::vector<int>& data) {
    if (data.size() != 4) throw std::invalid_argument("Incorrect data size for Hamming 7,4");

    std::vector<int> hamming(7);

    hamming[0] = data[0] ^ data[1] ^ data[3];
    hamming[1] = data[0] ^ data[2] ^ data[3];
    hamming[3] = data[1] ^ data[2] ^ data[3];

    hamming[2] = data[0];
    hamming[4] = data[1];
    hamming[5] = data[2];
    hamming[6] = data[3];

    return hamming;
}

std::vector<int> decode_hamming_7_4(std::vector<int> hamming) {
    if (hamming.size() != 7) throw std::invalid_argument("Incorrect data size for Hamming 7,4");

    int p1 = hamming[0] ^ hamming[2] ^ hamming[4] ^ hamming[6];
    int p2 = hamming[1] ^ hamming[2] ^ hamming[5] ^ hamming[6];
    int p4 = hamming[3] ^ hamming[4] ^ hamming[5] ^ hamming[6];

    int syndrome = (p4 << 2) | (p2 << 1) | p1;

    if (syndrome != 0) {
        hamming[syndrome - 1] = !hamming[syndrome - 1];
    }

    return { hamming[2], hamming[4], hamming[5], hamming[6] };
}

std::vector<int> stringToBinaryVector(const std::string& message) {
    std::vector<int> binaryMessage;
    for (char const& c : message) {
        std::bitset<8> bits(c);
        for (int i = 0; i < 8; ++i) {
            binaryMessage.push_back(bits[i]);
        }
    }
    return binaryMessage;
}

void MatriceEmbedding::EmbedMessageInImage(const string& message) {
    std::vector<int> dataBits = stringToBinaryVector(message);

    std::string endMarker = "\x03";
    std::vector<int> endMarkerBits = stringToBinaryVector(endMarker);
    dataBits.insert(dataBits.end(), endMarkerBits.begin(), endMarkerBits.end());

    std::vector<int> hammingBits;
    for (size_t i = 0; i < dataBits.size(); i += 4) {
        std::vector<int> chunk;
        for (size_t j = i; j < i + 4 && j < dataBits.size(); ++j) {
            chunk.push_back(dataBits[j]);
        }

        while (chunk.size() < 4) {
            chunk.push_back(0);
        }

        std::vector<int> hammingChunk = encode_hamming_7_4(chunk);
        hammingBits.insert(hammingBits.end(), hammingChunk.begin(), hammingChunk.end());
    }

    int dataIndex = 0;
    int dataLength = hammingBits.size();
    bool complete = false;

    Bitmap* bmp = GdiPlusManager::getInstance().getImage();
    Bitmap* dest = new Gdiplus::Bitmap(bmp->GetWidth(), bmp->GetHeight());

    for (UINT y = 0; y < bmp->GetHeight() && !complete; ++y) {
        for (UINT x = 0; x < bmp->GetWidth(); ++x) {

            Color pixelColor;
            bmp->GetPixel(x, y, &pixelColor);

            BYTE red = pixelColor.GetR();
            BYTE green = pixelColor.GetG();
            BYTE blue = pixelColor.GetB();

            if (dataIndex < dataLength) {
                red = (red & 0xFE) | hammingBits[dataIndex];
                dataIndex++;
            }
            if (dataIndex < dataLength) {
                green = (green & 0xFE) | hammingBits[dataIndex];
                dataIndex++;
            }
            if (dataIndex < dataLength) {
                blue = (blue & 0xFE) | hammingBits[dataIndex];
                dataIndex++;
            }

            Color newColor(pixelColor.GetA(), red, green, blue);
            dest->SetPixel(x, y, newColor);
        }
    }

    if (dataIndex < dataLength) {
        Application::log("Image too small to hide the entire message");
    }
    else {
        Application::log(("Message hidden: " + message).c_str());
    }

    GdiPlusManager::getInstance().setGeneratedImage(dest);
}

string MatriceEmbedding::DecodeMessageFromImage(Bitmap* bmp) {
    std::vector<int> hammingBits;
    int bitCount = 0;

    if (!bmp) {
        Application::log("Error: Image not found.");
        return "";
    }

    for (UINT y = 0; y < bmp->GetHeight(); ++y) {
        for (UINT x = 0; x < bmp->GetWidth(); ++x) {
            Color pixelColor;
            bmp->GetPixel(x, y, &pixelColor);

            BYTE red = pixelColor.GetR();
            BYTE green = pixelColor.GetG();
            BYTE blue = pixelColor.GetB();

            hammingBits.push_back(red & 0x01);
            ++bitCount;

            hammingBits.push_back(green & 0x01);
            ++bitCount;

            hammingBits.push_back(blue & 0x01);
            ++bitCount;
        }
    }

    std::vector<int> decodedBits;
    for (size_t i = 0; i + 7 <= hammingBits.size(); i += 7) {
        std::vector<int> chunk(hammingBits.begin() + i, hammingBits.begin() + i + 7);
        std::vector<int> dataChunk = decode_hamming_7_4(chunk);
        decodedBits.insert(decodedBits.end(), dataChunk.begin(), dataChunk.end());
    }

    string decodedMessage = "";
    if (decodedBits.size() % 8 != 0) {
        Application::log("Error: Incorrect number of decoded bits for conversion.");
        return "";
    }

    for (size_t i = 0; i < decodedBits.size(); i += 8) {
        bitset<8> byte;
        for (int j = 0; j < 8; ++j) {
            byte[j] = decodedBits[i + j];
        }

        char decodedChar = static_cast<char>(byte.to_ulong());

        if (decodedChar == '\x03') {
            break;
        }

        decodedMessage += decodedChar;
    }

    Application::log(("Message decoded: " + decodedMessage).c_str());

    return decodedMessage;
}
