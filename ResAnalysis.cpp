#include "ResAnalysis.h"
#include "Application.h"
#include "GDIplos.h"
#include <random>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
using namespace Gdiplus;

ResAnalysis& ResAnalysis::GetInstance() {
    static ResAnalysis instance;
    return instance;
}

ResAnalysis::ResAnalysis() {
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    Application::log("GDI+ started successfully.");
}

ResAnalysis::~ResAnalysis() {
    // Shutdown GDI+
    GdiplusShutdown(gdiplusToken);
}

std::string ConvertWCharToChar(const WCHAR* wstr) {
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    std::string str(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &str[0], sizeNeeded, NULL, NULL);
    return str;
}

void ResAnalysis::AnalyseHistogram() {
    
    Bitmap* bmp = GdiPlusManager::getInstance().getImage();

    if (!bmp) {
        Application::log("Error: Invalid bitmap pointer!");
        return;
    }

    // Create a histogram array
    const int histogramSize = 256;
    std::vector<int> histogram(histogramSize, 0);

    for (UINT y = 0; y < bmp->GetHeight(); ++y) {
        for (UINT x = 0; x < bmp->GetWidth(); ++x) {
            Color color;
            bmp->GetPixel(x, y, &color);
            int grayValue = (color.GetR() + color.GetG() + color.GetB()) / 3; // Convert to grayscale
            histogram[grayValue]++;
        }
    }

    for (int i = 0; i < histogramSize; ++i) {
        WCHAR logMessage[100];
        swprintf(logMessage, 100, L"Pixel value %d: %d", i, histogram[i]);
        Application::log(ConvertWCharToChar(logMessage).c_str());
    }
}

std::string ResAnalysis::ExtractLSB() {
    Bitmap* bmp = GdiPlusManager::getInstance().getImage();

    if (!bmp) {
        Application::log("Error: Invalid bitmap pointer!");
        return "";
    }

    std::string hiddenMessage;
    const int messageTerminationLength = 8;
    int terminationCount = 0;

    // Get pixel data
    for (UINT y = 0; y < bmp->GetHeight(); ++y) {
        for (UINT x = 0; x < bmp->GetWidth(); ++x) {
            Color color;
            bmp->GetPixel(x, y, &color);

            // B
            hiddenMessage += std::to_string(color.GetB() & 1);

            if (hiddenMessage.length() >= messageTerminationLength) {
                // Check if the last 8 bits are 00000000
                if (hiddenMessage.substr(hiddenMessage.length() - messageTerminationLength) == "00000000") {
                    return hiddenMessage;
                }
            }
        }
    }

    // If no termination found, return the hidden message anyway
    return hiddenMessage;
}

