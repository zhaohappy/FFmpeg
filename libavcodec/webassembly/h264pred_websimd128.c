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

#include <wasm_simd128.h>
#include "h264pred_webassembly.h"

void ff_h264_intra_pred_vert_8x8_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i;
    v128_t a = wasm_v128_load64_zero(src - stride);
    for (i = 0; i < 8; i++) {
        wasm_v128_store64_lane(src + i * stride, a, 0);
    }
}

void ff_h264_intra_pred_horiz_8x8_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i;
    for (i = 0; i < 8; i++) {
        v128_t a = wasm_u32x4_splat((src[-1 + i * stride]) * 0x01010101U);
        wasm_v128_store64_lane(src + i * stride, a, 0);
    }
}

void ff_h264_intra_predict_plane_8x8_websimd128(uint8_t *src, ptrdiff_t stride) {
    int j, k;
    int a;

    const uint8_t *const src0 = src + 3 - stride;
    const uint8_t *src1 = src + 4 * stride - 1;
    const uint8_t *src2 = src1 - 2 * stride;
    int H = src0[1] - src0[-1];
    int V = src1[0] - src2[0];
    for (k = 2; k <= 4; ++k)
    {
        src1 += stride;
        src2 -= stride;
        H += k * (src0[k] - src0[-k]);
        V += k * (src1[0] - src2[0]);
    }
    H = (17 * H + 16) >> 5;
    V = (17 * V + 16) >> 5;

    a = 16 * (src1[0] + src2[8] + 1) - 3 * (V + H);

    __i16x8 multiplier = {
        0x0000,
        0x0001,
        0x0002,
        0x0003,
        0x0004,
        0x0005,
        0x0006,
        0x0007,
    };
    __i16x8 h_vector = ((__i16x8)wasm_u16x8_splat(H) * (__i16x8)multiplier);

    __i16x8 a_vector = (__i16x8)wasm_u16x8_splat(a);
    __i16x8 v_vector = (__i16x8)wasm_u16x8_splat(V);

    for (j = 0; j < 4; j++) {
        __i16x8 sum0 = (a_vector + h_vector) >> 5;
        __i16x8 sum1 = (a_vector + v_vector + h_vector) >> 5;
        v128_t sum = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum1);
        wasm_v128_store64_lane(src, sum, 0);
        wasm_v128_store64_lane(src + stride, sum, 1);
        src += 2 * stride;
        a_vector += 2 * v_vector;
    }
}

void ff_h264_intra_predict_dc_4blk_8x8_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i;
    int dc0, dc1, dc2;
    v128_t dc0splat;

    dc0 = dc1 = dc2 = 0;
    for (i = 0; i < 4; i++) {
        dc0 += src[-1 + i * stride] + src[i - stride];
        dc1 += src[4 + i - stride];
        dc2 += src[-1 + (i + 4) * stride];
    }
    dc0splat = wasm_u32x4_make(
        (((dc0 + 4) >> 3) * 0x01010101U),
        (((dc1 + 2) >> 2) * 0x01010101U),
        (((dc2 + 2) >> 2) * 0x01010101U),
        (((dc1 + dc2 + 4) >> 3) * 0x01010101U)
    );
    for (i = 0; i < 4; i++) {
        wasm_v128_store64_lane(src, dc0splat, 0);
        src += stride;
    }
    for (i = 4; i < 8; i++) {
        wasm_v128_store64_lane(src, dc0splat, 1);
        src += stride;
    }
}

void ff_h264_intra_predict_hor_dc_8x8_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i;
    int dc0, dc2;
    v128_t dc0splat, dc2splat;

    dc0 = dc2 = 0;
    for (i = 0; i < 4; i++) {
        dc0 += src[-1 + i * stride];
        dc2 += src[-1 + (i + 4) * stride];
    }
    dc0splat = wasm_u32x4_splat(((dc0 + 2) >> 2) * 0x01010101U);
    dc2splat = wasm_u32x4_splat(((dc2 + 2) >> 2) * 0x01010101U);

    for (i = 0; i < 4; i++) {
        wasm_v128_store64_lane(src, dc0splat, 0);
        src += stride;
    }
    for (i = 4; i < 8; i++) {
        wasm_v128_store64_lane(src, dc2splat, 0);
        src += stride;
    }
}

void ff_h264_intra_predict_vert_dc_8x8_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i;
    int dc0, dc1;
    v128_t dc0splat;

    dc0 = dc1 = 0;
    for (i = 0; i < 4; i++) {
        dc0 += src[i - stride];
        dc1 += src[4 + i - stride];
    }
    dc0splat = wasm_u32x4_make(
        (((dc0 + 2) >> 2) * 0x01010101U),
        (((dc1 + 2) >> 2) * 0x01010101U),
        0,
        0
    );
    for (i = 0; i < 8; i++) {
        wasm_v128_store64_lane(src, dc0splat, 0);
        src += stride;
    }
}

static void pred4x4_dc_8_websimd128(uint8_t *src, const uint8_t *topright, ptrdiff_t stride) {
    const int dc = (src[-stride] + src[1 - stride] + src[2 - stride] + src[3 - stride] + src[-1 + 0 * stride] + src[-1 + 1 * stride] + src[-1 + 2 * stride] + src[-1 + 3 * stride] + 4) >> 3;
    const v128_t a = wasm_u32x4_splat(((dc)*0x01010101U));

    wasm_v128_store32_lane(src + 0 * stride, a, 0);
    wasm_v128_store32_lane(src + 1 * stride, a, 0);
    wasm_v128_store32_lane(src + 2 * stride, a, 0);
    wasm_v128_store32_lane(src + 3 * stride, a, 0);
}

void ff_h264_intra_predict_mad_cow_dc_l0t_8x8_websimd128(uint8_t *src, ptrdiff_t stride) {
    ff_h264_intra_predict_vert_dc_8x8_websimd128(src, stride);
    pred4x4_dc_8_websimd128(src, ((void *)0), stride);
}

static void pred8x8_dc_8_wensimd128(uint8_t *src, ptrdiff_t stride) {
    int i;
    int dc0, dc1, dc2;
    v128_t dc0splat, dc1splat;

    dc0 = dc1 = dc2 = 0;
    for (i = 0; i < 4; i++) {
        dc0 += src[-1 + i * stride] + src[i - stride];
        dc1 += src[4 + i - stride];
        dc2 += src[-1 + (i + 4) * stride];
    }
    dc0splat = wasm_u32x4_make(
        (((dc0 + 4) >> 3) * 0x01010101U),
        (((dc1 + 2) >> 2) * 0x01010101U),
        0,
        0
    );
    dc1splat = wasm_u32x4_make(
        (((dc2 + 2) >> 2) * 0x01010101U),
        (((dc1 + dc2 + 4) >> 3) * 0x01010101U),
        0,
        0
    );

    for (i = 0; i < 4; i++) {
        wasm_v128_store64_lane(src, dc0splat, 0);
        src += stride;
    }
    for (i = 4; i < 8; i++) {
        wasm_v128_store64_lane(src, dc1splat, 0);
        src += stride;
    }
}

static void pred4x4_top_dc_8_websimd128(uint8_t *src, const uint8_t *topright, ptrdiff_t stride) {
    const int dc = (src[-stride] + src[1 - stride] + src[2 - stride] + src[3 - stride] + 2) >> 2;
    const v128_t a = wasm_u32x4_splat(((dc)*0x01010101U));
    wasm_v128_store32_lane(src + 0 * stride, a, 0);
    wasm_v128_store32_lane(src + 1 * stride, a, 0);
    wasm_v128_store32_lane(src + 2 * stride, a, 0);
    wasm_v128_store32_lane(src + 3 * stride, a, 0);
}

void ff_h264_intra_predict_mad_cow_dc_0lt_8x8_websimd128(uint8_t *src, ptrdiff_t stride) {
    pred8x8_dc_8_wensimd128(src, stride);
    pred4x4_top_dc_8_websimd128(src, ((void *)0), stride);
}

static void pred4x4_128_dc_8_websimd128(uint8_t *src, const uint8_t *topright, ptrdiff_t stride) {
    const v128_t a = wasm_u32x4_splat((1 << (8 - 1)) * 0x01010101U);
    wasm_v128_store32_lane(src + 0 * stride, a, 0);
    wasm_v128_store32_lane(src + 1 * stride, a, 0);
    wasm_v128_store32_lane(src + 2 * stride, a, 0);
    wasm_v128_store32_lane(src + 3 * stride, a, 0);
}

void ff_h264_intra_predict_mad_cow_dc_l00_8x8_websimd128(uint8_t *src, ptrdiff_t stride) {
    ff_h264_intra_predict_hor_dc_8x8_websimd128(src, stride);
    pred4x4_128_dc_8_websimd128(src + 4 * stride, ((void *)0), stride);
    pred4x4_128_dc_8_websimd128(src + 4 * stride + 4 * sizeof(uint8_t), ((void *)0), stride);
}

void ff_h264_intra_predict_mad_cow_dc_0l0_8x8_websimd128(uint8_t *src, ptrdiff_t stride) {
    ff_h264_intra_predict_hor_dc_8x8_websimd128(src, stride);
    pred4x4_128_dc_8_websimd128(src, ((void *)0), stride);
    pred4x4_128_dc_8_websimd128(src + 4 * sizeof(uint8_t), ((void *)0), stride);
}

void ff_h264_intra_pred_dc_128_8x8_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i;
    const v128_t a = wasm_u32x4_splat(((1 << (8 - 1)) + 0) * 0x01010101U);
    for (i = 0; i < 8; i++) {
        wasm_v128_store64_lane(src + i * stride, a, 0);
    }
}

void ff_h264_intra_pred_dc_16x16_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i, dc = 0;
    v128_t dcsplat;

    for (i = 0; i < 16; i++) {
        dc += src[-1 + i * stride];
    }

    for (i = 0; i < 16; i++) {
        dc += src[i - stride];
    }

    dcsplat = wasm_u32x4_splat(((dc + 16) >> 5) * 0x01010101U);
    for (i = 0; i < 16; i++) {
        wasm_v128_store(src, dcsplat);
        src += stride;
    };
}

void ff_h264_intra_pred_vert_16x16_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i;
    v128_t d = wasm_v128_load(src - stride);
    for (i = 0; i < 16; i++) {
        wasm_v128_store(src, d);
        src += stride;
    }
}

void ff_h264_intra_pred_horiz_16x16_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i;
    for (i = 0; i < 16; i++) {
        const v128_t a = wasm_u32x4_splat((src[-1]) * 0x01010101U);
        wasm_v128_store(src, a);
        src += stride;
    }
}

void ff_h264_intra_predict_plane_16x16_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i, j, k;
    int a;

    const uint8_t *const src0 = src + 7 - stride;
    const uint8_t *src1 = src + 8 * stride - 1;
    const uint8_t *src2 = src1 - 2 * stride;
    int H = src0[1] - src0[-1];
    int V = src1[0] - src2[0];
    for (k = 2; k <= 8; ++k) {
        src1 += stride;
        src2 -= stride;
        H += k * (src0[k] - src0[-k]);
        V += k * (src1[0] - src2[0]);
    }
    
    H = (5 * H + 32) >> 6;
    V = (5 * V + 32) >> 6;

    a = 16 * (src1[0] + src2[16] + 1) - 7 * (V + H);

    __i16x8 multiplier_low = {
        0x0000,
        0x0001,
        0x0002,
        0x0003,
        0x0004,
        0x0005,
        0x0006,
        0x0007,
    };
    __i16x8 multiplier_high = {
        0x0008,
        0x0009,
        0x000a,
        0x000b,
        0x000c,
        0x000d,
        0x000e,
        0x000f,
    };

    __i16x8 h_vector_low = (__i16x8)wasm_u16x8_splat(H) * (__i16x8)multiplier_low;
    __i16x8 h_vector_high = (__i16x8)wasm_u16x8_splat(H) * (__i16x8)multiplier_high;

    for (j = 16; j > 0; --j) {
        __i16x8 b_vector = (__i16x8)wasm_i16x8_splat(a);
        a += V;
        __i16x8 sum0 = (b_vector + h_vector_low) >> 5;
        __i16x8 sum1 = (b_vector + h_vector_high) >> 5;
        v128_t sum = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum1);
        wasm_v128_store(src, sum);
        src += stride;
    }
}

void ff_h264_intra_pred_dc_left_16x16_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i, dc = 0;
    v128_t dcsplat;

    for (i = 0; i < 16; i++) {
        dc += src[-1 + i * stride];
    }

    dcsplat = wasm_u32x4_splat(((dc + 8) >> 4) * 0x01010101U);
    for (i = 0; i < 16; i++) {
        wasm_v128_store(src, dcsplat);
        src += stride;
    };
}

void ff_h264_intra_pred_dc_top_16x16_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i, dc = 0;
    v128_t dcsplat;

    for (i = 0; i < 16; i++) {
        dc += src[i - stride];
    }

    dcsplat = wasm_u32x4_splat(((dc + 8) >> 4) * 0x01010101U);
    for (i = 0; i < 16; i++) {
        wasm_v128_store(src, dcsplat);
        src += stride;
    };
}

void ff_h264_intra_pred_dc_128_16x16_websimd128(uint8_t *src, ptrdiff_t stride) {
    int i;

    v128_t dcsplat = wasm_u32x4_splat(((1 << (8 - 1)) + 0) * 0x01010101U);

    for (i = 0; i < 16; i++) {
        wasm_v128_store(src, dcsplat);
        src += stride;
    };
}