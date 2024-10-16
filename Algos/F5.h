#pragma once

#include "BinaryBuffer.h"
#include "GDIplos.h"
#include <string>

using namespace std;

class F5 {
public:
    static void DCT_1D(double* block);
    static void DCT_2D(double block[8][8]);
    static void IDCT_1D(double* block);
    static void IDCT_2D(double block[8][8]);
    static void QuantizeBlock(double block[8][8], const int quantMatrix[8][8]);
    static void DequantizeBlock(double block[8][8], const int quantMatrix[8][8]);
    static void EmbedMessageInImage(const BinaryBuffer& message);
    static BinaryBuffer DecodeMessageFromImage(Gdiplus::Bitmap* bmp);
    static void Permute(vector<double>& coefficients, int seed);
};