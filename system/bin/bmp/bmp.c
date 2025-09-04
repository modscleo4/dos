#include "bmp.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <bmp file>\n", argv[0]);
        return 1;
    }

    int fb0_fd = open("/dev/fb0", O_WRONLY);
    if (fb0_fd < 0) {
        printf("failed to open /dev/fb0: %d\n", fb0_fd);
        return 1;
    }

    int bitmap_fd = open(argv[1], O_RDONLY);
    if (bitmap_fd < 0) {
        printf("failed to open %s: %d\n", argv[1], bitmap_fd);
        return 1;
    }

    BITMAPFILEHEADER bmfh;
    read(bitmap_fd, &bmfh, sizeof(BITMAPFILEHEADER));
    if (bmfh.bfType[0] != 'B' || bmfh.bfType[1] != 'M') {
        printf("invalid BMP file\n");
        return 1;
    }

    printf("File type: %c%c\n", bmfh.bfType[0], bmfh.bfType[1]);
    printf("File size: %u\n", bmfh.bfSize);
    printf("RGBQuad offset: %u\n", bmfh.bfOffBits);

    BITMAPINFOHEADER bmih;
    read(bitmap_fd, &bmih, sizeof(BITMAPINFOHEADER));
    printf("Header size: %u\n", bmih.biSize);
    printf("Width: %d\n", bmih.biWidth);
    printf("Height: %d\n", bmih.biHeight);
    printf("Planes: %d\n", bmih.biPlanes);
    printf("Bit count: %d\n", bmih.biBitCount);
    printf("Compression: %u\n", bmih.biCompression);
    printf("Image size: %u\n", bmih.biSizeImage);

    // Skip to RGBQuad
    lseek(bitmap_fd, 0, SEEK_SET);
    uint8_t *data = malloc(bmfh.bfOffBits + bmih.biSizeImage);
    read(bitmap_fd, data, bmih.biSizeImage);

    RGBQUAD *quad = malloc(sizeof(RGBQUAD) * bmih.biWidth);
    for (int y = 0; y < bmih.biHeight; y++) {
        if (bmih.biBitCount == 24) {
            // 24-bit BMP
            int row_width = bmih.biWidth * 3;
            uint8_t *row = data + bmfh.bfOffBits + (bmih.biHeight - y - 1) * (row_width + (row_width % 4 == 0 ? 0 : 4 - (row_width % 4)));

            for (int x = 0; x < bmih.biWidth; x++) {
                quad[x].rgbBlue  = row[x * 3];
                quad[x].rgbGreen = row[x * 3 + 1];
                quad[x].rgbRed   = row[x * 3 + 2];
                quad[x].rgbReserved = 0;
            }
        }

        write(fb0_fd, quad, sizeof(RGBQUAD) * bmih.biWidth);
        lseek(fb0_fd, y * 800 * 32 / 8 + 0, SEEK_SET);
    }

    close(bitmap_fd);
    close(fb0_fd);

    return 0;
}
