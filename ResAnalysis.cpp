#include "ResAnalysis.h"
#include "Application.h"
#include <random>
#include <cmath>
using namespace Gdiplus;

ResAnalysis& ResAnalysis::GetInstance() {
	static ResAnalysis instance;
	return instance;
}

ResAnalysis::ResAnalysis()
{
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
	Application::log("GDI+ started successfully.");
}

ResAnalysis::~ResAnalysis()
{
}

static int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

BYTE Clamp(int value) {
	if (value < 0) return 0;
	if (value > 255) return 255;
	return static_cast<BYTE>(value);
}

bool ResAnalysis::AddNoise(double mean, double stddev) {

    Gdiplus::Bitmap* src = Bitmap::FromFile(L"Test.png", false);

    if (!src) {
        Application::log("Failed to get image");
        return false;
    }

    // Random number generator for Gaussian distribution
    std::default_random_engine generator;
    std::normal_distribution<double> distribution(mean, stddev);

    // Lock the bitmap for pixel manipulation
    BitmapData bitmapData;
    Rect rect(0, 0, src->GetWidth(), src->GetHeight());

    if (src->LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData) == Ok) {
        BYTE* pixels = static_cast<BYTE*>(bitmapData.Scan0);
        int stride = bitmapData.Stride;

        // Loop through each pixel
        for (UINT y = 0; y < src->GetHeight(); ++y) {
            for (UINT x = 0; x < src->GetWidth(); ++x) {
                int index = y * stride + x * 4; // 4 bytes per pixel (ARGB)

                // Generate Gaussian noise for each color channel
                int noiseR = static_cast<int>(distribution(generator));
                int noiseG = static_cast<int>(distribution(generator));
                int noiseB = static_cast<int>(distribution(generator));

                // Add noise and clamp values to stay within [0, 255]
                pixels[index + 2] = Clamp(pixels[index + 2] + noiseR); // R
                pixels[index + 1] = Clamp(pixels[index + 1] + noiseG); // G
                pixels[index + 0] = Clamp(pixels[index + 0] + noiseB); // B
            }
        }

        // Unlock the bitmap
        src->UnlockBits(&bitmapData);
    }

    CLSID clsid;
    CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsid);
    src->Save(L"output_with_noise.png", &clsid, nullptr);

    Application::log("Image generated successfully with noise");
    return true;
}

bool ResAnalysis::AddCompression(ULONG quality) { //50 by default

    Gdiplus::Bitmap* src = Bitmap::FromFile(L"Test.png", false);

    if (!src) {
        Application::log("Failed to get image");
        return false;
    }


    if (quality > 100) quality = 100;
    if (quality < 0) quality = 0;

    // Get the JPEG encoder CLSID
    CLSID jpegClsid;
    if (GetEncoderClsid(L"image/jpeg", &jpegClsid) == -1) {
        Application::log("JPEG encoder not found");
        return false;
    }

    // Set the quality parameter
    EncoderParameters encoderParameters;
    encoderParameters.Count = 1;
    encoderParameters.Parameter[0].Guid = EncoderQuality;
    encoderParameters.Parameter[0].Type = EncoderParameterValueTypeLong;
    encoderParameters.Parameter[0].NumberOfValues = 1;
    encoderParameters.Parameter[0].Value = &quality;

    // Create a memory stream to save the compressed image
    IStream* stream = nullptr;
    HRESULT hr = CreateStreamOnHGlobal(NULL, TRUE, &stream);
    if (FAILED(hr) || stream == nullptr) {
        Application::log("Failed to create memory stream");
        return false;
    }

    // Save the image to the stream with the specified compression level
    Status status = src->Save(stream, &jpegClsid, &encoderParameters);
    if (status != Ok) {
        Application::log("Failed to save the image to the stream");
        stream->Release();
        return false;
    }

    // Load the compressed image from the stream into a new bitmap
    Bitmap* compressedBitmap = new Bitmap(stream);
    if (compressedBitmap->GetLastStatus() != Ok) {
        Application::log("Failed to create compressed bitmap from stream");
        delete compressedBitmap;
        stream->Release();
        return false;
    }

    CLSID clsid;
    CLSIDFromString(L"{557CF406-1A04-11D3-9A73-0000F81EF32E}", &clsid);
    src->Save(L"output_with_Compression.png", &clsid, nullptr);

    // Clean up
    stream->Release();
    Application::log("Image generated successfully with compression quality: " + quality);
    return true;
}

void ResAnalysis::GenerateImages() {
    ResAnalysis::AddNoise(0.0, 20.0);
    ResAnalysis::AddCompression(50);
}
