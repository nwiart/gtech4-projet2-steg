#include <cmath>
#include <random>
#include <vector>
#include <algorithm>
#include "F5.h"
#include "Application.h"

#define BIT(byte, i) ((byte & (1 << i)) >> i)
#undef min
#undef max

void F5::DCT_1D(double* block) {
    const double PI = 3.14159265358979323846;
    double result[8] = { 0 };

    for (int u = 0; u < 8; ++u) {
        double sum = 0;
        for (int x = 0; x < 8; ++x) {
            sum += block[x] * cos((PI * (2 * x + 1) * u) / 16.0);
        }
        result[u] = sum * (u == 0 ? 1 / sqrt(2) : 1.0);
    }

    for (int i = 0; i < 8; ++i) {
        block[i] = result[i] / 2.0;
    }
}

void F5::DCT_2D(double block[8][8]) {
    for (int i = 0; i < 8; ++i) {
        DCT_1D(block[i]);
    }

    for (int i = 0; i < 8; ++i) {

        double col[8];
        for (int j = 0; j < 8; ++j) {
            col[j] = block[j][i];
        }
        DCT_1D(col);
        for (int j = 0; j < 8; ++j) {
            block[j][i] = col[j];
        }
    }
}

void F5::IDCT_1D(double* block) 
{
    const double PI = 3.14159265358979323846;
    double result[8] = { 0 };

    for (int x = 0; x < 8; ++x) 
    {
        double sum = 0;
        for (int u = 0; u < 8; ++u) 
        {
            sum += block[u] * cos((PI * (2 * x + 1) * u) / 16.0) * (u == 0 ? 1 / sqrt(2) : 1.0);
        }
        result[x] = sum / 2.0;
    }

    for (int i = 0; i < 8; ++i) 
    {
        block[i] = result[i];
    }
}

void F5::IDCT_2D(double block[8][8]) 
{
    for (int i = 0; i < 8; ++i) 
    {
        IDCT_1D(block[i]);
    }

    for (int i = 0; i < 8; ++i) 
    {
        double col[8];
        for (int j = 0; j < 8; ++j) 
        {
            col[j] = block[j][i];
        }
        IDCT_1D(col);
        for (int j = 0; j < 8; ++j) 
        {
            block[j][i] = col[j];
        }
    }
}

void F5::QuantizeBlock(double block[8][8], const int quantMatrix[8][8]) 
{
    for (int i = 0; i < 8; ++i) 
    {
        for (int j = 0; j < 8; ++j) 
        {
            block[i][j] = round(block[i][j] / quantMatrix[i][j]);
        }
    }
}

void F5::DequantizeBlock(double block[8][8], const int quantMatrix[8][8]) 
{
    for (int i = 0; i < 8; ++i) 
    {
        for (int j = 0; j < 8; ++j) 
        {
            block[i][j] = block[i][j] * quantMatrix[i][j];
        }
    }
}

void F5::EmbedMessageInImage(const BinaryBuffer& message)
{
    Gdiplus::Bitmap* bitmap = GdiPlusManager::getInstance().getImage();
    if (!bitmap || bitmap->GetWidth() == 0 || bitmap->GetHeight() == 0) {
        Application::log("Input image is invalid!");
        return;
    }

    const int quantMatrix[8][8] =
    {
      { 16, 11, 10, 16, 24, 40, 51, 61 },
      { 12, 12, 14, 19, 26, 58, 60, 55 },
      { 14, 13, 16, 24, 40, 57, 69, 56 },
      { 14, 17, 22, 29, 51, 87, 80, 62 },
      { 18, 22, 37, 56, 68, 109, 103, 77 },
      { 24, 35, 55, 64, 81, 104, 113, 92 },
      { 49, 64, 78, 87, 103, 121, 120, 101 },
      { 72, 92, 95, 98, 112, 100, 103, 99 }
    };

    int bitIndex = 0;
    const int numBits = message.getSize() * 8;
    int width = bitmap->GetWidth();
    int height = bitmap->GetHeight();

    for (int i = 0; i < width; i += 8)
    {
        for (int j = 0; j < height; j += 8) {
            double block[8][8] = { 0 };

            for (int x = 0; x < 8; ++x)
            {
                for (int y = 0; y < 8; ++y)
                {
                    if (i + x < width && j + y < height)
                    {
                        Gdiplus::Color pixelColor;
                        bitmap->GetPixel(i + x, j + y, &pixelColor);

                        int r = pixelColor.GetR();
                        int g = pixelColor.GetG();
                        int b = pixelColor.GetB();

                        block[x][y] = 0.299 * r + 0.587 * g + 0.114 * b;
                    }
                }
            }

            DCT_2D(block);
            QuantizeBlock(block, quantMatrix);

            std::vector<double> coefficients;
            for (int u = 1; u < 8; ++u) {
                for (int v = 1; v < 8; ++v) {
                    if (block[u][v] != 0) {
                        coefficients.push_back(block[u][v]);
                    }
                }
            }

            Permute(coefficients, i + j);

            int coeffIndex = 0;
            for (int u = 1; u < 8 && bitIndex < numBits && coeffIndex < coefficients.size(); ++u) {
                for (int v = 1; v < 8 && bitIndex < numBits && coeffIndex < coefficients.size(); ++v) {
                    if (block[u][v] != 0) {
                        int bit = BIT(message.getData()[bitIndex / 8], bitIndex % 8);
                        coefficients[coeffIndex] = (static_cast<int>(coefficients[coeffIndex]) & ~1) | bit;
                        ++bitIndex;
                        ++coeffIndex;
                    }
                }
            }

            coeffIndex = 0;
            for (int u = 1; u < 8 && coeffIndex < coefficients.size(); ++u) {
                for (int v = 1; v < 8 && coeffIndex < coefficients.size(); ++v) {
                    if (block[u][v] != 0) {
                        block[u][v] = coefficients[coeffIndex];
                        ++coeffIndex;
                    }
                }
            }

            DequantizeBlock(block, quantMatrix);
            IDCT_2D(block);

            for (int x = 0; x < 8; ++x)
            {
                for (int y = 0; y < 8; ++y)
                {
                    if (i + x < width && j + y < height)
                    {
                        Gdiplus::Color originalColor;
                        bitmap->GetPixel(i + x, j + y, &originalColor);

                        int luminance = round(block[x][y]);
                        luminance = std::max(0, std::min(255, luminance));

                        int newR = std::max(0, std::min(255, originalColor.GetR() + (luminance - originalColor.GetR())));
                        int newG = std::max(0, std::min(255, originalColor.GetG() + (luminance - originalColor.GetG())));
                        int newB = std::max(0, std::min(255, originalColor.GetB() + (luminance - originalColor.GetB())));

                        Gdiplus::Color pixelColor(newR, newG, newB);
                        bitmap->SetPixel(i + x, j + y, pixelColor);
                    }
                }
            }
        }
    }

    GdiPlusManager::getInstance().setGeneratedImage(bitmap);
    Application::log("Message successfully encoded!");
}

BinaryBuffer F5::DecodeMessageFromImage(Gdiplus::Bitmap* bmp) 
{
    if (!bmp || bmp->GetLastStatus() != Gdiplus::Ok) 
    {
        Application::log("Failed to load image");
        return {};
    }

    const int quantMatrix[8][8] = 
    {
        { 16, 11, 10, 16, 24, 40, 51, 61 },
        { 12, 12, 14, 19, 26, 58, 60, 55 },
        { 14, 13, 16, 24, 40, 57, 69, 56 },
        { 14, 17, 22, 29, 51, 87, 80, 62 },
        { 18, 22, 37, 56, 68, 109, 103, 77 },
        { 24, 35, 55, 64, 81, 104, 113, 92 },
        { 49, 64, 78, 87, 103, 121, 120, 101 },
        { 72, 92, 95, 98, 112, 100, 103, 99 }
    };

    int width = bmp->GetWidth();
    int height = bmp->GetHeight();

    std::vector<int> messageBits;
    unsigned char tempByte = 0;
    int bitCount = 0;

    BinaryBuffer messageBuffer;
    for (int i = 0; i < width; i += 8) 
    {
        for (int j = 0; j < height; j += 8) 
        {
            double block[8][8] = { 0 };

            for (int x = 0; x < 8; ++x) 
            {
                for (int y = 0; y < 8; ++y) 
                {
                    if (i + x < width && j + y < height) 
                    {
                        Gdiplus::Color pixelColor;
                        bmp->GetPixel(i + x, j + y, &pixelColor);
                        block[x][y] = 0.299 * pixelColor.GetR() + 0.587 * pixelColor.GetG() + 0.114 * pixelColor.GetB();
                    }
                }
            }

            DCT_2D(block);
            DequantizeBlock(block, quantMatrix);

            for (int u = 1; u < 8; ++u) 
            {
                for (int v = 1; v < 8; ++v) 
                {
                    if (block[u][v] != 0) 
                    {
                        int bit = static_cast<int>(block[u][v]) & 1;
                        tempByte |= (bit << bitCount);
                        bitCount++;

                        if (bitCount == 8) 
                        {
                            unsigned char* data = messageBuffer.getData();
                            data[messageBuffer.getSize()] = tempByte;
                            bitCount = 0;
                            tempByte = 0;
                        }
                    }
                }
            }
        }
    }

    return messageBuffer;
}

void F5::Permute(std::vector<double>& coefficients, int seed) 
{
    std::mt19937 rng(seed);
    std::shuffle(coefficients.begin(), coefficients.end(), rng);
}
