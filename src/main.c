#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#include <raylib.h>

#include "qoi.h"

int main(int argc, char **argv) {
    FILE *f;
    u8 *buf;
    i64 len;
    qoi_decoder q;
    qoi_header h;
    qoi_color *pixels;
    if (argc < 2) {
        fprintf(stderr, "usage: %s PATH\n", argv[0]);
        return 1;
    }
    if (!(f = fopen(argv[1], "r"))) {
        fprintf(stderr, "failed to open file: %s\n", argv[1]);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = calloc((size_t) len, sizeof(u8));
    fread(buf, sizeof(u8), (size_t) len, f);
    qoi_decoder_new(&q, buf, len);
    if (!qoi_decode_header(&q, &h)) {
        fprintf(stderr, "failed to decode header\n");
        return 1;
    }
    pixels = calloc(h.width * h.height, sizeof(qoi_color));
    qoi_decode(&q, &h, pixels);

    InitWindow(800, 450, "qoi test");
    {
        Image im;
        Texture2D tex;
        im.data = pixels;
        im.width = (int) h.width;
        im.height = (int) h.height;
        im.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
        im.mipmaps = 1;
        tex = LoadTextureFromImage(im);
        while (!WindowShouldClose()) {
            BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("hi", 0, 0, 100, BLACK);
            DrawTexture(tex, 10, 10, WHITE);
            EndDrawing();
        }
        CloseWindow();
    }

    return 0;
}
