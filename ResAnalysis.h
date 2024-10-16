#pragma once
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>

#pragma comment (lib, "Gdiplus.lib")

class ResAnalysis
{
public:
    static ResAnalysis& GetInstance();

    ResAnalysis();
    ~ResAnalysis();

    void AnalyseHistogram();
    std::string ExtractLSB();

private:
    ULONG_PTR gdiplusToken; // Store GDI+ token
};
