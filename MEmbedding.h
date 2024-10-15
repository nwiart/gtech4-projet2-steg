#pragma once
#include "GDIplos.h"
#include <windows.h>
#include <iostream>
#include <bitset>
#include <string>


using namespace Gdiplus;
using namespace std;

class MatriceEmbedding {
public:
    Gdiplus::Bitmap* EmbedMessageInImage(const string& message);
    string DecodeMessageFromImage(Bitmap* bmp);
};