#ifndef BMP_H_
#define BMP_H_

#include <stddef.h>

typedef struct _BITMAPINFOHEADER {
    int width;
    int height;
    short color_planes;
    short bit_per_pixel;
    int compression_method;
    int image_size;
    int horizontal_res;
    int vertical_res;
    int color_count;
    int important_colors;
} __attribute__((packed)) BITMAPINFOHEADER;

typedef struct _BMPHeader {
    char magic[2];
    int filesize;
    short reserved_1;
    short reserved_2;
    int pixel_offset;
    int dib_header_size;
} __attribute__((packed)) BMPHeader;

typedef struct _ColourEntry {
    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha_maybe;
} ColourEntry;

#endif // BMP_H_