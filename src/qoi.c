#include "qoi.h"

static u32 read_u32_be(qoi_decoder *q) {
    u32 ret = 0;
    if (q->cur + 4 >= q->len) return 0xffffffff;
    ret |= q->buf[q->cur++] << 24;
    ret |= q->buf[q->cur++] << 16;
    ret |= q->buf[q->cur++] << 8;
    ret |= q->buf[q->cur++];
    return ret;
}

typedef enum {
    QOI_OP_INVALID = -3,
    QOI_OP_RGB = -2,
    QOI_OP_RGBA = -1,
    QOI_OP_INDEX = 0,
    QOI_OP_DIFF = 1,
    QOI_OP_LUMA = 2,
    QOI_OP_RUN = 3
} qoi_op;
static qoi_op cur_op(qoi_decoder *q) {
    u8 v;
    if (q->cur >= q->len) return QOI_OP_INVALID;
    v = q->buf[q->cur];
    if ((i8) v == QOI_OP_RGB) return QOI_OP_RGB;
    if ((i8) v == QOI_OP_RGBA) return QOI_OP_RGBA;
    return v >> 6;
}

void qoi_decoder_new(qoi_decoder *q, u8 *buf, i64 len) {
    i64 i;
    q->buf = buf;
    q->len = len;
    q->cur = 0;
    for (i = 0; i < 64; ++i) q->palette[i] = qoi_color_new(0, 0, 0, 0);
}

bool qoi_decode_header(qoi_decoder *q, qoi_header *h) {
    if (q->cur + 14 > q->len) return false;
    if (read_u32_be(q) != ('q' << 24 | 'o' << 16 | 'i' << 8 | 'f')) {
        return false;
    }
    h->width = read_u32_be(q);
    h->height = read_u32_be(q);
    h->channels = q->buf[q->cur++];
    h->colorspace = q->buf[q->cur++];
    return true;
}

bool qoi_decode(qoi_decoder *q, qoi_header *h, qoi_color *pixels) {
    i64 len = h->width * h->height;
    i64 pidx = 0;
    qoi_color c = qoi_color_new(0, 0, 0, 255);
    u8 v = 0, w = 0;
    u8 padding[8] = {0, 0, 0, 0, 0, 0, 0, 1};
    i64 i;
    for (i = 0; i < 8; ++i) if (padding[i] != q->buf[q->len - 8 + i]) return false;
    while (q->cur < q->len - 8 && pidx < len) {
        /* we never need to range-check q->cur mid-loop, since we know the padding exists */
        switch (cur_op(q)) {
        case QOI_OP_INVALID: return false;
        case QOI_OP_RGB:
            q->cur++;
            c.r = q->buf[q->cur++];
            c.g = q->buf[q->cur++];
            c.b = q->buf[q->cur++];
            pixels[pidx++] = c;
            break;
        case QOI_OP_RGBA:
            q->cur++;
            c.r = q->buf[q->cur++];
            c.g = q->buf[q->cur++];
            c.b = q->buf[q->cur++];
            c.a = q->buf[q->cur++];
            pixels[pidx++] = c;
            break;
        case QOI_OP_INDEX:
            v = q->buf[q->cur++] & 0x3f /* 0b111111 */;
            c = q->palette[v];
            pixels[pidx++] = c;
            break;
        case QOI_OP_DIFF: {
            v = q->buf[q->cur++];
            c.r += (u8) ((v >> 4 & 0x3) - 2);
            c.g += (u8) ((v >> 2 & 0x3) - 2);
            c.b += (u8) ((v & 0x3) - 2);
            pixels[pidx++] = c;
            break;
        }
        case QOI_OP_LUMA: {
            v = (q->buf[q->cur++] & 0x3f) - 32;
            w = q->buf[q->cur++];
            c.r += (u8) (((w >> 4) & 0xf) - 8 + v);
            c.g += v;
            c.b += (u8) ((w & 0xf) - 8 + v);
            pixels[pidx++] = c;
            break;
        }
        case QOI_OP_RUN:
            v = (q->buf[q->cur++] & 0x3f) + 1;
            for (; v > 0; v--) {
                pixels[pidx++] = c;
            }
            break;
        default: return false;
        }
        q->palette[qoi_color_hash(c)] = c;
    }
    return true;
}
