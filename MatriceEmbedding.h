#include "GDIplos.h"
#include <windows.h>
#include <iostream>
#include <bitset>
#include <string>


using namespace Gdiplus;
using namespace std;

class MatriceEmbedding {
public:
    void EmbedMessageInImage(Bitmap* bmp, const string& message);
    string DecodeMessageFromImage(Bitmap* bmp, int messageLength);
};