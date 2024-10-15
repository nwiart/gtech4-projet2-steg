#pragma once

#include "GDIplos.h"
#include <string>


class LSB {
public:
    static void EmbedMessageInImage(const std::string& message);
    static std::string DecodeMessageFromImage(Gdiplus::Bitmap* bmp, int messageLength);
};
