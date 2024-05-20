#ifndef BMP_H
#define BMP_H

#include <stdint.h>

typedef struct CIEXYZ {
    int32_t ciexyzX;
    int32_t ciexyzY;
    int32_t ciexyzZ;
} CIEXYZ;

typedef struct CIEXYZTRIPLE {
    uint32_t ciexyzRed;
    uint32_t ciexyzGreen;
    uint32_t ciexyzBlue;
} CIEXYZTRIPLE;

typedef struct RGBQUAD {
    uint8_t rgbBlue;
    uint8_t rgbGreen;
    uint8_t rgbRed;
    uint8_t rgbReserved;
} RGBQUAD;

#pragma pack(push, 1)
typedef struct BITMAPFILEHEADER {
    char bfType[2];      // "BM"
    uint32_t bfSize;      // File size in bytes
    uint16_t bfReserved1; // 0
    uint16_t bfReserved2; // 0
    uint32_t bfOffBits;   // Offset to image data in bytes
} BITMAPFILEHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct BITMAPCOREHEADER {
    uint32_t bcSize;     // Size of this header
    uint16_t bcWidth;    // Image width in pixels
    uint16_t bcHeight;   // Image height in pixels
    uint16_t bcPlanes;   // Number of color planes. Must be 1
    uint16_t bcBitCount; // Number of bits per pixel. 1, 4, 8, 24 or 32
} BITMAPCOREHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct BITMAPINFOHEADER {
    uint32_t biSize;         // Size of this header
    int32_t biWidth;         // Image width in pixels
    int32_t biHeight;        // Image height in pixels
    uint16_t biPlanes;       // Number of color planes. Must be 1
    uint16_t biBitCount;     // Number of bits per pixel. 1, 4, 8, 24 or 32

    uint32_t biCompression;  // Compression type.
    uint32_t biSizeImage;    // Image size in bytes. 0 if no compression
    int32_t biXPelsPerMeter; // Horizontal resolution in pixels per meter
    int32_t biYPelsPerMeter; // Vertical resolution in pixels per meter
    uint32_t biClrUsed;      // Number of colors index in the color table
    uint32_t biClrImportant; // Number of important colors
} BITMAPINFOHEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct BITMAPV4HEADER {
    BITMAPINFOHEADER biHeader;
    uint32_t bV4RedMask;       // Red channel bit mask
    uint32_t bV4GreenMask;     // Green channel bit mask
    uint32_t bV4BlueMask;      // Blue channel bit mask
    uint32_t bV4AlphaMask;     // Alpha channel bit mask
    uint32_t bV4CSType;        // Color space type
    CIEXYZTRIPLE bV4Endpoints; // CIEXYZTRIPLE structure that specifies the x, y, and z coordinates of the three colors that correspond to the red, green, and blue endpoints for the logical color space associated with the bitmap.
    uint32_t bV4GammaRed;      // Red channel gamma value
    uint32_t bV4GammaGreen;    // Green channel gamma value
    uint32_t bV4GammaBlue;     // Blue channel gamma value
} BITMAPV4HEADER;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct BITMAPV5HEADER {
    BITMAPV4HEADER bV4Header;
    uint32_t bV5Intent;      // Rendering intent for bitmap
    uint32_t bV5ProfileData; // Offset to the start of the profile data
    uint32_t bV5ProfileSize; // Size, in bytes, of embedded profile data
    uint32_t bV5Reserved;    // Reserved. Must be zero
} BITMAPV5HEADER;
#pragma pack(pop)

enum BitmapCompression {
    BI_RGB = 0,           // No compression
    BI_RLE8 = 1,          // RLE 8-bit/pixel
    BI_RLE4 = 2,          // RLE 4-bit/pixel
    BI_BITFIELDS = 3,     // Uncompressed with RGB masks
    BI_JPEG = 4,          // JPEG or RLE-24
    BI_PNG = 5,           // PNG
};

enum BitmapColorSpaceType {
    LCS_CALIBRATED_RGB = 0, // Calibrated RGB
};

int main(int argc, char *argv[]);

#endif // BMP_H
