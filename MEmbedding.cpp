#include "MEmbedding.h"
#include "Application.h"
#include "GDIplos.h"
#include <bitset>
#include <string>

static std::vector<bool> StringToBinary(const string& message) {
    std::vector<bool> bits;
    for (char c : message) {
        for (int i = 7; i >= 0; --i) {
            bits.push_back((c >> i) & 1);
        }
    }
    return bits;
}

std::vector<bool> calculateHammingErrorVector(const std::vector<bool>& coverBits, const std::vector<bool>& messageBits) {
    std::vector<bool> errorVector(7, false);
    // For simplicity, assuming a hardcoded parity check matrix for (7,4) Hamming code
    int parityCheckMatrix[3][7] = {
        {1, 0, 1, 0, 1, 0, 1}, // row for first parity bit
        {0, 1, 1, 0, 0, 1, 1}, // row for second parity bit
        {0, 0, 0, 1, 1, 1, 1}  // row for third parity bit
    };

    // Compute the syndrome
    int syndrome[3] = { 0, 0, 0 };
    for (int i = 0; i < 7; ++i) {
        if (coverBits[i] != messageBits[i]) {
            for (int j = 0; j < 3; ++j) {
                syndrome[j] ^= parityCheckMatrix[j][i];
            }
        }
    }

    // Determine which bit to flip based on the syndrome
    int errorPosition = (syndrome[0] << 2) | (syndrome[1] << 1) | syndrome[2];
    if (errorPosition > 0 && errorPosition <= 7) {
        errorVector[errorPosition - 1] = true;
    }

    return errorVector;
}

Gdiplus::Bitmap* MatriceEmbedding::EmbedMessageInImage(const string& message) {

    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(GdiPlusManager::getInstance().getImage()->GetWidth(), GdiPlusManager::getInstance().getImage()->GetHeight());
    if (!bitmap) {
        Application::log("Failed to encode message!");
        return bitmap;
    }

    std::vector<bool> messageBits = StringToBinary(message);
    int bitIndex = 0;
    std::vector<bool> coverBits(7);
    std::vector<bool> messageBlock(4);

    // Iterate over each pixel to encode the message using matrix embedding
    for (UINT y = 0; y < bitmap->GetHeight(); y++) {
        for (UINT x = 0; x < bitmap->GetWidth(); x++) {
            if (bitIndex >= messageBits.size()) {
                break; // Exit if all message bits are embedded
            }

            // Read 7 cover bits from the image
            for (int i = 0; i < 7; i++) {
                if (bitIndex + i < messageBits.size()) {
                    Gdiplus::Color pixelColor;
                    GdiPlusManager::getInstance().getImage()->GetPixel((x + i) % bitmap->GetWidth(), y, &pixelColor);
                    coverBits[i] = pixelColor.GetR() & 1;
                }
            }

            // Get the next 4 message bits
            for (int i = 0; i < 4; i++) {
                messageBlock[i] = (bitIndex + i < messageBits.size()) ? messageBits[bitIndex + i] : 0;
            }

            // Calculate the error vector based on the Hamming code (7,4)
            std::vector<bool> errorVector = calculateHammingErrorVector(coverBits, messageBlock);

            // Modify the cover bits according to the error vector
            for (int i = 0; i < 7; i++) {
                if (errorVector[i]) {
                    // Flip the bit in the image (modify the LSB)
                    Gdiplus::Color pixelColor;
                    GdiPlusManager::getInstance().getImage()->GetPixel((x + i) % bitmap->GetWidth(), y, &pixelColor);
                    BYTE newR = pixelColor.GetR() ^ 1; // Flip the LSB
                    Gdiplus::Color newColor(pixelColor.GetA(), newR, pixelColor.GetG(), pixelColor.GetB());
                    bitmap->SetPixel((x + i) % bitmap->GetWidth(), y, newColor);
                }
            }

            // Move to the next block of 4 message bits
            bitIndex += 4;
            x += 6; // Skip the extra pixels used for the block
        }
    }

    Application::log(("Encoded message: " + message).c_str());
    GdiPlusManager::getInstance().setGeneratedImage(bitmap);
    return bitmap;
}

string MatriceEmbedding::DecodeMessageFromImage(Bitmap* bmp) {
    std::string decodedMessage;
    std::vector<bool> messageBits;

    // Read the cover bits from the image in blocks
    for (UINT y = 0; y < bmp->GetHeight(); y++) {
        for (UINT x = 0; x < bmp->GetWidth(); x += 7) { // Step by 7, assuming (7,4) Hamming
            std::vector<bool> coverBits(7);
            for (int i = 0; i < 7; i++) {
                Gdiplus::Color pixelColor;
                bmp->GetPixel((x + i) % bmp->GetWidth(), y, &pixelColor);
                coverBits[i] = pixelColor.GetR() & 1; // Get the LSB of the red channel
            }

            // Calculate the syndrome
            std::vector<bool> errorSyndrome = calculateHammingErrorVector(coverBits, messageBits);
            // Correct errors if necessary
            if (errorSyndrome != std::vector<bool>(3, false)) {
                // Correct the error (similar logic to encoding)
                int errorPosition = (errorSyndrome[0] << 2) | (errorSyndrome[1] << 1) | errorSyndrome[2];
                if (errorPosition > 0 && errorPosition <= 7) {
                    coverBits[errorPosition - 1] = coverBits[errorPosition - 1] ^ 1; // Flip the erroneous bit
                }
            }

            // Extract the message bits from the coverBits
            for (int i = 0; i < 4; i++) { // Get the first 4 bits as message bits
                messageBits.push_back(coverBits[i]); // Assuming the message is in the first 4 positions
            }
        }
    }

    // Convert the bits back to a string
    for (size_t i = 0; i < messageBits.size(); i += 8) {
        char c = 0;
        for (int j = 0; j < 8; ++j) {
            if (i + j < messageBits.size()) {
                c |= (messageBits[i + j] << (7 - j)); // Assemble the byte
            }
        }
        decodedMessage += c; // Append the character
    }

    return decodedMessage;
}
