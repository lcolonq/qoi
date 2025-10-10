#ifndef QOI_H
#define QOI_H

#include "types.h"

typedef struct {
    u8 r, g, b, a;
} qoi_color;

typedef enum {
    QOI_CHANNELS_RGB = 3,
    QOI_CHANNELS_RGBA = 4
} qoi_channels;

typedef enum {
    QOI_COLORSPACE_SRGB = 0,
    QOI_COLORSPACE_LINEAR = 1
} qoi_colorspace;

typedef struct {
    u8 *buf;
    i64 len;
    i64 cur;
    qoi_color palette[64];
} qoi_decoder;

typedef struct {
    u32 width, height;
    u8 channels, colorspace;
} qoi_header;

static qoi_color qoi_color_new(u8 r, u8 g, u8 b, u8 a) {
    qoi_color ret;
    ret.r = r; ret.g = g; ret.b = b; ret.a = a;
    return ret;
}
static u8 qoi_color_hash(qoi_color c) {
    return (u8) (c.r * 3 + c.g * 5 + c.b * 7 + c.a * 11) % 64;
}

void qoi_decoder_new(qoi_decoder *q, u8 *buf, i64 len);
bool qoi_decode_header(qoi_decoder *q, qoi_header *h);
bool qoi_decode(qoi_decoder *q, qoi_header *h, qoi_color *pixels);

#endif
