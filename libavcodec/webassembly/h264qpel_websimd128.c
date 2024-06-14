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
#include "h264qpel_webassembly.h"
#include "h264dsp_webassembly.h"

static inline void avg_pixels4_l2_8(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, int src_stride1, int src_stride2) {
    v128_t a = wasm_v128_load32_zero(src1);
    a[1] = wasm_v128_load32_zero(src1 + src_stride1)[0];
    a[2] = wasm_v128_load32_zero(src1 + 2 * src_stride1)[0];
    a[3] = wasm_v128_load32_zero(src1 + 3 * src_stride1)[0];

    v128_t b = wasm_v128_load32_zero(src2);
    b[1] = wasm_v128_load32_zero(src2 + src_stride2)[0];
    b[2] = wasm_v128_load32_zero(src2 + 2 * src_stride2)[0];
    b[3] = wasm_v128_load32_zero(src2 + 3 * src_stride2)[0];

    v128_t d = wasm_v128_load32_zero(dst);
    d[1] = wasm_v128_load32_zero(dst + dst_stride)[0];
    d[2] = wasm_v128_load32_zero(dst + 2 * dst_stride)[0];
    d[3] = wasm_v128_load32_zero(dst + 3 * dst_stride)[0];

    v128_t result = wasm_u8x16_avgr(d, wasm_u8x16_avgr(a, b));

    wasm_v128_store32_lane(dst, result, 0);
    wasm_v128_store32_lane(dst + dst_stride, result, 1);
    wasm_v128_store32_lane(dst + 2 * dst_stride, result, 2);
    wasm_v128_store32_lane(dst + 3 * dst_stride, result, 3);
}
static inline void avg_pixels8_l2_8(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, int src_stride1, int src_stride2) {
    int i;
    for (i = 0; i < 4; i++) {
        v128_t a = wasm_u8x16_narrow_i16x8(
            wasm_u16x8_load8x8(src1),
            wasm_u16x8_load8x8(src1 + src_stride1)
        );
        v128_t b = wasm_u8x16_narrow_i16x8(
            wasm_u16x8_load8x8(src2),
            wasm_u16x8_load8x8(src2 + src_stride2)
        );
        v128_t d = wasm_u8x16_narrow_i16x8(
            wasm_u16x8_load8x8(dst),
            wasm_u16x8_load8x8(dst + dst_stride)
        );

        d = wasm_u8x16_avgr(
            d,
            wasm_u8x16_avgr(a, b)
        );

        wasm_v128_store64_lane(dst, d, 0);
        wasm_v128_store64_lane(dst + dst_stride, d, 1);

        src1 += 2 * src_stride1;
        src2 += 2 * src_stride2;
        dst += 2 * dst_stride;
    }
}
static inline void avg_pixels16_l2_8(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, int src_stride1, int src_stride2) {
    int i;
    for (i = 0; i < 16; i++) {
        v128_t a = wasm_v128_load(src1);
        v128_t b = wasm_v128_load(src2);
        v128_t d = wasm_v128_load(dst);

        wasm_v128_store(dst, wasm_u8x16_avgr(
            d,
            wasm_u8x16_avgr(a, b)
        ));
        src1 += src_stride1;
        src2 += src_stride2;
        dst += dst_stride;
    }
}

static inline void put_pixels4_l2_8(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, int src_stride1, int src_stride2) {
    __u32x4 a = wasm_v128_load32_zero(src1);
    a[1] = ((__u32x4)wasm_v128_load32_zero(src1 + src_stride1))[0];
    a[2] = ((__u32x4)wasm_v128_load32_zero(src1 + 2 * src_stride1))[0];
    a[3] = ((__u32x4)wasm_v128_load32_zero(src1 + 3 * src_stride1))[0];

    __u32x4 b = wasm_v128_load32_zero(src2);
    b[1] = ((__u32x4)wasm_v128_load32_zero(src2 + src_stride2))[0];
    b[2] = ((__u32x4)wasm_v128_load32_zero(src2 + 2 * src_stride2))[0];
    b[3] = ((__u32x4)wasm_v128_load32_zero(src2 + 3 * src_stride2))[0];

    v128_t result = wasm_u8x16_avgr(a, b);

    wasm_v128_store32_lane(dst, result, 0);
    wasm_v128_store32_lane(dst + dst_stride, result, 1);
    wasm_v128_store32_lane(dst + 2 * dst_stride, result, 2);
    wasm_v128_store32_lane(dst + 3 * dst_stride, result, 3);
}
static inline void put_pixels8_l2_8(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, int src_stride1, int src_stride2) {
    int i;
    v128_t a0, b0, sum0, sum1;
    for (i = 0; i < 4; i++) {
        a0 = wasm_u16x8_load8x8(src1);
        b0 = wasm_u16x8_load8x8(src2);
        sum0 = wasm_u16x8_avgr(a0, b0);

        a0 = wasm_u16x8_load8x8(src1 + src_stride1);
        b0 = wasm_u16x8_load8x8(src2 + src_stride2);
        sum1 = wasm_u16x8_avgr(a0, b0);

        sum0 = wasm_u8x16_narrow_i16x8(sum0, sum1);

        wasm_v128_store64_lane(dst, sum0, 0);
        wasm_v128_store64_lane(dst + dst_stride, sum0, 1);

        src1 += 2 * src_stride1;
        src2 += 2 * src_stride2;
        dst += 2 * dst_stride;
    }
}
static inline void put_pixels16_l2_8(uint8_t *dst, const uint8_t *src1, const uint8_t *src2, int dst_stride, int src_stride1, int src_stride2) {
    int i;
    for (i = 0; i < 16; i++) {
        v128_t a = wasm_v128_load(src1);
        v128_t b = wasm_v128_load(src2);
        wasm_v128_store(dst, wasm_u8x16_avgr(a, b));
        src1 += src_stride1;
        src2 += src_stride2;
        dst += dst_stride;
    }
}

static inline void avg_pixels4_8_websimd128(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size) {
    v128_t b = wasm_v128_load32_zero(block);
    b[1] = wasm_v128_load32_zero(block + line_size)[0];
    b[2] = wasm_v128_load32_zero(block + 2 * line_size)[0];
    b[3] = wasm_v128_load32_zero(block + 3 * line_size)[0];

    v128_t p = wasm_v128_load32_zero(pixels);
    p[1] = wasm_v128_load32_zero(pixels + line_size)[0];
    p[2] = wasm_v128_load32_zero(pixels + 2 * line_size)[0];
    p[3] = wasm_v128_load32_zero(pixels + 3 * line_size)[0];

    v128_t result = wasm_u8x16_avgr(p, b);

    wasm_v128_store32_lane(block, result, 0);
    wasm_v128_store32_lane(block + line_size, result, 1);
    wasm_v128_store32_lane(block + 2 * line_size, result, 2);
    wasm_v128_store32_lane(block + 3 * line_size, result, 3);
}
static inline void avg_pixels8_8_websimd128(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size) {
    int i;
    for (i = 0; i < 4; i++) {
        v128_t a = wasm_u8x16_narrow_i16x8(
            wasm_u16x8_load8x8(block),
            wasm_u16x8_load8x8(block + line_size)
        );
        v128_t b = wasm_u8x16_narrow_i16x8(
            wasm_u16x8_load8x8(pixels),
            wasm_u16x8_load8x8(pixels + line_size)
        );

        v128_t res = wasm_u8x16_avgr(a, b);

        wasm_v128_store64_lane(block, res, 0);
        wasm_v128_store64_lane(block + line_size, res, 1);

        pixels += 2 * line_size;
        block += 2 * line_size;
    }
}
static inline void avg_pixels16_8_websimd128(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size) {
    int i;
    for (i = 0; i < 16; i++) {
        v128_t a = wasm_v128_load(block);
        v128_t b = wasm_v128_load(pixels);
        wasm_v128_store(block, wasm_u8x16_avgr(a, b));
        pixels += line_size;
        block += line_size;
    }
}

static inline void put_pixels4_8_websimd128(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size) {
    int i;
    for (i = 0; i < 4; i++) {
        wasm_v128_store32_lane(block, wasm_v128_load32_zero(pixels), 0);
        pixels += line_size;
        block += line_size;
    }
}
static inline void put_pixels8_8_websimd128(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size) {
    int i;
    for (i = 0; i < 8; i++) {
        wasm_v128_store64_lane(block, wasm_v128_load64_zero(pixels), 0);
        pixels += line_size;
        block += line_size;
    }
}
static inline void put_pixels16_8_websimd128(uint8_t *block, const uint8_t *pixels, ptrdiff_t line_size) {
    int i;
    for (i = 0; i < 16; i++) {
        wasm_v128_store(block, wasm_v128_load(pixels));
        pixels += line_size;
        block += line_size;
    }
}

static inline void copy_block4_8(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride, int h) {
    int i;
    for (i = 0; i < h; i++) {
        wasm_v128_store32_lane(dst, wasm_v128_load32_zero(src), 0);
        dst += dstStride;
        src += srcStride;
    }
}
static inline void copy_block8_8(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride, int h) {
    int i;
    for (i = 0; i < h; i++) {
        wasm_v128_store64_lane(dst, wasm_v128_load64_zero(src), 0);
        dst += dstStride;
        src += srcStride;
    }
}
static inline void copy_block16_8(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride, int h) {
    int i;
    for (i = 0; i < h; i++) {
        wasm_v128_store(dst, wasm_v128_load(src));
        dst += dstStride;
        src += srcStride;
    }
}

static void put_h264_qpel4_h_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    v128_t d0 = wasm_v128_load32_zero(src);
    v128_t d1 = wasm_v128_load32_zero(src + srcStride);
    __i16x8 s0 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src + 1);
    d1 = wasm_v128_load32_zero(src + srcStride + 1);
    __i16x8 s1 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src - 1);
    d1 = wasm_v128_load32_zero(src + srcStride - 1);
    __i16x8 s_1 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src + 2);
    d1 = wasm_v128_load32_zero(src + srcStride + 2);
    __i16x8 s2 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src - 2);
    d1 = wasm_v128_load32_zero(src + srcStride - 2);
    __i16x8 s_2 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );
    
    d0 = wasm_v128_load32_zero(src + 3);
    d1 = wasm_v128_load32_zero(src + srcStride + 3);
    __i16x8 s3 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    __i16x8 sum0 = ((s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3) + 16) >> 5;

    src += srcStride * 2;

    d0 = wasm_v128_load32_zero(src);
    d1 = wasm_v128_load32_zero(src + srcStride);
    s0 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src + 1);
    d1 = wasm_v128_load32_zero(src + srcStride + 1);
    s1 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src - 1);
    d1 = wasm_v128_load32_zero(src + srcStride - 1);
    s_1 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src + 2);
    d1 = wasm_v128_load32_zero(src + srcStride + 2);
    s2 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src - 2);
    d1 = wasm_v128_load32_zero(src + srcStride - 2);
    s_2 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );
    
    d0 = wasm_v128_load32_zero(src + 3);
    d1 = wasm_v128_load32_zero(src + srcStride + 3);
    s3 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    __i16x8 sum1 = ((s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3) + 16) >> 5;

    sum0 = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum1);

    wasm_v128_store32_lane(dst, (v128_t)sum0, 0);
    wasm_v128_store32_lane(dst + dstStride, (v128_t)sum0, 1);
    wasm_v128_store32_lane(dst + 2 * dstStride, (v128_t)sum0, 2);
    wasm_v128_store32_lane(dst + 3 * dstStride, (v128_t)sum0, 3);
}
static void put_h264_qpel4_v_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    __i32x4 srcB = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src - 2 * srcStride)));
    __i32x4 srcA = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src - 1 * srcStride)));
    __i32x4 src0 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src)));
    __i32x4 src1 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 1 * srcStride)));
    __i32x4 src2 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 2 * srcStride)));
    __i32x4 src3 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 3 * srcStride)));
    __i32x4 src4 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 4 * srcStride)));
    __i32x4 src5 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 5 * srcStride)));
    __i32x4 src6 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 6 * srcStride)));

    __i32x4 d0 = ((src0 + src1) * 20 - (srcA + src2) * 5 + (srcB + src3) + 16) >> 5;
    __i32x4 d1 = ((src1 + src2) * 20 - (src0 + src3) * 5 + (srcA + src4) + 16) >> 5;
    __i32x4 d2 = ((src2 + src3) * 20 - (src1 + src4) * 5 + (src0+ src5) + 16) >> 5;
    __i32x4 d3 = ((src3 + src4) * 20 - (src2 + src5) * 5 + (src1 + src6) + 16) >> 5;

    v128_t sum = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_narrow_i32x4((v128_t)d0, (v128_t)d1),
        wasm_u16x8_narrow_i32x4((v128_t)d2, (v128_t)d3)
    );

    wasm_v128_store32_lane(dst, sum, 0);
    wasm_v128_store32_lane(dst + dstStride, sum, 1);
    wasm_v128_store32_lane(dst + 2 * dstStride, sum, 2);
    wasm_v128_store32_lane(dst + 3 * dstStride, sum, 3);
}
static void put_h264_qpel4_hv_lowpass_8_websimd128(uint8_t *dst, int16_t *tmp, const uint8_t *src, int dstStride, int tmpStride, int srcStride) {
    const int h = 4;
    int i;
    src -= 2 * srcStride;

    for (i = 0; i < 4; i++) {

        v128_t d0 = wasm_v128_load32_zero(src);
        v128_t d1 = wasm_v128_load32_zero(src + srcStride);
        __i16x8 s0 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );

        d0 = wasm_v128_load32_zero(src + 1);
        d1 = wasm_v128_load32_zero(src + srcStride + 1);
        __i16x8 s1 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );

        d0 = wasm_v128_load32_zero(src - 1);
        d1 = wasm_v128_load32_zero(src + srcStride - 1);
        __i16x8 s_1 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );

        d0 = wasm_v128_load32_zero(src + 2);
        d1 = wasm_v128_load32_zero(src + srcStride + 2);
        __i16x8 s2 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );

        d0 = wasm_v128_load32_zero(src - 2);
        d1 = wasm_v128_load32_zero(src + srcStride - 2);
        __i16x8 s_2 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );
        
        d0 = wasm_v128_load32_zero(src + 3);
        d1 = wasm_v128_load32_zero(src + srcStride + 3);
        __i16x8 s3 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );

        v128_t sum = (v128_t)(((s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3)));

        wasm_v128_store64_lane(tmp, sum, 0);
        wasm_v128_store64_lane(tmp + tmpStride, sum, 1);

        tmp += tmpStride * 2;
        src += srcStride * 2;
    }

    tmp[0] = (src[0] + src[1]) * 20 - (src[-1] + src[2]) * 5 + (src[-2] + src[3]);
    tmp[1] = (src[1] + src[2]) * 20 - (src[0] + src[3]) * 5 + (src[-1] + src[4]);
    tmp[2] = (src[2] + src[3]) * 20 - (src[1] + src[4]) * 5 + (src[0] + src[5]);
    tmp[3] = (src[3] + src[4]) * 20 - (src[2] + src[5]) * 5 + (src[1] + src[6]);

    tmp -= tmpStride * 6;

    __i32x4 srcB = wasm_u32x4_load16x4(tmp - 2 * tmpStride);
    __i32x4 srcA = wasm_u32x4_load16x4(tmp - 1 * tmpStride);
    __i32x4 src0 = wasm_u32x4_load16x4(tmp);
    __i32x4 src1 = wasm_u32x4_load16x4(tmp + 1 * tmpStride);
    __i32x4 src2 = wasm_u32x4_load16x4(tmp + 2 * tmpStride);
    __i32x4 src3 = wasm_u32x4_load16x4(tmp + 3 * tmpStride);
    __i32x4 src4 = wasm_u32x4_load16x4(tmp + 4 * tmpStride);
    __i32x4 src5 = wasm_u32x4_load16x4(tmp + 5 * tmpStride);
    __i32x4 src6 = wasm_u32x4_load16x4(tmp + 6 * tmpStride);

    __i32x4 d0 = ((src0 + src1) * 20 - (srcA + src2) * 5 + (srcB + src3) + 512) >> 10;
    __i32x4 d1 = ((src1 + src2) * 20 - (src0 + src3) * 5 + (srcA + src4) + 512) >> 10;
    __i32x4 d2 = ((src2 + src3) * 20 - (src1 + src4) * 5 + (src0+ src5) + 512) >> 10;
    __i32x4 d3 = ((src3 + src4) * 20 - (src2 + src5) * 5 + (src1 + src6) + 512) >> 10;

    v128_t sum = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_narrow_i32x4(d0, d1),
        wasm_u16x8_narrow_i32x4(d2, d3)
    );

    wasm_v128_store32_lane(dst, sum, 0);
    wasm_v128_store32_lane(dst + dstStride, sum, 1);
    wasm_v128_store32_lane(dst + 2 * dstStride, sum, 2);
    wasm_v128_store32_lane(dst + 3 * dstStride, sum, 3);
}

static void put_h264_qpel8_h_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    int i;
    for (i = 0; i < 4; i++) {

        __i16x8 s0 = (__i16x8)wasm_u16x8_load8x8(src);
        __i16x8 s1 = (__i16x8)wasm_u16x8_load8x8(src + 1);
        __i16x8 s_1 = (__i16x8)wasm_u16x8_load8x8(src - 1);
        __i16x8 s2 = (__i16x8)wasm_u16x8_load8x8(src + 2);
        __i16x8 s_2 = (__i16x8)wasm_u16x8_load8x8(src - 2);
        __i16x8 s3 = (__i16x8)wasm_u16x8_load8x8(src + 3);

        __i16x8 sum0 = ((s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3) + 16) >> 5;

        s0 = wasm_u16x8_load8x8(src + srcStride);
        s1 = wasm_u16x8_load8x8(src + 1 + srcStride);
        s_1 = wasm_u16x8_load8x8(src - 1 + srcStride);
        s2 = wasm_u16x8_load8x8(src + 2 + srcStride);
        s_2 = wasm_u16x8_load8x8(src - 2 + srcStride);
        s3 = wasm_u16x8_load8x8(src + 3 + srcStride);

        __i16x8 sum1 = ((s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3) + 16) >> 5;

        sum0 = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum1);
        
        wasm_v128_store64_lane(dst, (v128_t)sum0, 0);
        wasm_v128_store64_lane(dst + dstStride, (v128_t)sum0, 1);

        dst += 2 * dstStride;
        src += 2 * srcStride;
    }
}
static void put_h264_qpel8_v_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    __i16x8 srcB = (__i16x8)wasm_u16x8_load8x8(
        src - 2 * srcStride
    );
    __i16x8 srcA = (__i16x8)wasm_u16x8_load8x8(
        src - srcStride
    );
    __i16x8 src0 = (__i16x8)wasm_u16x8_load8x8(
        src
    );
    __i16x8 src1 = (__i16x8)wasm_u16x8_load8x8(
        src + 1 * srcStride
    );
    __i16x8 src2 = (__i16x8)wasm_u16x8_load8x8(
        src + 2 * srcStride
    );
    __i16x8 src3 = (__i16x8)wasm_u16x8_load8x8(
        src + 3 * srcStride
    );
    __i16x8 src4 = (__i16x8)wasm_u16x8_load8x8(
        src + 4 * srcStride
    );
    __i16x8 src5 = (__i16x8)wasm_u16x8_load8x8(
        src + 5 * srcStride
    );
    __i16x8 src6 = (__i16x8)wasm_u16x8_load8x8(
        src + 6 * srcStride
    );
    __i16x8 src7 = (__i16x8)wasm_u16x8_load8x8(
        src + 7 * srcStride
    );
    __i16x8 src8 = (__i16x8)wasm_u16x8_load8x8(
        src + 8 * srcStride
    );
    __i16x8 src9 = (__i16x8)wasm_u16x8_load8x8(
        src + 9 * srcStride
    );
    __i16x8 src10 = (__i16x8)wasm_u16x8_load8x8(
        src + 10 * srcStride
    );

    __i16x8 d0 = ((src0 + src1) * 20 - (srcA + src2) * 5 + (srcB + src3) + 16) >> 5;
    __i16x8 d1 = ((src1 + src2) * 20 - (src0 + src3) * 5 + (srcA + src4) + 16) >> 5;
    __i16x8 d2 = ((src2 + src3) * 20 - (src1 + src4) * 5 + (src0+ src5) + 16) >> 5;
    __i16x8 d3 = ((src3 + src4) * 20 - (src2 + src5) * 5 + (src1 + src6) + 16) >> 5;
    __i16x8 d4 = ((src4 + src5) * 20 - (src3 + src6) * 5 + (src2 + src7) + 16) >> 5;
    __i16x8 d5 = ((src5 + src6) * 20 - (src4 + src7) * 5 + (src3 + src8) + 16) >> 5;
    __i16x8 d6 = ((src6 + src7) * 20 - (src5 + src8) * 5 + (src4 + src9) + 16) >> 5;
    __i16x8 d7 = ((src7 + src8) * 20 - (src6 + src9) * 5 + (src5 + src10) + 16) >> 5;

    v128_t sum0 = wasm_u8x16_narrow_i16x8((v128_t)d0, (v128_t)d1);
    v128_t sum1 = wasm_u8x16_narrow_i16x8((v128_t)d2, (v128_t)d3);
    v128_t sum2 = wasm_u8x16_narrow_i16x8((v128_t)d4, (v128_t)d5);
    v128_t sum3 = wasm_u8x16_narrow_i16x8((v128_t)d6, (v128_t)d7);

    wasm_v128_store64_lane(dst, sum0, 0);
    wasm_v128_store64_lane(dst + dstStride, sum0, 1);
    wasm_v128_store64_lane(dst + 2 * dstStride, sum1, 0);
    wasm_v128_store64_lane(dst + 3 * dstStride, sum1, 1);
    wasm_v128_store64_lane(dst + 4 * dstStride, sum2, 0);
    wasm_v128_store64_lane(dst + 5 * dstStride, sum2, 1);
    wasm_v128_store64_lane(dst + 6 * dstStride, sum3, 0);
    wasm_v128_store64_lane(dst + 7 * dstStride, sum3, 1);
}
static void put_h264_qpel8_hv_lowpass_8_websimd128(uint8_t *dst, int16_t *tmp, const uint8_t *src, int dstStride, int tmpStride, int srcStride) {
    const int h = 8;
    int i;
    src -= 2 * srcStride;

    for (i = 0; i < h + 5; i++) {
        __i16x8 s0 = (__i16x8)wasm_u16x8_load8x8(src);
        __i16x8 s1 = (__i16x8)wasm_u16x8_load8x8(src + 1);
        __i16x8 s_1 = (__i16x8)wasm_u16x8_load8x8(src - 1);
        __i16x8 s2 = (__i16x8)wasm_u16x8_load8x8(src + 2);
        __i16x8 s_2 = (__i16x8)wasm_u16x8_load8x8(src - 2);
        __i16x8 s3 = (__i16x8)wasm_u16x8_load8x8(src + 3);

        __i16x8 sum = (((s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3)));
        wasm_v128_store(tmp, (v128_t)sum);
        
        tmp += tmpStride;
        src += srcStride;
    }
    tmp -= tmpStride * (h + 5 - 2);

    __i32x4 tmpB = (__i32x4)wasm_i32x4_load16x4(tmp -2 * tmpStride);
    __i32x4 tmpA = (__i32x4)wasm_i32x4_load16x4(tmp -1 * tmpStride);
    __i32x4 tmp0 = (__i32x4)wasm_i32x4_load16x4(tmp);
    __i32x4 tmp1 = (__i32x4)wasm_i32x4_load16x4(tmp + 1 * tmpStride);
    __i32x4 tmp2 = (__i32x4)wasm_i32x4_load16x4(tmp + 2 * tmpStride);
    __i32x4 tmp3 = (__i32x4)wasm_i32x4_load16x4(tmp + 3 * tmpStride);
    __i32x4 tmp4 = (__i32x4)wasm_i32x4_load16x4(tmp + 4 * tmpStride);
    __i32x4 tmp5 = (__i32x4)wasm_i32x4_load16x4(tmp + 5 * tmpStride);
    __i32x4 tmp6 = (__i32x4)wasm_i32x4_load16x4(tmp + 6 * tmpStride);
    __i32x4 tmp7 = (__i32x4)wasm_i32x4_load16x4(tmp + 7 * tmpStride);
    __i32x4 tmp8 = (__i32x4)wasm_i32x4_load16x4(tmp + 8 * tmpStride);
    __i32x4 tmp9 = (__i32x4)wasm_i32x4_load16x4(tmp + 9 * tmpStride);
    __i32x4 tmp10 = (__i32x4)wasm_i32x4_load16x4(tmp + 10 * tmpStride);

    __i32x4 d0 = (((tmp0 + tmp1) * 20 - (tmpA + tmp2) * 5 + (tmpB + tmp3) + 512) >> 10);
    __i32x4 d1 = (((tmp1 + tmp2) * 20 - (tmp0 + tmp3) * 5 + (tmpA + tmp4) + 512) >> 10);
    __i32x4 d2 = (((tmp2 + tmp3) * 20 - (tmp1 + tmp4) * 5 + (tmp0 + tmp5) + 512) >> 10);
    __i32x4 d3 = (((tmp3 + tmp4) * 20 - (tmp2 + tmp5) * 5 + (tmp1 + tmp6) + 512) >> 10);
    __i32x4 d4 = (((tmp4 + tmp5) * 20 - (tmp3 + tmp6) * 5 + (tmp2 + tmp7) + 512) >> 10);
    __i32x4 d5 = (((tmp5 + tmp6) * 20 - (tmp4 + tmp7) * 5 + (tmp3 + tmp8) + 512) >> 10);
    __i32x4 d6 = (((tmp6 + tmp7) * 20 - (tmp5 + tmp8) * 5 + (tmp4 + tmp9) + 512) >> 10);
    __i32x4 d7 = (((tmp7 + tmp8) * 20 - (tmp6 + tmp9) * 5 + (tmp5 + tmp10) + 512) >> 10);

    tmp += 4;
    
    tmpB = (__i32x4)wasm_i32x4_load16x4(tmp -2 * tmpStride);
    tmpA = (__i32x4)wasm_i32x4_load16x4(tmp -1 * tmpStride);
    tmp0 = (__i32x4)wasm_i32x4_load16x4(tmp);
    tmp1 = (__i32x4)wasm_i32x4_load16x4(tmp + 1 * tmpStride);
    tmp2 = (__i32x4)wasm_i32x4_load16x4(tmp + 2 * tmpStride);
    tmp3 = (__i32x4)wasm_i32x4_load16x4(tmp + 3 * tmpStride);
    tmp4 = (__i32x4)wasm_i32x4_load16x4(tmp + 4 * tmpStride);
    tmp5 = (__i32x4)wasm_i32x4_load16x4(tmp + 5 * tmpStride);
    tmp6 = (__i32x4)wasm_i32x4_load16x4(tmp + 6 * tmpStride);
    tmp7 = (__i32x4)wasm_i32x4_load16x4(tmp + 7 * tmpStride);
    tmp8 = (__i32x4)wasm_i32x4_load16x4(tmp + 8 * tmpStride);
    tmp9 = (__i32x4)wasm_i32x4_load16x4(tmp + 9 * tmpStride);
    tmp10 = (__i32x4)wasm_i32x4_load16x4(tmp + 10 * tmpStride);

    __i32x4 d8 = (((tmp0 + tmp1) * 20 - (tmpA + tmp2) * 5 + (tmpB + tmp3) + 512) >> 10);
    __i32x4 d9 = (((tmp1 + tmp2) * 20 - (tmp0 + tmp3) * 5 + (tmpA + tmp4) + 512) >> 10);
    __i32x4 d10 = (((tmp2 + tmp3) * 20 - (tmp1 + tmp4) * 5 + (tmp0 + tmp5) + 512) >> 10);
    __i32x4 d11 = (((tmp3 + tmp4) * 20 - (tmp2 + tmp5) * 5 + (tmp1 + tmp6) + 512) >> 10);
    __i32x4 d12 = (((tmp4 + tmp5) * 20 - (tmp3 + tmp6) * 5 + (tmp2 + tmp7) + 512) >> 10);
    __i32x4 d13 = (((tmp5 + tmp6) * 20 - (tmp4 + tmp7) * 5 + (tmp3 + tmp8) + 512) >> 10);
    __i32x4 d14 = (((tmp6 + tmp7) * 20 - (tmp5 + tmp8) * 5 + (tmp4 + tmp9) + 512) >> 10);
    __i32x4 d15 = (((tmp7 + tmp8) * 20 - (tmp6 + tmp9) * 5 + (tmp5 + tmp10) + 512) >> 10);

    v128_t sum0 = wasm_i16x8_narrow_i32x4((v128_t)d0, (v128_t)d8);
    v128_t sum1 = wasm_i16x8_narrow_i32x4((v128_t)d1, (v128_t)d9);
    v128_t sum2 = wasm_i16x8_narrow_i32x4((v128_t)d2, (v128_t)d10);
    v128_t sum3 = wasm_i16x8_narrow_i32x4((v128_t)d3, (v128_t)d11);
    v128_t sum4 = wasm_i16x8_narrow_i32x4((v128_t)d4, (v128_t)d12);
    v128_t sum5 = wasm_i16x8_narrow_i32x4((v128_t)d5, (v128_t)d13);
    v128_t sum6 = wasm_i16x8_narrow_i32x4((v128_t)d6, (v128_t)d14);
    v128_t sum7 = wasm_i16x8_narrow_i32x4((v128_t)d7, (v128_t)d15);

    sum0 = wasm_u8x16_narrow_i16x8(sum0, sum1);
    sum1 = wasm_u8x16_narrow_i16x8(sum2, sum3);
    sum2 = wasm_u8x16_narrow_i16x8(sum4, sum5);
    sum3 = wasm_u8x16_narrow_i16x8(sum6, sum7);

    wasm_v128_store64_lane(dst, sum0, 0);
    wasm_v128_store64_lane(dst + dstStride, sum0, 1);
    wasm_v128_store64_lane(dst + 2 * dstStride, sum1, 0);
    wasm_v128_store64_lane(dst + 3 * dstStride, sum1, 1);
    wasm_v128_store64_lane(dst + 4 * dstStride, sum2, 0);
    wasm_v128_store64_lane(dst + 5 * dstStride, sum2, 1);
    wasm_v128_store64_lane(dst + 6 * dstStride, sum3, 0);
    wasm_v128_store64_lane(dst + 7 * dstStride, sum3, 1);
}

static void put_h264_qpel16_v_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    put_h264_qpel8_v_lowpass_8_websimd128(dst, src, dstStride, srcStride);
    put_h264_qpel8_v_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), src + 8 * sizeof(uint8_t), dstStride, srcStride);
    src += 8 * srcStride;
    dst += 8 * dstStride;
    put_h264_qpel8_v_lowpass_8_websimd128(dst, src, dstStride, srcStride);
    put_h264_qpel8_v_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), src + 8 * sizeof(uint8_t), dstStride, srcStride);
}
static void put_h264_qpel16_h_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    put_h264_qpel8_h_lowpass_8_websimd128(dst, src, dstStride, srcStride);
    put_h264_qpel8_h_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), src + 8 * sizeof(uint8_t), dstStride, srcStride);
    src += 8 * srcStride;
    dst += 8 * dstStride;
    put_h264_qpel8_h_lowpass_8_websimd128(dst, src, dstStride, srcStride);
    put_h264_qpel8_h_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), src + 8 * sizeof(uint8_t), dstStride, srcStride);
}
static void put_h264_qpel16_hv_lowpass_8_websimd128(uint8_t *dst, int16_t *tmp, const uint8_t *src, int dstStride, int tmpStride, int srcStride) {
    put_h264_qpel8_hv_lowpass_8_websimd128(dst, tmp, src, dstStride, tmpStride, srcStride);
    put_h264_qpel8_hv_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), tmp + 8, src + 8 * sizeof(uint8_t), dstStride, tmpStride, srcStride);
    src += 8 * srcStride;
    dst += 8 * dstStride;
    put_h264_qpel8_hv_lowpass_8_websimd128(dst, tmp, src, dstStride, tmpStride, srcStride);
    put_h264_qpel8_hv_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), tmp + 8, src + 8 * sizeof(uint8_t), dstStride, tmpStride, srcStride);
}

static void avg_h264_qpel4_h_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    v128_t d0 = wasm_v128_load32_zero(src);
    v128_t d1 = wasm_v128_load32_zero(src + srcStride);
    __i16x8 s0 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src + 1);
    d1 = wasm_v128_load32_zero(src + srcStride + 1);
    __i16x8 s1 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src - 1);
    d1 = wasm_v128_load32_zero(src + srcStride - 1);
    __i16x8 s_1 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src + 2);
    d1 = wasm_v128_load32_zero(src + srcStride + 2);
    __i16x8 s2 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src - 2);
    d1 = wasm_v128_load32_zero(src + srcStride - 2);
    __i16x8 s_2 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );
    
    d0 = wasm_v128_load32_zero(src + 3);
    d1 = wasm_v128_load32_zero(src + srcStride + 3);
    __i16x8 s3 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    __i16x8 sum0 = ((s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3) + 16) >> 5;

    src += srcStride * 2;

    d0 = wasm_v128_load32_zero(src);
    d1 = wasm_v128_load32_zero(src + srcStride);
    s0 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src + 1);
    d1 = wasm_v128_load32_zero(src + srcStride + 1);
    s1 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src - 1);
    d1 = wasm_v128_load32_zero(src + srcStride - 1);
    s_1 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src + 2);
    d1 = wasm_v128_load32_zero(src + srcStride + 2);
    s2 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    d0 = wasm_v128_load32_zero(src - 2);
    d1 = wasm_v128_load32_zero(src + srcStride - 2);
    s_2 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );
    
    d0 = wasm_v128_load32_zero(src + 3);
    d1 = wasm_v128_load32_zero(src + srcStride + 3);
    s3 = (__i16x8)wasm_i16x8_shuffle(
        wasm_u16x8_extend_low_u8x16(d0),
        wasm_u16x8_extend_low_u8x16(d1),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    __i16x8 sum1 = ((s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3) + 16) >> 5;

    sum0 = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum1);

    v128_t ds = wasm_v128_load32_zero(dst);
    ds[1] = wasm_v128_load32_zero(dst + dstStride)[0];
    ds[2] = wasm_v128_load32_zero(dst + 2 * dstStride)[0];
    ds[3] = wasm_v128_load32_zero(dst + 3 * dstStride)[0];

    sum0 = wasm_u8x16_avgr((v128_t)sum0, ds);

    wasm_v128_store32_lane(dst, (v128_t)sum0, 0);
    wasm_v128_store32_lane(dst + dstStride, (v128_t)sum0, 1);
    wasm_v128_store32_lane(dst + 2 * dstStride, (v128_t)sum0, 2);
    wasm_v128_store32_lane(dst + 3 * dstStride, (v128_t)sum0, 3);
}
static void avg_h264_qpel4_v_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    __i32x4 srcB = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src - 2 * srcStride)));
    __i32x4 srcA = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src - 1 * srcStride)));
    __i32x4 src0 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src)));
    __i32x4 src1 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 1 * srcStride)));
    __i32x4 src2 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 2 * srcStride)));
    __i32x4 src3 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 3 * srcStride)));
    __i32x4 src4 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 4 * srcStride)));
    __i32x4 src5 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 5 * srcStride)));
    __i32x4 src6 = (__i32x4)wasm_u32x4_extend_low_u16x8(wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 6 * srcStride)));

    __i32x4 d0 = ((src0 + src1) * 20 - (srcA + src2) * 5 + (srcB + src3) + 16) >> 5;
    __i32x4 d1 = ((src1 + src2) * 20 - (src0 + src3) * 5 + (srcA + src4) + 16) >> 5;
    __i32x4 d2 = ((src2 + src3) * 20 - (src1 + src4) * 5 + (src0+ src5) + 16) >> 5;
    __i32x4 d3 = ((src3 + src4) * 20 - (src2 + src5) * 5 + (src1 + src6) + 16) >> 5;

    v128_t sum = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_narrow_i32x4((v128_t)d0, (v128_t)d1),
        wasm_u16x8_narrow_i32x4((v128_t)d2, (v128_t)d3)
    );

    __u32x4 ds = wasm_v128_load32_zero(dst);
    ds[1] = ((__u32x4)wasm_v128_load32_zero(dst + dstStride))[0];
    ds[2] = ((__u32x4)wasm_v128_load32_zero(dst + 2 * dstStride))[0];
    ds[3] = ((__u32x4)wasm_v128_load32_zero(dst + 3 * dstStride))[0];

    sum = wasm_u8x16_avgr(sum, ds);

    wasm_v128_store32_lane(dst, sum, 0);
    wasm_v128_store32_lane(dst + dstStride, sum, 1);
    wasm_v128_store32_lane(dst + 2 * dstStride, sum, 2);
    wasm_v128_store32_lane(dst + 3 * dstStride, sum, 3);
}
static void avg_h264_qpel4_hv_lowpass_8_websimd128(uint8_t *dst, int16_t *tmp, const uint8_t *src, int dstStride, int tmpStride, int srcStride) {
    const int h = 4;
    int i;
    src -= 2 * srcStride;

    for (i = 0; i < 4; i++) {

        v128_t d0 = wasm_v128_load32_zero(src);
        v128_t d1 = wasm_v128_load32_zero(src + srcStride);
        __i16x8 s0 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );

        d0 = wasm_v128_load32_zero(src + 1);
        d1 = wasm_v128_load32_zero(src + srcStride + 1);
        __i16x8 s1 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );

        d0 = wasm_v128_load32_zero(src - 1);
        d1 = wasm_v128_load32_zero(src + srcStride - 1);
        __i16x8 s_1 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );

        d0 = wasm_v128_load32_zero(src + 2);
        d1 = wasm_v128_load32_zero(src + srcStride + 2);
        __i16x8 s2 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );

        d0 = wasm_v128_load32_zero(src - 2);
        d1 = wasm_v128_load32_zero(src + srcStride - 2);
        __i16x8 s_2 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );
        
        d0 = wasm_v128_load32_zero(src + 3);
        d1 = wasm_v128_load32_zero(src + srcStride + 3);
        __i16x8 s3 = (__i16x8)wasm_i16x8_shuffle(
            wasm_u16x8_extend_low_u8x16(d0),
            wasm_u16x8_extend_low_u8x16(d1),
            0, 1, 2, 3, 8, 9, 10, 11
        );

        __i16x8 sum = ((s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3));

        wasm_v128_store64_lane(tmp, (v128_t)sum, 0);
        wasm_v128_store64_lane(tmp + tmpStride, (v128_t)sum, 1);

        tmp += tmpStride * 2;
        src += srcStride * 2;
    }

    tmp[0] = (src[0] + src[1]) * 20 - (src[-1] + src[2]) * 5 + (src[-2] + src[3]);
    tmp[1] = (src[1] + src[2]) * 20 - (src[0] + src[3]) * 5 + (src[-1] + src[4]);
    tmp[2] = (src[2] + src[3]) * 20 - (src[1] + src[4]) * 5 + (src[0] + src[5]);
    tmp[3] = (src[3] + src[4]) * 20 - (src[2] + src[5]) * 5 + (src[1] + src[6]);

    tmp -= tmpStride * 6;

    __i32x4 srcB = wasm_u32x4_load16x4(tmp - 2 * tmpStride);
    __i32x4 srcA = wasm_u32x4_load16x4(tmp - 1 * tmpStride);
    __i32x4 src0 = wasm_u32x4_load16x4(tmp);
    __i32x4 src1 = wasm_u32x4_load16x4(tmp + 1 * tmpStride);
    __i32x4 src2 = wasm_u32x4_load16x4(tmp + 2 * tmpStride);
    __i32x4 src3 = wasm_u32x4_load16x4(tmp + 3 * tmpStride);
    __i32x4 src4 = wasm_u32x4_load16x4(tmp + 4 * tmpStride);
    __i32x4 src5 = wasm_u32x4_load16x4(tmp + 5 * tmpStride);
    __i32x4 src6 = wasm_u32x4_load16x4(tmp + 6 * tmpStride);

    __i32x4 d0 = ((src0 + src1) * 20 - (srcA + src2) * 5 + (srcB + src3) + 512) >> 10;
    __i32x4 d1 = ((src1 + src2) * 20 - (src0 + src3) * 5 + (srcA + src4) + 512) >> 10;
    __i32x4 d2 = ((src2 + src3) * 20 - (src1 + src4) * 5 + (src0+ src5) + 512) >> 10;
    __i32x4 d3 = ((src3 + src4) * 20 - (src2 + src5) * 5 + (src1 + src6) + 512) >> 10;

    v128_t sum = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_narrow_i32x4(d0, d1),
        wasm_u16x8_narrow_i32x4(d2, d3)
    );

    v128_t ds = wasm_v128_load32_zero(dst);
    ds[1] = wasm_v128_load32_zero(dst + dstStride)[0];
    ds[2] = wasm_v128_load32_zero(dst + 2 * dstStride)[0];
    ds[3] = wasm_v128_load32_zero(dst + 3 * dstStride)[0];

    sum = wasm_u8x16_avgr(sum, ds);

    wasm_v128_store32_lane(dst, sum, 0);
    wasm_v128_store32_lane(dst + dstStride, sum, 1);
    wasm_v128_store32_lane(dst + 2 * dstStride, sum, 2);
    wasm_v128_store32_lane(dst + 3 * dstStride, sum, 3);
}

static void avg_h264_qpel8_h_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    int i;
    for (i = 0; i < 4; i++) {

        __i16x8 s0 = (__i16x8)wasm_u16x8_load8x8(src);
        __i16x8 s1 = (__i16x8)wasm_u16x8_load8x8(src + 1);
        __i16x8 s_1 = (__i16x8)wasm_u16x8_load8x8(src - 1);
        __i16x8 s2 = (__i16x8)wasm_u16x8_load8x8(src + 2);
        __i16x8 s_2 = (__i16x8)wasm_u16x8_load8x8(src - 2);
        __i16x8 s3 = (__i16x8)wasm_u16x8_load8x8(src + 3);

        __i16x8 sum0 = ((s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3) + 16) >> 5;

        s0 = wasm_u16x8_load8x8(src + srcStride);
        s1 = wasm_u16x8_load8x8(src + 1 + srcStride);
        s_1 = wasm_u16x8_load8x8(src - 1 + srcStride);
        s2 = wasm_u16x8_load8x8(src + 2 + srcStride);
        s_2 = wasm_u16x8_load8x8(src - 2 + srcStride);
        s3 = wasm_u16x8_load8x8(src + 3 + srcStride);

        __i16x8 sum1 = ((s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3) + 16) >> 5;

        sum0 = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum1);
        v128_t ds = wasm_u8x16_narrow_i16x8(
            wasm_u16x8_load8x8(dst),
            wasm_u16x8_load8x8(dst + dstStride)
        );
        sum0 = wasm_u8x16_avgr((v128_t)sum0, ds);
        
        wasm_v128_store64_lane(dst, (v128_t)sum0, 0);
        wasm_v128_store64_lane(dst + dstStride, (v128_t)sum0, 1);

        dst += 2 * dstStride;
        src += 2 * srcStride;
    }
}
static void avg_h264_qpel8_v_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    __i16x8 srcB = (__i16x8)wasm_u16x8_load8x8(
        src - 2 * srcStride
    );
    __i16x8 srcA = (__i16x8)wasm_u16x8_load8x8(
        src - srcStride
    );
    __i16x8 src0 = (__i16x8)wasm_u16x8_load8x8(
        src
    );
    __i16x8 src1 = (__i16x8)wasm_u16x8_load8x8(
        src + 1 * srcStride
    );
    __i16x8 src2 = (__i16x8)wasm_u16x8_load8x8(
        src + 2 * srcStride
    );
    __i16x8 src3 = (__i16x8)wasm_u16x8_load8x8(
        src + 3 * srcStride
    );
    __i16x8 src4 = (__i16x8)wasm_u16x8_load8x8(
        src + 4 * srcStride
    );
    __i16x8 src5 = (__i16x8)wasm_u16x8_load8x8(
        src + 5 * srcStride
    );
    __i16x8 src6 = (__i16x8)wasm_u16x8_load8x8(
        src + 6 * srcStride
    );
    __i16x8 src7 = (__i16x8)wasm_u16x8_load8x8(
        src + 7 * srcStride
    );
    __i16x8 src8 = (__i16x8)wasm_u16x8_load8x8(
        src + 8 * srcStride
    );
    __i16x8 src9 = (__i16x8)wasm_u16x8_load8x8(
        src + 9 * srcStride
    );
    __i16x8 src10 = (__i16x8)wasm_u16x8_load8x8(
        src + 10 * srcStride
    );

    __i16x8 d0 = ((src0 + src1) * 20 - (srcA + src2) * 5 + (srcB + src3) + 16) >> 5;
    __i16x8 d1 = ((src1 + src2) * 20 - (src0 + src3) * 5 + (srcA + src4) + 16) >> 5;
    __i16x8 d2 = ((src2 + src3) * 20 - (src1 + src4) * 5 + (src0+ src5) + 16) >> 5;
    __i16x8 d3 = ((src3 + src4) * 20 - (src2 + src5) * 5 + (src1 + src6) + 16) >> 5;
    __i16x8 d4 = ((src4 + src5) * 20 - (src3 + src6) * 5 + (src2 + src7) + 16) >> 5;
    __i16x8 d5 = ((src5 + src6) * 20 - (src4 + src7) * 5 + (src3 + src8) + 16) >> 5;
    __i16x8 d6 = ((src6 + src7) * 20 - (src5 + src8) * 5 + (src4 + src9) + 16) >> 5;
    __i16x8 d7 = ((src7 + src8) * 20 - (src6 + src9) * 5 + (src5 + src10) + 16) >> 5;

    v128_t sum0 = wasm_u8x16_narrow_i16x8((v128_t)d0, (v128_t)d1);
    v128_t sum1 = wasm_u8x16_narrow_i16x8((v128_t)d2, (v128_t)d3);
    v128_t sum2 = wasm_u8x16_narrow_i16x8((v128_t)d4, (v128_t)d5);
    v128_t sum3 = wasm_u8x16_narrow_i16x8((v128_t)d6, (v128_t)d7);

    v128_t ds0 = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_load8x8(dst),
        wasm_u16x8_load8x8(dst + dstStride)
    );
    v128_t ds1 = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_load8x8(dst + 2 * dstStride),
        wasm_u16x8_load8x8(dst + 3 * dstStride)
    );
    v128_t ds2 = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_load8x8(dst + 4 * dstStride),
        wasm_u16x8_load8x8(dst + 5 * dstStride)
    );
    v128_t ds3 = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_load8x8(dst + 6 * dstStride),
        wasm_u16x8_load8x8(dst + 7 * dstStride)
    );

    sum0 = wasm_u8x16_avgr(sum0, ds0);
    sum1 = wasm_u8x16_avgr(sum1, ds1);
    sum2 = wasm_u8x16_avgr(sum2, ds2);
    sum3 = wasm_u8x16_avgr(sum3, ds3);

    wasm_v128_store64_lane(dst, sum0, 0);
    wasm_v128_store64_lane(dst + dstStride, sum0, 1);
    wasm_v128_store64_lane(dst + 2 * dstStride, sum1, 0);
    wasm_v128_store64_lane(dst + 3 * dstStride, sum1, 1);
    wasm_v128_store64_lane(dst + 4 * dstStride, sum2, 0);
    wasm_v128_store64_lane(dst + 5 * dstStride, sum2, 1);
    wasm_v128_store64_lane(dst + 6 * dstStride, sum3, 0);
    wasm_v128_store64_lane(dst + 7 * dstStride, sum3, 1);
}
static void avg_h264_qpel8_hv_lowpass_8_websimd128(uint8_t *dst, int16_t *tmp, const uint8_t *src, int dstStride, int tmpStride, int srcStride) {
    const int h = 8;
    int i;
    src -= 2 * srcStride;

    for (i = 0; i < h + 5; i++) {
        __i16x8 s0 = (__i16x8)wasm_u16x8_load8x8(src);
        __i16x8 s1 = (__i16x8)wasm_u16x8_load8x8(src + 1);
        __i16x8 s_1 = (__i16x8)wasm_u16x8_load8x8(src - 1);
        __i16x8 s2 = (__i16x8)wasm_u16x8_load8x8(src + 2);
        __i16x8 s_2 = (__i16x8)wasm_u16x8_load8x8(src - 2);
        __i16x8 s3 = (__i16x8)wasm_u16x8_load8x8(src + 3);

        __i16x8 sum = (s0 + s1) * 20 - (s_1 + s2) * 5 + (s_2 + s3);
        wasm_v128_store(tmp, (v128_t)sum);
        tmp += tmpStride;
        src += srcStride;
    }
    tmp -= tmpStride * (h + 5 - 2);

    __i32x4 tmpB = (__i32x4)wasm_i32x4_load16x4(tmp -2 * tmpStride);
    __i32x4 tmpA = (__i32x4)wasm_i32x4_load16x4(tmp -1 * tmpStride);
    __i32x4 tmp0 = (__i32x4)wasm_i32x4_load16x4(tmp);
    __i32x4 tmp1 = (__i32x4)wasm_i32x4_load16x4(tmp + 1 * tmpStride);
    __i32x4 tmp2 = (__i32x4)wasm_i32x4_load16x4(tmp + 2 * tmpStride);
    __i32x4 tmp3 = (__i32x4)wasm_i32x4_load16x4(tmp + 3 * tmpStride);
    __i32x4 tmp4 = (__i32x4)wasm_i32x4_load16x4(tmp + 4 * tmpStride);
    __i32x4 tmp5 = (__i32x4)wasm_i32x4_load16x4(tmp + 5 * tmpStride);
    __i32x4 tmp6 = (__i32x4)wasm_i32x4_load16x4(tmp + 6 * tmpStride);
    __i32x4 tmp7 = (__i32x4)wasm_i32x4_load16x4(tmp + 7 * tmpStride);
    __i32x4 tmp8 = (__i32x4)wasm_i32x4_load16x4(tmp + 8 * tmpStride);
    __i32x4 tmp9 = (__i32x4)wasm_i32x4_load16x4(tmp + 9 * tmpStride);
    __i32x4 tmp10 = (__i32x4)wasm_i32x4_load16x4(tmp + 10 * tmpStride);

    __i32x4 d0 = (((tmp0 + tmp1) * 20 - (tmpA + tmp2) * 5 + (tmpB + tmp3) + 512) >> 10);
    __i32x4 d1 = (((tmp1 + tmp2) * 20 - (tmp0 + tmp3) * 5 + (tmpA + tmp4) + 512) >> 10);
    __i32x4 d2 = (((tmp2 + tmp3) * 20 - (tmp1 + tmp4) * 5 + (tmp0 + tmp5) + 512) >> 10);
    __i32x4 d3 = (((tmp3 + tmp4) * 20 - (tmp2 + tmp5) * 5 + (tmp1 + tmp6) + 512) >> 10);
    __i32x4 d4 = (((tmp4 + tmp5) * 20 - (tmp3 + tmp6) * 5 + (tmp2 + tmp7) + 512) >> 10);
    __i32x4 d5 = (((tmp5 + tmp6) * 20 - (tmp4 + tmp7) * 5 + (tmp3 + tmp8) + 512) >> 10);
    __i32x4 d6 = (((tmp6 + tmp7) * 20 - (tmp5 + tmp8) * 5 + (tmp4 + tmp9) + 512) >> 10);
    __i32x4 d7 = (((tmp7 + tmp8) * 20 - (tmp6 + tmp9) * 5 + (tmp5 + tmp10) + 512) >> 10);

    tmp += 4;
    
    tmpB = (__i32x4)wasm_i32x4_load16x4(tmp -2 * tmpStride);
    tmpA = (__i32x4)wasm_i32x4_load16x4(tmp -1 * tmpStride);
    tmp0 = (__i32x4)wasm_i32x4_load16x4(tmp);
    tmp1 = (__i32x4)wasm_i32x4_load16x4(tmp + 1 * tmpStride);
    tmp2 = (__i32x4)wasm_i32x4_load16x4(tmp + 2 * tmpStride);
    tmp3 = (__i32x4)wasm_i32x4_load16x4(tmp + 3 * tmpStride);
    tmp4 = (__i32x4)wasm_i32x4_load16x4(tmp + 4 * tmpStride);
    tmp5 = (__i32x4)wasm_i32x4_load16x4(tmp + 5 * tmpStride);
    tmp6 = (__i32x4)wasm_i32x4_load16x4(tmp + 6 * tmpStride);
    tmp7 = (__i32x4)wasm_i32x4_load16x4(tmp + 7 * tmpStride);
    tmp8 = (__i32x4)wasm_i32x4_load16x4(tmp + 8 * tmpStride);
    tmp9 = (__i32x4)wasm_i32x4_load16x4(tmp + 9 * tmpStride);
    tmp10 = (__i32x4)wasm_i32x4_load16x4(tmp + 10 * tmpStride);

    __i32x4 d8 = (((tmp0 + tmp1) * 20 - (tmpA + tmp2) * 5 + (tmpB + tmp3) + 512) >> 10);
    __i32x4 d9 = (((tmp1 + tmp2) * 20 - (tmp0 + tmp3) * 5 + (tmpA + tmp4) + 512) >> 10);
    __i32x4 d10 = (((tmp2 + tmp3) * 20 - (tmp1 + tmp4) * 5 + (tmp0 + tmp5) + 512) >> 10);
    __i32x4 d11 = (((tmp3 + tmp4) * 20 - (tmp2 + tmp5) * 5 + (tmp1 + tmp6) + 512) >> 10);
    __i32x4 d12 = (((tmp4 + tmp5) * 20 - (tmp3 + tmp6) * 5 + (tmp2 + tmp7) + 512) >> 10);
    __i32x4 d13 = (((tmp5 + tmp6) * 20 - (tmp4 + tmp7) * 5 + (tmp3 + tmp8) + 512) >> 10);
    __i32x4 d14 = (((tmp6 + tmp7) * 20 - (tmp5 + tmp8) * 5 + (tmp4 + tmp9) + 512) >> 10);
    __i32x4 d15 = (((tmp7 + tmp8) * 20 - (tmp6 + tmp9) * 5 + (tmp5 + tmp10) + 512) >> 10);

    v128_t sum0 = wasm_i16x8_narrow_i32x4((v128_t)d0, (v128_t)d8);
    v128_t sum1 = wasm_i16x8_narrow_i32x4((v128_t)d1, (v128_t)d9);
    v128_t sum2 = wasm_i16x8_narrow_i32x4((v128_t)d2, (v128_t)d10);
    v128_t sum3 = wasm_i16x8_narrow_i32x4((v128_t)d3, (v128_t)d11);
    v128_t sum4 = wasm_i16x8_narrow_i32x4((v128_t)d4, (v128_t)d12);
    v128_t sum5 = wasm_i16x8_narrow_i32x4((v128_t)d5, (v128_t)d13);
    v128_t sum6 = wasm_i16x8_narrow_i32x4((v128_t)d6, (v128_t)d14);
    v128_t sum7 = wasm_i16x8_narrow_i32x4((v128_t)d7, (v128_t)d15);

    sum0 = wasm_u8x16_narrow_i16x8(sum0, sum1);
    sum1 = wasm_u8x16_narrow_i16x8(sum2, sum3);
    sum2 = wasm_u8x16_narrow_i16x8(sum4, sum5);
    sum3 = wasm_u8x16_narrow_i16x8(sum6, sum7);

    v128_t ds0 = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_load8x8(dst),
        wasm_u16x8_load8x8(dst + dstStride)
    );
    v128_t ds1 = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_load8x8(dst + 2 * dstStride),
        wasm_u16x8_load8x8(dst + 3 * dstStride)
    );
    v128_t ds2 = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_load8x8(dst + 4 * dstStride),
        wasm_u16x8_load8x8(dst + 5 * dstStride)
    );
    v128_t ds3 = wasm_u8x16_narrow_i16x8(
        wasm_u16x8_load8x8(dst + 6 * dstStride),
        wasm_u16x8_load8x8(dst + 7 * dstStride)
    );

    sum0 = wasm_u8x16_avgr(sum0, ds0);
    sum1 = wasm_u8x16_avgr(sum1, ds1);
    sum2 = wasm_u8x16_avgr(sum2, ds2);
    sum3 = wasm_u8x16_avgr(sum3, ds3);

    wasm_v128_store64_lane(dst, sum0, 0);
    wasm_v128_store64_lane(dst + dstStride, sum0, 1);
    wasm_v128_store64_lane(dst + 2 * dstStride, sum1, 0);
    wasm_v128_store64_lane(dst + 3 * dstStride, sum1, 1);
    wasm_v128_store64_lane(dst + 4 * dstStride, sum2, 0);
    wasm_v128_store64_lane(dst + 5 * dstStride, sum2, 1);
    wasm_v128_store64_lane(dst + 6 * dstStride, sum3, 0);
    wasm_v128_store64_lane(dst + 7 * dstStride, sum3, 1);
}

static void avg_h264_qpel16_v_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    avg_h264_qpel8_v_lowpass_8_websimd128(dst, src, dstStride, srcStride);
    avg_h264_qpel8_v_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), src + 8 * sizeof(uint8_t), dstStride, srcStride);
    src += 8 * srcStride;
    dst += 8 * dstStride;
    avg_h264_qpel8_v_lowpass_8_websimd128(dst, src, dstStride, srcStride);
    avg_h264_qpel8_v_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), src + 8 * sizeof(uint8_t), dstStride, srcStride);
}
static void avg_h264_qpel16_h_lowpass_8_websimd128(uint8_t *dst, const uint8_t *src, int dstStride, int srcStride) {
    avg_h264_qpel8_h_lowpass_8_websimd128(dst, src, dstStride, srcStride);
    avg_h264_qpel8_h_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), src + 8 * sizeof(uint8_t), dstStride, srcStride);
    src += 8 * srcStride;
    dst += 8 * dstStride;
    avg_h264_qpel8_h_lowpass_8_websimd128(dst, src, dstStride, srcStride);
    avg_h264_qpel8_h_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), src + 8 * sizeof(uint8_t), dstStride, srcStride);
}
static void avg_h264_qpel16_hv_lowpass_8_websimd128(uint8_t *dst, int16_t *tmp, const uint8_t *src, int dstStride, int tmpStride, int srcStride) {
    avg_h264_qpel8_hv_lowpass_8_websimd128(dst, tmp, src, dstStride, tmpStride, srcStride);
    avg_h264_qpel8_hv_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), tmp + 8, src + 8 * sizeof(uint8_t), dstStride, tmpStride, srcStride);
    src += 8 * srcStride;
    dst += 8 * dstStride;
    avg_h264_qpel8_hv_lowpass_8_websimd128(dst, tmp, src, dstStride, tmpStride, srcStride);
    avg_h264_qpel8_hv_lowpass_8_websimd128(dst + 8 * sizeof(uint8_t), tmp + 8, src + 8 * sizeof(uint8_t), dstStride, tmpStride, srcStride);
}

void put_h264_qpel4_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    put_pixels4_8_websimd128(dst, src, stride);
}
void put_h264_qpel4_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(half, src, 4 * sizeof(uint8_t), stride);
    put_pixels4_l2_8(dst, src, half, stride, stride, 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    put_h264_qpel4_h_lowpass_8_websimd128(dst, src, stride, stride);
}
void put_h264_qpel4_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(half, src, 4 * sizeof(uint8_t), stride);
    put_pixels4_l2_8(dst, src + sizeof(uint8_t), half, stride, stride, 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t half[4 * 4 * sizeof(uint8_t)];
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(half, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    put_pixels4_l2_8(dst, full_mid, half, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(dst, full_mid, stride, 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t half[4 * 4 * sizeof(uint8_t)];
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(half, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    put_pixels4_l2_8(dst, full_mid + 4 * sizeof(uint8_t), half, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src, 4 * sizeof(uint8_t), stride);
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    put_pixels4_l2_8(dst, halfH, halfV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src, 4 * sizeof(uint8_t), stride);
    copy_block4_8(full, src - stride * 2 + sizeof(uint8_t), 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    put_pixels4_l2_8(dst, halfH, halfV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src + stride, 4 * sizeof(uint8_t), stride);
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    put_pixels4_l2_8(dst, halfH, halfV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src + stride, 4 * sizeof(uint8_t), stride);
    copy_block4_8(full, src - stride * 2 + sizeof(uint8_t), 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    put_pixels4_l2_8(dst, halfH, halfV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[4 * (4 + 5) * sizeof(uint8_t)];
    put_h264_qpel4_hv_lowpass_8_websimd128(dst, tmp, src, stride, 4 * sizeof(uint8_t), stride);
}
void put_h264_qpel4_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfHV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src, 4 * sizeof(uint8_t), stride);
    put_h264_qpel4_hv_lowpass_8_websimd128(halfHV, tmp, src, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t), stride);
    put_pixels4_l2_8(dst, halfH, halfHV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfHV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src + stride, 4 * sizeof(uint8_t), stride);
    put_h264_qpel4_hv_lowpass_8_websimd128(halfHV, tmp, src, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t), stride);
    put_pixels4_l2_8(dst, halfH, halfHV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    int16_t tmp[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    uint8_t halfHV[4 * 4 * sizeof(uint8_t)];
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    put_h264_qpel4_hv_lowpass_8_websimd128(halfHV, tmp, src, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t), stride);
    put_pixels4_l2_8(dst, halfV, halfHV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void put_h264_qpel4_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    int16_t tmp[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    uint8_t halfHV[4 * 4 * sizeof(uint8_t)];
    copy_block4_8(full, src - stride * 2 + sizeof(uint8_t), 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    put_h264_qpel4_hv_lowpass_8_websimd128(halfHV, tmp, src, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t), stride);
    put_pixels4_l2_8(dst, halfV, halfHV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void put_h264_qpel8_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    put_pixels8_8_websimd128(dst, src, stride);
}
void put_h264_qpel8_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(half, src, 8 * sizeof(uint8_t), stride);
    put_pixels8_l2_8(dst, src, half, stride, stride, 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    put_h264_qpel8_h_lowpass_8_websimd128(dst, src, stride, stride);
}
void put_h264_qpel8_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(half, src, 8 * sizeof(uint8_t), stride);
    put_pixels8_l2_8(dst, src + sizeof(uint8_t), half, stride, stride, 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t half[8 * 8 * sizeof(uint8_t)];
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(half, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    put_pixels8_l2_8(dst, full_mid, half, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(dst, full_mid, stride, 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t half[8 * 8 * sizeof(uint8_t)];
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(half, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    put_pixels8_l2_8(dst, full_mid + 8 * sizeof(uint8_t), half, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src, 8 * sizeof(uint8_t), stride);
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    put_pixels8_l2_8(dst, halfH, halfV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src, 8 * sizeof(uint8_t), stride);
    copy_block8_8(full, src - stride * 2 + sizeof(uint8_t), 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    put_pixels8_l2_8(dst, halfH, halfV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src + stride, 8 * sizeof(uint8_t), stride);
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    put_pixels8_l2_8(dst, halfH, halfV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src + stride, 8 * sizeof(uint8_t), stride);
    copy_block8_8(full, src - stride * 2 + sizeof(uint8_t), 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    put_pixels8_l2_8(dst, halfH, halfV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[8 * (8 + 5) * sizeof(uint8_t)];
    put_h264_qpel8_hv_lowpass_8_websimd128(dst, tmp, src, stride, 8 * sizeof(uint8_t), stride);
}
void put_h264_qpel8_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfHV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src, 8 * sizeof(uint8_t), stride);
    put_h264_qpel8_hv_lowpass_8_websimd128(halfHV, tmp, src, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t), stride);
    put_pixels8_l2_8(dst, halfH, halfHV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfHV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src + stride, 8 * sizeof(uint8_t), stride);
    put_h264_qpel8_hv_lowpass_8_websimd128(halfHV, tmp, src, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t), stride);
    put_pixels8_l2_8(dst, halfH, halfHV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    int16_t tmp[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    uint8_t halfHV[8 * 8 * sizeof(uint8_t)];
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    put_h264_qpel8_hv_lowpass_8_websimd128(halfHV, tmp, src, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t), stride);
    put_pixels8_l2_8(dst, halfV, halfHV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void put_h264_qpel8_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    int16_t tmp[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    uint8_t halfHV[8 * 8 * sizeof(uint8_t)];
    copy_block8_8(full, src - stride * 2 + sizeof(uint8_t), 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    put_h264_qpel8_hv_lowpass_8_websimd128(halfHV, tmp, src, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t), stride);
    put_pixels8_l2_8(dst, halfV, halfHV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void put_h264_qpel16_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    put_pixels16_8_websimd128(dst, src, stride);
}
void put_h264_qpel16_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(half, src, 16 * sizeof(uint8_t), stride);
    put_pixels16_l2_8(dst, src, half, stride, stride, 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    put_h264_qpel16_h_lowpass_8_websimd128(dst, src, stride, stride);
}
void put_h264_qpel16_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(half, src, 16 * sizeof(uint8_t), stride);
    put_pixels16_l2_8(dst, src + sizeof(uint8_t), half, stride, stride, 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t half[16 * 16 * sizeof(uint8_t)];
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(half, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    put_pixels16_l2_8(dst, full_mid, half, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(dst, full_mid, stride, 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t half[16 * 16 * sizeof(uint8_t)];
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(half, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    put_pixels16_l2_8(dst, full_mid + 16 * sizeof(uint8_t), half, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src, 16 * sizeof(uint8_t), stride);
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    put_pixels16_l2_8(dst, halfH, halfV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src, 16 * sizeof(uint8_t), stride);
    copy_block16_8(full, src - stride * 2 + sizeof(uint8_t), 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    put_pixels16_l2_8(dst, halfH, halfV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src + stride, 16 * sizeof(uint8_t), stride);
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    put_pixels16_l2_8(dst, halfH, halfV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src + stride, 16 * sizeof(uint8_t), stride);
    copy_block16_8(full, src - stride * 2 + sizeof(uint8_t), 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    put_pixels16_l2_8(dst, halfH, halfV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[16 * (16 + 5) * sizeof(uint8_t)];
    put_h264_qpel16_hv_lowpass_8_websimd128(dst, tmp, src, stride, 16 * sizeof(uint8_t), stride);
}
void put_h264_qpel16_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfHV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src, 16 * sizeof(uint8_t), stride);
    put_h264_qpel16_hv_lowpass_8_websimd128(halfHV, tmp, src, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t), stride);
    put_pixels16_l2_8(dst, halfH, halfHV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfHV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src + stride, 16 * sizeof(uint8_t), stride);
    put_h264_qpel16_hv_lowpass_8_websimd128(halfHV, tmp, src, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t), stride);
    put_pixels16_l2_8(dst, halfH, halfHV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    int16_t tmp[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    uint8_t halfHV[16 * 16 * sizeof(uint8_t)];
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    put_h264_qpel16_hv_lowpass_8_websimd128(halfHV, tmp, src, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t), stride);
    put_pixels16_l2_8(dst, halfV, halfHV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void put_h264_qpel16_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    int16_t tmp[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    uint8_t halfHV[16 * 16 * sizeof(uint8_t)];
    copy_block16_8(full, src - stride * 2 + sizeof(uint8_t), 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    put_h264_qpel16_hv_lowpass_8_websimd128(halfHV, tmp, src, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t), stride);
    put_pixels16_l2_8(dst, halfV, halfHV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    avg_pixels4_8_websimd128(dst, src, stride);
}
void avg_h264_qpel4_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(half, src, 4 * sizeof(uint8_t), stride);
    avg_pixels4_l2_8(dst, src, half, stride, stride, 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    avg_h264_qpel4_h_lowpass_8_websimd128(dst, src, stride, stride);
}
void avg_h264_qpel4_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(half, src, 4 * sizeof(uint8_t), stride);
    avg_pixels4_l2_8(dst, src + sizeof(uint8_t), half, stride, stride, 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t half[4 * 4 * sizeof(uint8_t)];
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(half, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    avg_pixels4_l2_8(dst, full_mid, half, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    avg_h264_qpel4_v_lowpass_8_websimd128(dst, full_mid, stride, 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t half[4 * 4 * sizeof(uint8_t)];
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(half, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    avg_pixels4_l2_8(dst, full_mid + 4 * sizeof(uint8_t), half, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src, 4 * sizeof(uint8_t), stride);
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    avg_pixels4_l2_8(dst, halfH, halfV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src, 4 * sizeof(uint8_t), stride);
    copy_block4_8(full, src - stride * 2 + sizeof(uint8_t), 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    avg_pixels4_l2_8(dst, halfH, halfV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src + stride, 4 * sizeof(uint8_t), stride);
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    avg_pixels4_l2_8(dst, halfH, halfV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src + stride, 4 * sizeof(uint8_t), stride);
    copy_block4_8(full, src - stride * 2 + sizeof(uint8_t), 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    avg_pixels4_l2_8(dst, halfH, halfV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[4 * (4 + 5) * sizeof(uint8_t)];
    avg_h264_qpel4_hv_lowpass_8_websimd128(dst, tmp, src, stride, 4 * sizeof(uint8_t), stride);
}
void avg_h264_qpel4_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfHV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src, 4 * sizeof(uint8_t), stride);
    put_h264_qpel4_hv_lowpass_8_websimd128(halfHV, tmp, src, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t), stride);
    avg_pixels4_l2_8(dst, halfH, halfHV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t halfH[4 * 4 * sizeof(uint8_t)];
    uint8_t halfHV[4 * 4 * sizeof(uint8_t)];
    put_h264_qpel4_h_lowpass_8_websimd128(halfH, src + stride, 4 * sizeof(uint8_t), stride);
    put_h264_qpel4_hv_lowpass_8_websimd128(halfHV, tmp, src, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t), stride);
    avg_pixels4_l2_8(dst, halfH, halfHV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    int16_t tmp[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    uint8_t halfHV[4 * 4 * sizeof(uint8_t)];
    copy_block4_8(full, src - stride * 2, 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    put_h264_qpel4_hv_lowpass_8_websimd128(halfHV, tmp, src, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t), stride);
    avg_pixels4_l2_8(dst, halfV, halfHV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void avg_h264_qpel4_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 4 * 2 * sizeof(uint8_t);
    int16_t tmp[4 * (4 + 5) * sizeof(uint8_t)];
    uint8_t halfV[4 * 4 * sizeof(uint8_t)];
    uint8_t halfHV[4 * 4 * sizeof(uint8_t)];
    copy_block4_8(full, src - stride * 2 + sizeof(uint8_t), 4 * sizeof(uint8_t), stride, 4 + 5);
    put_h264_qpel4_v_lowpass_8_websimd128(halfV, full_mid, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
    put_h264_qpel4_hv_lowpass_8_websimd128(halfHV, tmp, src, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t), stride);
    avg_pixels4_l2_8(dst, halfV, halfHV, stride, 4 * sizeof(uint8_t), 4 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    avg_pixels8_8_websimd128(dst, src, stride);
}
void avg_h264_qpel8_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(half, src, 8 * sizeof(uint8_t), stride);
    avg_pixels8_l2_8(dst, src, half, stride, stride, 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    avg_h264_qpel8_h_lowpass_8_websimd128(dst, src, stride, stride);
}
void avg_h264_qpel8_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(half, src, 8 * sizeof(uint8_t), stride);
    avg_pixels8_l2_8(dst, src + sizeof(uint8_t), half, stride, stride, 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t half[8 * 8 * sizeof(uint8_t)];
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(half, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    avg_pixels8_l2_8(dst, full_mid, half, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    avg_h264_qpel8_v_lowpass_8_websimd128(dst, full_mid, stride, 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t half[8 * 8 * sizeof(uint8_t)];
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(half, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    avg_pixels8_l2_8(dst, full_mid + 8 * sizeof(uint8_t), half, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src, 8 * sizeof(uint8_t), stride);
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    avg_pixels8_l2_8(dst, halfH, halfV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src, 8 * sizeof(uint8_t), stride);
    copy_block8_8(full, src - stride * 2 + sizeof(uint8_t), 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    avg_pixels8_l2_8(dst, halfH, halfV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src + stride, 8 * sizeof(uint8_t), stride);
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    avg_pixels8_l2_8(dst, halfH, halfV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src + stride, 8 * sizeof(uint8_t), stride);
    copy_block8_8(full, src - stride * 2 + sizeof(uint8_t), 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    avg_pixels8_l2_8(dst, halfH, halfV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[8 * (8 + 5) * sizeof(uint8_t)];
    avg_h264_qpel8_hv_lowpass_8_websimd128(dst, tmp, src, stride, 8 * sizeof(uint8_t), stride);
}
void avg_h264_qpel8_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfHV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src, 8 * sizeof(uint8_t), stride);
    put_h264_qpel8_hv_lowpass_8_websimd128(halfHV, tmp, src, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t), stride);
    avg_pixels8_l2_8(dst, halfH, halfHV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t halfH[8 * 8 * sizeof(uint8_t)];
    uint8_t halfHV[8 * 8 * sizeof(uint8_t)];
    put_h264_qpel8_h_lowpass_8_websimd128(halfH, src + stride, 8 * sizeof(uint8_t), stride);
    put_h264_qpel8_hv_lowpass_8_websimd128(halfHV, tmp, src, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t), stride);
    avg_pixels8_l2_8(dst, halfH, halfHV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    int16_t tmp[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    uint8_t halfHV[8 * 8 * sizeof(uint8_t)];
    copy_block8_8(full, src - stride * 2, 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    put_h264_qpel8_hv_lowpass_8_websimd128(halfHV, tmp, src, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t), stride);
    avg_pixels8_l2_8(dst, halfV, halfHV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void avg_h264_qpel8_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 8 * 2 * sizeof(uint8_t);
    int16_t tmp[8 * (8 + 5) * sizeof(uint8_t)];
    uint8_t halfV[8 * 8 * sizeof(uint8_t)];
    uint8_t halfHV[8 * 8 * sizeof(uint8_t)];
    copy_block8_8(full, src - stride * 2 + sizeof(uint8_t), 8 * sizeof(uint8_t), stride, 8 + 5);
    put_h264_qpel8_v_lowpass_8_websimd128(halfV, full_mid, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
    put_h264_qpel8_hv_lowpass_8_websimd128(halfHV, tmp, src, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t), stride);
    avg_pixels8_l2_8(dst, halfV, halfHV, stride, 8 * sizeof(uint8_t), 8 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    avg_pixels16_8_websimd128(dst, src, stride);
}
void avg_h264_qpel16_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(half, src, 16 * sizeof(uint8_t), stride);
    avg_pixels16_l2_8(dst, src, half, stride, stride, 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    avg_h264_qpel16_h_lowpass_8_websimd128(dst, src, stride, stride);
}
void avg_h264_qpel16_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t half[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(half, src, 16 * sizeof(uint8_t), stride);
    avg_pixels16_l2_8(dst, src + sizeof(uint8_t), half, stride, stride, 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t half[16 * 16 * sizeof(uint8_t)];
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(half, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    avg_pixels16_l2_8(dst, full_mid, half, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    avg_h264_qpel16_v_lowpass_8_websimd128(dst, full_mid, stride, 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t half[16 * 16 * sizeof(uint8_t)];
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(half, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    avg_pixels16_l2_8(dst, full_mid + 16 * sizeof(uint8_t), half, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src, 16 * sizeof(uint8_t), stride);
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    avg_pixels16_l2_8(dst, halfH, halfV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src, 16 * sizeof(uint8_t), stride);
    copy_block16_8(full, src - stride * 2 + sizeof(uint8_t), 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    avg_pixels16_l2_8(dst, halfH, halfV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src + stride, 16 * sizeof(uint8_t), stride);
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    avg_pixels16_l2_8(dst, halfH, halfV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src + stride, 16 * sizeof(uint8_t), stride);
    copy_block16_8(full, src - stride * 2 + sizeof(uint8_t), 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    avg_pixels16_l2_8(dst, halfH, halfV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[16 * (16 + 5) * sizeof(uint8_t)];
    avg_h264_qpel16_hv_lowpass_8_websimd128(dst, tmp, src, stride, 16 * sizeof(uint8_t), stride);
}
void avg_h264_qpel16_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfHV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src, 16 * sizeof(uint8_t), stride);
    put_h264_qpel16_hv_lowpass_8_websimd128(halfHV, tmp, src, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t), stride);
    avg_pixels16_l2_8(dst, halfH, halfHV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    int16_t tmp[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t halfH[16 * 16 * sizeof(uint8_t)];
    uint8_t halfHV[16 * 16 * sizeof(uint8_t)];
    put_h264_qpel16_h_lowpass_8_websimd128(halfH, src + stride, 16 * sizeof(uint8_t), stride);
    put_h264_qpel16_hv_lowpass_8_websimd128(halfHV, tmp, src, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t), stride);
    avg_pixels16_l2_8(dst, halfH, halfHV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    int16_t tmp[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    uint8_t halfHV[16 * 16 * sizeof(uint8_t)];
    copy_block16_8(full, src - stride * 2, 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    put_h264_qpel16_hv_lowpass_8_websimd128(halfHV, tmp, src, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t), stride);
    avg_pixels16_l2_8(dst, halfV, halfHV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}
void avg_h264_qpel16_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride) {
    uint8_t full[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t *const full_mid = full + 16 * 2 * sizeof(uint8_t);
    int16_t tmp[16 * (16 + 5) * sizeof(uint8_t)];
    uint8_t halfV[16 * 16 * sizeof(uint8_t)];
    uint8_t halfHV[16 * 16 * sizeof(uint8_t)];
    copy_block16_8(full, src - stride * 2 + sizeof(uint8_t), 16 * sizeof(uint8_t), stride, 16 + 5);
    put_h264_qpel16_v_lowpass_8_websimd128(halfV, full_mid, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
    put_h264_qpel16_hv_lowpass_8_websimd128(halfHV, tmp, src, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t), stride);
    avg_pixels16_l2_8(dst, halfV, halfHV, stride, 16 * sizeof(uint8_t), 16 * sizeof(uint8_t));
}