#pragma once

#include "BinaryBuffer.h"
#include "GDIplos.h"
#include <string>


class LSB {
public:
    static void EmbedMessageInImage(const BinaryBuffer& msg);
    static BinaryBuffer DecodeMessageFromImage(Gdiplus::Bitmap* bmp);
};
