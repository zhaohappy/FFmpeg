/*
 * Copyright (c) 2022 zhaogaoxing
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "config.h"
#include "h264chroma_webassembly.h"

#if HAVE_WEBSIMD128
static av_cold void h264chroma_init_websimd128(H264ChromaContext *c, int bit_depth)
{
    const int high_bit_depth = bit_depth > 8;

    if (!high_bit_depth) {
        c->put_h264_chroma_pixels_tab[0] = ff_put_h264_chroma_mc8_8_websimd128;
        c->put_h264_chroma_pixels_tab[1] = ff_put_h264_chroma_mc4_8_websimd128;

        c->avg_h264_chroma_pixels_tab[0] = ff_avg_h264_chroma_mc8_8_websimd128;
        c->avg_h264_chroma_pixels_tab[1] = ff_avg_h264_chroma_mc4_8_websimd128;
    }
}
#endif  // #if HAVE_WEBSIMD128

void ff_h264chroma_init_webassembly(H264ChromaContext *c, int bit_depth)
{
#if HAVE_WEBSIMD128
    h264chroma_init_websimd128(c, bit_depth);
#endif /* HAVE_WEBSIMD128 */
}
