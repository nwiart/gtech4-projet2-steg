#pragma once

#include "GDIplos.h"
#include <string>


class MatriceEmbedding {
public:
    static void EmbedMessageInImage(const std::string& message);
    static std::string DecodeMessageFromImage(Gdiplus::Bitmap* bmp, int messageLength);
};
