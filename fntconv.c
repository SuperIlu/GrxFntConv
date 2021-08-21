/**
 ** fntconv.c ---- convert TTF to GRX font
 **
 ** Copyright (C) 2021 Andre Seidelt <superilu@yahoo.com>
 **
 ** This program is based on the GRX graphics library.
 **
 ** The GRX graphics library is free software; you can redistribute it
 ** and/or modify it under some conditions; see the "copying.grx" file
 ** for details.
 **
 ** This library is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 **
 **/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <math.h>

#include "grxdebug.h"
#include "libgrx.h"
#include "grfontdv.h"
#include "libschrift/schrift.h"
#include "bdf.h"

char *_GR_debug_file;
int _GR_debug_line;
const char *_GR_debug_function;

#define MIN(x, y) ((x < y) ? (x) : (y))
#define MAX(x, y) ((x > y) ? (x) : (y))

#define MIN_CHAR 32
#define MAX_CHAR 255

#define OUT_GRX 0
#define OUT_C 1
#define OUT_FNA 2

void _GR_debug_printf(char *fmt, ...) {
#ifdef DEBUG
    va_list ap;
    printf("%s|%s|%d: ", _GR_debug_file, _GR_debug_function, _GR_debug_line);
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
#endif
}

static SFT sft;

int ch_width(int ch) {
    GRX_ENTER();
    SFT_GMetrics gmetrics;
    SFT_Glyph glyph;
    if (sft_lookup(&sft, ch, &glyph) != 0) {
        printf("Lookup error: %d\n", ch);
        exit(1);
    }
    if (sft_gmetrics(&sft, glyph, &gmetrics) != 0) {
        printf("Metrics error: %d\n", ch);
        exit(1);
    }
    DBGPRINTF(DBG_FONT, ("charwdt(%d)=%dx%d, %f\n", ch, gmetrics.minWidth, gmetrics.minHeight, gmetrics.advanceWidth));
    if (gmetrics.advanceWidth > 0) {
        GRX_RETURN(ceil(gmetrics.advanceWidth));
    } else {
        GRX_RETURN(-1);
    }
}

int ch_bitmap(int ch, int w, int h, char *buffer) {
    int res = FALSE;
    GRX_ENTER();
    DBGPRINTF(DBG_FONT, ("bitmap(%d, %d, %d)\n", ch, w, h));

    SFT_GMetrics gmetrics;
    SFT_Glyph glyph;

    if (sft_lookup(&sft, ch, &glyph) != 0) {
        printf("Lookup error\n");
        exit(1);
    }
    if (sft_gmetrics(&sft, glyph, &gmetrics) != 0) {
        printf("Metrics error\n");
        exit(1);
    }

    if (gmetrics.advanceWidth > w) {
        printf("WARN: advanceWidth > width (%f, %d) for %d, '%c'\n", gmetrics.advanceWidth, w, ch, (char)ch);
    }

    if (gmetrics.minHeight > h) {
        printf("WARN: gmetrics.minHeight > h (%d, %d) for %d, '%c'\n", gmetrics.minHeight, h, ch, (char)ch);
    }

#ifdef DEBUG
    printf("advanceWidth    : %f\n", gmetrics.advanceWidth);
    printf("leftSideBearing : %f\n", gmetrics.leftSideBearing);
    printf("yOffset         : %d\n", gmetrics.yOffset);
    printf("minWidth        : %d\n", gmetrics.minWidth);
    printf("minHeight       : %d\n", gmetrics.minHeight);
    printf("glyph(%d, %d)=%c, ", gmetrics.minWidth, gmetrics.minHeight, (char)ch);
#endif

    // render glyph to temporary image
    char pixels[500 * 500];
    SFT_Image sft_image;
    sft_image.width = 100;
    sft_image.height = 100;
    sft_image.pixels = pixels;
    if (sft_render(&sft, glyph, sft_image) != 0) {
        printf("Render error\n");
        exit(1);
    }

    // clear buffer
    int bytes = (w - 1) / 8 + 1;
    memset(buffer, '\0', bytes * h);

    int start_line = (h + gmetrics.yOffset) - 1;
    if (start_line < 0) {
        printf("WARN: start_line out of range for %d, '%c': %d\n", ch, (char)ch, start_line);
        start_line = 0;
    }
    buffer += bytes * start_line;

#ifdef DEBUG
    printf("bytes(%d, %d)\n", bytes, start_line);
#endif

    // convert image from bytes to bits
    for (int y = 0; y < h && y < sft_image.height; y++) {
        for (int x = 0; x < w && x < sft_image.width; x++) {
            if (pixels[x + y * sft_image.width] != 0) {
                buffer[x >> 3] |= 1 << (7 - (x & 7));
            }
#ifdef DEBUG
            if (pixels[x + y * sft_image.width] != 0) {
                printf("*");
            } else {
                printf(".");
            }
#endif
        }
        buffer += bytes;
#ifdef DEBUG
        printf("\n");
#endif
    }
    res = TRUE;

    GRX_RETURN(res);
}

static void usage(char *prgname) {
    printf("Usage:\n");
    printf("  %s [-cf] <TTF or BDF file> <size> <outfile>\n", prgname);
}

int main(int argc, char *argv[]) {
    int opt;
    char *name = argv[1];
    char baseName[1024];  // dirty, dirty me!
    int size = 0;
    char *outfile = argv[3];
    int out_format = OUT_GRX;
    GrFontHeader h;
    GrFont *fnt;

    // check parameters
    while ((opt = getopt(argc, argv, "cf")) != -1) {
        switch (opt) {
            case 'c':
                out_format = OUT_C;
                break;
            case 'f':
                out_format = OUT_FNA;
                break;
            default: /* '?' */
                usage(argv[0]);
                exit(1);
        }
    }

    if (optind + 3 > argc) {
        usage(argv[0]);
        exit(1);
    }

    // create in/outfile/size/basename
    name = argv[optind];
    size = atoi(argv[optind + 1]);
    outfile = argv[optind + 2];

    int pos = 0;
    while (name[pos] && name[pos] != '.') {
        baseName[pos] = name[pos];
        pos++;
    }
    baseName[pos] = 0x00;

    if (size <= 0) {
        printf("Invalid size: %d\n", size);
        exit(1);
    }

    // init SFT
    sft.xScale = size;
    sft.yScale = size;
    sft.xOffset = 0;
    sft.yOffset = 0;
    sft.flags = SFT_DOWNWARD_Y;

    // load TTF
    sft.font = sft_loadfile(name);
    if (sft.font) {
        printf("TTF detected\n");
        printf("Loaded %s:\n", baseName);

        // extract line metrics
        SFT_LMetrics lmetrics;
        if (sft_lmetrics(&sft, &lmetrics) != 0) {
            printf("Line metrics error\n");
            exit(1);
        }

        printf("  ascender  : %f\n", lmetrics.ascender);
        printf("  descender : %f\n", lmetrics.descender);
        printf("  lineGap   : %f\n", lmetrics.lineGap);

        // extract character min/max width
        int minWidth = 0xFFFF;
        int maxWidth = 0x0000;
        for (int ch = MIN_CHAR; ch <= MAX_CHAR; ch++) {
            SFT_GMetrics gmetrics;
            SFT_Glyph glyph;
            if (sft_lookup(&sft, ch, &glyph) != 0) {
                printf("Lookup error: %d\n", ch);
                exit(1);
            }
            if (sft_gmetrics(&sft, glyph, &gmetrics) != 0) {
                printf("Metrics error: %d\n", ch);
                exit(1);
            }

            // get character sizes
            minWidth = MIN(minWidth, (int)gmetrics.advanceWidth);
            maxWidth = MAX(maxWidth, (int)gmetrics.advanceWidth);
        }

        printf("  minWidth : %d\n", minWidth);
        printf("  maxWidth : %d\n", maxWidth);

        // assemble header
        h.name = name;
        h.family = baseName;
        h.proportional = minWidth != maxWidth;
        h.scalable = TRUE;
        h.preloaded = FALSE;
        h.modified = GR_FONTCVT_NONE;
        h.width = (minWidth + maxWidth) / 2;
        h.height = ceil(lmetrics.ascender + lmetrics.lineGap);
        h.baseline = ceil(lmetrics.ascender);
        h.ulpos = round(lmetrics.ascender + lmetrics.descender);
        h.ulheight = 1;
        h.minchar = MIN_CHAR;
        h.numchars = MAX_CHAR - MIN_CHAR;
        h.encoding = GR_FONTENC_UNICODE;

        // create GrFont
        fnt = _GrBuildFont(&h, GR_FONTCVT_NONE, 0, 0, 0, 0, ch_width, ch_bitmap, FALSE);
    } else if (bdf_read_data(name, &h, size)) {
        printf("BDF detected\n");

        // create GrFont
        fnt = _GrBuildFont(&h, GR_FONTCVT_NONE, 0, 0, 0, 0, bdf_width, bdf_bitmap, FALSE);
    } else {
        printf("font building error\n");
        exit(1);
    }

    int res;
    if (out_format == OUT_GRX) {
        res = GrDumpGrxFont(fnt, outfile);
    } else if (out_format == OUT_C) {
        res = GrDumpFont(fnt, "fontdef", outfile);
    } else if (out_format == OUT_FNA) {
        res = GrDumpFnaFont(fnt, outfile);
    } else {
        printf("Unknown output format %d\n", out_format);
        exit(1);
    }

    if (res) {
        printf("Wrote output to %s\n", outfile);
        printf("  width    : %d\n", h.width);
        printf("  height   : %d\n", h.height);
        printf("  baseline : %d\n", h.baseline);
        exit(0);
    } else {
        printf("Error writing to %s\n", outfile);
        exit(1);
    }
}
