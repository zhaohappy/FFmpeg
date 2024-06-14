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


#include <stddef.h>
#include <wasm_simd128.h>
#include "h264chroma_webassembly.h"

void ff_put_h264_chroma_mc4_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride, int height, int x, int y) {
    const int A=(8-x)*(8-y);
    const int B=(  x)*(8-y);
    const int C=(8-x)*(  y);
    const int D=(  x)*(  y);
    const __u16x8 A128 = (__u16x8)wasm_u16x8_splat(A);
    const __u16x8 B128 = (__u16x8)wasm_u16x8_splat(B);
    const __u16x8 C128 = (__u16x8)wasm_u16x8_splat(C);
    const __u16x8 D128 = (__u16x8)wasm_u16x8_splat(D);
    int i;

    if(D) {
        for(i = 0; i < height; i++){
            __u16x8 src0 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src));
            __u16x8 src1 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 1));
            __u16x8 src2 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + stride));
            __u16x8 src3 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + stride + 1));

            __u16x8 sum = A128 * src0 + B128 * src1 + C128 * src2 + D128 * src3;
            sum = (sum + 32) >> 6;
            sum = wasm_u8x16_narrow_i16x8((v128_t)sum, (v128_t)sum);
            wasm_v128_store32_lane(dst, (v128_t)sum, 0);

            dst += stride;
            src += stride;
        }
    }
    else if (B + C) {
        const __u16x8 E128 = B128 + C128;
        const ptrdiff_t step = C ? stride : 1;
        for(i = 0; i < height; i++){
            __u16x8 src0 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src));
            __u16x8 src1 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + step));

            __u16x8 sum = A128 * src0 + E128 * src1;
            sum = (sum + 32) >> 6;
            sum = wasm_u8x16_narrow_i16x8((v128_t)sum, (v128_t)sum);
            wasm_v128_store32_lane(dst, (v128_t)sum, 0);
            
            dst += stride;
            src += stride;
        }
    }
    else {
        for ( i = 0; i < height; i++){
            __u16x8 src0 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src));

            __u16x8 sum = A128 * src0;
            sum = (sum + 32) >> 6;
            sum = wasm_u8x16_narrow_i16x8((v128_t)sum, (v128_t)sum);
            wasm_v128_store32_lane(dst, (v128_t)sum, 0);

            dst += stride;
            src += stride;
        }
    }
}

void ff_avg_h264_chroma_mc4_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride, int height, int x, int y) {
    const int A=(8-x)*(8-y);
    const int B=(  x)*(8-y);
    const int C=(8-x)*(  y);
    const int D=(  x)*(  y);
    const __u16x8 A128 = (__u16x8)wasm_u16x8_splat(A);
    const __u16x8 B128 = (__u16x8)wasm_u16x8_splat(B);
    const __u16x8 C128 = (__u16x8)wasm_u16x8_splat(C);
    const __u16x8 D128 = (__u16x8)wasm_u16x8_splat(D);
    int i;

    if(D){
        for(i = 0; i < height; i++){
            __u16x8 dst128 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst));
            __u16x8 src0 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src));
            __u16x8 src1 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + 1));
            __u16x8 src2 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + stride));
            __u16x8 src3 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + stride + 1));

            __u16x8 sum = A128 * src0 + B128 * src1 + C128 * src2 + D128 * src3;
            sum = (sum + 32) >> 6;
            sum = (dst128 + sum + 1) >> 1;
            sum = wasm_u8x16_narrow_i16x8((v128_t)sum, (v128_t)sum);
            wasm_v128_store32_lane(dst, (v128_t)sum, 0);
            
            dst += stride;
            src += stride;
        }
    }
    else if (B + C) {
        const __u16x8 E128 = B128 + C128;;
        const ptrdiff_t step = C ? stride : 1;
        for(i = 0; i < height; i++){
            __u16x8 dst128 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst));
            __u16x8 src0 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src));
            __u16x8 src1 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src + step));

            __u16x8 sum = A128 * src0 + E128 * src1;
            sum = (sum + 32) >> 6;
            sum = (dst128 + sum + 1) >> 1;
            sum = wasm_u8x16_narrow_i16x8((v128_t)sum, (v128_t)sum);
            wasm_v128_store32_lane(dst, (v128_t)sum, 0);

            dst += stride;
            src += stride;
        }
    }
    else {
        for (i = 0; i < height; i++){
            __u16x8 dst128 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst));
            __u16x8 src0 = (__u16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src));

            __u16x8 sum = A128 * src0;
            sum = (sum + 32) >> 6;
            sum = (dst128 + sum + 1) >> 1;
            sum = wasm_u8x16_narrow_i16x8((v128_t)sum, (v128_t)sum);
            wasm_v128_store32_lane(dst, (v128_t)sum, 0);

            dst += stride;
            src += stride;
        }
    }
}

void ff_put_h264_chroma_mc8_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride, int height, int x, int y) {
    const int A=(8-x)*(8-y);
    const int B=(  x)*(8-y);
    const int C=(8-x)*(  y);
    const int D=(  x)*(  y);

    const __u16x8 A128 = (__u16x8)wasm_u16x8_splat(A);
    const __u16x8 B128 = (__u16x8)wasm_u16x8_splat(B);
    const __u16x8 C128 = (__u16x8)wasm_u16x8_splat(C);
    const __u16x8 D128 = (__u16x8)wasm_u16x8_splat(D);

    int i;

    int height2 = height >> 1;
    
    if (D) {
        for(i = 0; i < height2; i++) {
            __u16x8 src0 = (__u16x8)wasm_u16x8_load8x8(src);
            __u16x8 src1 = (__u16x8)wasm_u16x8_load8x8(src + 1);
            __u16x8 src2 = (__u16x8)wasm_u16x8_load8x8(src + stride);
            __u16x8 src3 = (__u16x8)wasm_u16x8_load8x8(src + stride + 1);
            __u16x8 src4 = (__u16x8)wasm_u16x8_load8x8(src + 2 * stride);
            __u16x8 src5 = (__u16x8)wasm_u16x8_load8x8(src + 2 * stride + 1);

            __u16x8 sum0 = (A128 * src0 + B128 * src1 + C128 * src2 + D128 * src3 + 32) >> 6;
            __u16x8 sum1 = (A128 * src2 + B128 * src3 + C128 * src4 + D128 * src5 + 32) >> 6;
            sum0 = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum1);
            wasm_v128_store64_lane(dst, (v128_t)sum0, 0);
            wasm_v128_store64_lane(dst + stride, (v128_t)sum0, 1);

            dst += 2 * stride;
            src += 2 * stride;
        }
        if (i * 2 != height) {
            __u16x8 src0 = (__u16x8)wasm_u16x8_load8x8(src);
            __u16x8 src1 = (__u16x8)wasm_u16x8_load8x8(src + 1);
            __u16x8 src2 = (__u16x8)wasm_u16x8_load8x8(src + stride);
            __u16x8 src3 = (__u16x8)wasm_u16x8_load8x8(src + stride + 1);
            __u16x8 sum0 = (A128 * src0 + B128 * src1 + C128 * src2 + D128 * src3 + 32) >> 6;
            sum0 = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum0);
            wasm_v128_store64_lane(dst, (v128_t)sum0, 0);
        }
    }
    else if (B + C) {
        const __u16x8 E128 = B128 + C128;
        const ptrdiff_t step = C ? stride : 1;
        for(i = 0; i < height2; i++) {
            __u16x8 src0 = (__u16x8)wasm_u16x8_load8x8(src);
            __u16x8 src1 = (__u16x8)wasm_u16x8_load8x8(src + step);
            __u16x8 src2 = (__u16x8)wasm_u16x8_load8x8(src + stride);
            __u16x8 src3 = (__u16x8)wasm_u16x8_load8x8(src + stride + step);

            __u16x8 sum0 = (A128 * src0 + E128 * src1 + 32) >> 6;
            __u16x8 sum1 = (A128 * src2 + E128 * src3 + 32) >> 6;
            sum0 = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum1);
            wasm_v128_store64_lane(dst, (v128_t)sum0, 0);
            wasm_v128_store64_lane(dst + stride, (v128_t)sum0, 1);

            dst += 2 * stride;
            src += 2 * stride;
        }
        if (i * 2 != height) {
            __u16x8 src0 = (__u16x8)wasm_u16x8_load8x8(src);
            __u16x8 src1 = (__u16x8)wasm_u16x8_load8x8(src + step);
            __u16x8 sum0 = (A128 * src0 + E128 * src1 + 32) >> 6;
            sum0 = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum0);
            wasm_v128_store64_lane(dst, (v128_t)sum0, 0);
        }
    }
    else {
        for (i = 0; i < height2; i++) {
            __u16x8 src0 = (__u16x8)wasm_u16x8_load8x8(src);
            __u16x8 src1 = (__u16x8)wasm_u16x8_load8x8(src + stride);
            
            __u16x8 sum0 = (A128 * src0 + 32) >> 6;
            __u16x8 sum1 = (A128 * src1 + 32) >> 6;
            sum0 = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum1);
            wasm_v128_store64_lane(dst, (v128_t)sum0, 0);
            wasm_v128_store64_lane(dst + stride, (v128_t)sum0, 1);

            dst += 2 * stride;
            src += 2 * stride;
        }
        if (i * 2 != height) {
            __u16x8 src0 = (__u16x8)wasm_u16x8_load8x8(src);
            __u16x8 sum0 = (A128 * src0 + 32) >> 6;
            sum0 = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum0);
            wasm_v128_store64_lane(dst, (v128_t)sum0, 0);
        }
    }
}

void ff_avg_h264_chroma_mc8_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride, int height, int x, int y) {
    const int A=(8-x)*(8-y);
    const int B=(  x)*(8-y);
    const int C=(8-x)*(  y);
    const int D=(  x)*(  y);

    const __u16x8 A128 = (__u16x8)wasm_u16x8_splat(A);
    const __u16x8 B128 = (__u16x8)wasm_u16x8_splat(B);
    const __u16x8 C128 = (__u16x8)wasm_u16x8_splat(C);
    const __u16x8 D128 = (__u16x8)wasm_u16x8_splat(D);

    int i;
    
    if(D) {
        for(i = 0; i < height; i++){
            __u16x8 dst128 = (__u16x8)wasm_u16x8_load8x8(dst);
            __u16x8 src0 = (__u16x8)wasm_u16x8_load8x8(src);
            __u16x8 src1 = (__u16x8)wasm_u16x8_load8x8(src + 1);
            __u16x8 src2 = (__u16x8)wasm_u16x8_load8x8(src + stride);
            __u16x8 src3 = (__u16x8)wasm_u16x8_load8x8(src + stride + 1);

            __u16x8 sum = A128 * src0 + B128 * src1 + C128 * src2 + D128 * src3;
            sum = (sum + 32) >> 6;
            sum = (dst128 + sum + 1) >> 1;
            sum = wasm_u8x16_narrow_i16x8((v128_t)sum, (v128_t)sum);
            wasm_v128_store64_lane(dst, (v128_t)sum, 0);

            dst += stride;
            src += stride;
        }
    }
    else if (B + C) {
        const __u16x8 E128= B128 + C128;
        const ptrdiff_t step = C ? stride : 1;
        for(i = 0; i < height; i++){
            __u16x8 dst128 = (__u16x8)wasm_u16x8_load8x8(dst);
            __u16x8 src0 = (__u16x8)wasm_u16x8_load8x8(src);
            __u16x8 src1 = (__u16x8)wasm_u16x8_load8x8(src + step);

            __u16x8 sum = A128 * src0 + E128 * src1;
            sum = (sum + 32) >> 6;
            sum = (dst128 + sum + 1) >> 1;
            sum = wasm_u8x16_narrow_i16x8((v128_t)sum, (v128_t)sum);
            wasm_v128_store64_lane(dst, (v128_t)sum, 0);

            dst += stride;
            src += stride;
        }
    }
    else {
        for (i = 0; i < height; i++){
            __u16x8 dst128 = (__u16x8)wasm_u16x8_load8x8(dst);
            __u16x8 src0 = (__u16x8)wasm_u16x8_load8x8(src);
            
            __u16x8 sum = A128 * src0;
            sum = (sum + 32) >> 6;
            sum = (dst128 + sum + 1) >> 1;
            sum = wasm_u8x16_narrow_i16x8((v128_t)sum, (v128_t)sum);
            wasm_v128_store64_lane(dst, (v128_t)sum, 0);

            dst += stride;
            src += stride;
        }
    }
}
