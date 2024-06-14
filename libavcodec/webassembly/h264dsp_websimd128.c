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
#include "h264dsp_webassembly.h"

#define INVERSE_VECTOR_16x16 \
    __u8x16 tl0 = (__u8x16)wasm_i8x16_shuffle(row0, row1, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    __u8x16 tl1 = (__u8x16)wasm_i8x16_shuffle(row2, row3, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    __u8x16 tl2 = (__u8x16)wasm_i8x16_shuffle(row4, row5, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    __u8x16 tl3 = (__u8x16)wasm_i8x16_shuffle(row6, row7, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    __u8x16 tl4 = (__u8x16)wasm_i8x16_shuffle(row8, row9, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    __u8x16 tl5 = (__u8x16)wasm_i8x16_shuffle(row10, row11, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    __u8x16 tl6 = (__u8x16)wasm_i8x16_shuffle(row12, row13, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    __u8x16 tl7 = (__u8x16)wasm_i8x16_shuffle(row14, row15, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    __u16x8 mll0 = (__u16x8)wasm_i16x8_shuffle(tl0, tl1, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mll1 = (__u16x8)wasm_i16x8_shuffle(tl2, tl3, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mll2 = (__u16x8)wasm_i16x8_shuffle(tl4, tl5, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mll3 = (__u16x8)wasm_i16x8_shuffle(tl6, tl7, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mlh0 = (__u16x8)wasm_i16x8_shuffle(tl0, tl1, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u16x8 mlh1 = (__u16x8)wasm_i16x8_shuffle(tl2, tl3, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u16x8 mlh2 = (__u16x8)wasm_i16x8_shuffle(tl4, tl5, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u16x8 mlh3 = (__u16x8)wasm_i16x8_shuffle(tl6, tl7, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u32x4 nlll0 = (__u32x4)wasm_i32x4_shuffle(mll0, mll1, 0, 4, 1, 5); \
    __u32x4 nlll1 = (__u32x4)wasm_i32x4_shuffle(mll2, mll3, 0, 4, 1, 5); \
    __u32x4 nllh0 = (__u32x4)wasm_i32x4_shuffle(mll0, mll1, 2, 6, 3, 7); \
    __u32x4 nllh1 = (__u32x4)wasm_i32x4_shuffle(mll2, mll3, 2, 6, 3, 7); \
    __u32x4 nlhl0 = (__u32x4)wasm_i32x4_shuffle(mlh0, mlh1, 0, 4, 1, 5); \
    __u32x4 nlhl1 = (__u32x4)wasm_i32x4_shuffle(mlh2, mlh3, 0, 4, 1, 5); \
    __u32x4 nlhh0 = (__u32x4)wasm_i32x4_shuffle(mlh0, mlh1, 2, 6, 3, 7); \
    __u32x4 nlhh1 = (__u32x4)wasm_i32x4_shuffle(mlh2, mlh3, 2, 6, 3, 7); \
    __u8x16 p3 = (__u8x16)wasm_i64x2_shuffle(nlll0, nlll1, 0, 2); \
    __u8x16 p2 = (__u8x16)wasm_i64x2_shuffle(nlll0, nlll1, 1, 3); \
    __u8x16 p1 = (__u8x16)wasm_i64x2_shuffle(nllh0, nllh1, 0, 2); \
    __u8x16 p0 = (__u8x16)wasm_i64x2_shuffle(nllh0, nllh1, 1, 3); \
    __u8x16 q0 = (__u8x16)wasm_i64x2_shuffle(nlhl0, nlhl1, 0, 2); \
    __u8x16 q1 = (__u8x16)wasm_i64x2_shuffle(nlhl0, nlhl1, 1, 3); \
    __u8x16 q2 = (__u8x16)wasm_i64x2_shuffle(nlhh0, nlhh1, 0, 2); \
    __u8x16 q3 = (__u8x16)wasm_i64x2_shuffle(nlhh0, nlhh1, 1, 3);

#define INVERSE_VECTOR_8x8 \
    __u16x8 mll0 = (__u16x8)wasm_i16x8_shuffle(row0, row1, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mll1 = (__u16x8)wasm_i16x8_shuffle(row2, row3, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mll2 = (__u16x8)wasm_i16x8_shuffle(row4, row5, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mll3 = (__u16x8)wasm_i16x8_shuffle(row6, row7, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mlh0 = (__u16x8)wasm_i16x8_shuffle(row0, row1, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u16x8 mlh1 = (__u16x8)wasm_i16x8_shuffle(row2, row3, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u16x8 mlh2 = (__u16x8)wasm_i16x8_shuffle(row4, row5, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u16x8 mlh3 = (__u16x8)wasm_i16x8_shuffle(row6, row7, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u32x4 nlll0 = (__u32x4)wasm_i32x4_shuffle(mll0, mll1, 0, 4, 1, 5); \
    __u32x4 nlll1 = (__u32x4)wasm_i32x4_shuffle(mll2, mll3, 0, 4, 1, 5); \
    __u32x4 nllh0 = (__u32x4)wasm_i32x4_shuffle(mll0, mll1, 2, 6, 3, 7); \
    __u32x4 nllh1 = (__u32x4)wasm_i32x4_shuffle(mll2, mll3, 2, 6, 3, 7); \
    __u32x4 nlhl0 = (__u32x4)wasm_i32x4_shuffle(mlh0, mlh1, 0, 4, 1, 5); \
    __u32x4 nlhl1 = (__u32x4)wasm_i32x4_shuffle(mlh2, mlh3, 0, 4, 1, 5); \
    __u32x4 nlhh0 = (__u32x4)wasm_i32x4_shuffle(mlh0, mlh1, 2, 6, 3, 7); \
    __u32x4 nlhh1 = (__u32x4)wasm_i32x4_shuffle(mlh2, mlh3, 2, 6, 3, 7); \
    __u16x8 p3 = (__u16x8)wasm_i64x2_shuffle(nlll0, nlll1, 0, 2); \
    __u16x8 p2 = (__u16x8)wasm_i64x2_shuffle(nlll0, nlll1, 1, 3); \
    __u16x8 p1 = (__u16x8)wasm_i64x2_shuffle(nllh0, nllh1, 0, 2); \
    __u16x8 p0 = (__u16x8)wasm_i64x2_shuffle(nllh0, nllh1, 1, 3); \
    __u16x8 q0 = (__u16x8)wasm_i64x2_shuffle(nlhl0, nlhl1, 0, 2); \
    __u16x8 q1 = (__u16x8)wasm_i64x2_shuffle(nlhl0, nlhl1, 1, 3); \
    __u16x8 q2 = (__u16x8)wasm_i64x2_shuffle(nlhh0, nlhh1, 0, 2); \
    __u16x8 q3 = (__u16x8)wasm_i64x2_shuffle(nlhh0, nlhh1, 1, 3);

#define INVERSE_VECTOR_8x8_P0Q0 \
    __u16x8 mll0 = (__u16x8)wasm_i16x8_shuffle(row0, row1, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mll1 = (__u16x8)wasm_i16x8_shuffle(row2, row3, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mll2 = (__u16x8)wasm_i16x8_shuffle(row4, row5, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mll3 = (__u16x8)wasm_i16x8_shuffle(row6, row7, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mlh0 = (__u16x8)wasm_i16x8_shuffle(row0, row1, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u16x8 mlh1 = (__u16x8)wasm_i16x8_shuffle(row2, row3, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u16x8 mlh2 = (__u16x8)wasm_i16x8_shuffle(row4, row5, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u16x8 mlh3 = (__u16x8)wasm_i16x8_shuffle(row6, row7, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u32x4 nllh0 = (__u32x4)wasm_i32x4_shuffle(mll0, mll1, 2, 6, 3, 7); \
    __u32x4 nllh1 = (__u32x4)wasm_i32x4_shuffle(mll2, mll3, 2, 6, 3, 7); \
    __u32x4 nlhl0 = (__u32x4)wasm_i32x4_shuffle(mlh0, mlh1, 0, 4, 1, 5); \
    __u32x4 nlhl1 = (__u32x4)wasm_i32x4_shuffle(mlh2, mlh3, 0, 4, 1, 5); \
    __u16x8 p0 = (__u16x8)wasm_i64x2_shuffle(nllh0, nllh1, 1, 3); \
    __u16x8 q0 = (__u16x8)wasm_i64x2_shuffle(nlhl0, nlhl1, 0, 2);

#define IN_INVERSE_VECTOR_16x16 \
    tl0 = (__u8x16)wasm_i8x16_shuffle(p3, p2, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    tl1 = (__u8x16)wasm_i8x16_shuffle(p1, p0, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    tl2 = (__u8x16)wasm_i8x16_shuffle(q0, q1, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    tl3 = (__u8x16)wasm_i8x16_shuffle(q2, q3, 0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23); \
    mll0 = (__u16x8)wasm_i16x8_shuffle(tl0, tl1, 0, 8, 1, 9, 2, 10, 3, 11); \
    mll1 = (__u16x8)wasm_i16x8_shuffle(tl2, tl3, 0, 8, 1, 9, 2, 10, 3, 11); \
    mlh0 = (__u16x8)wasm_i16x8_shuffle(tl0, tl1, 4, 12, 5, 13, 6, 14, 7, 15); \
    mlh1 = (__u16x8)wasm_i16x8_shuffle(tl2, tl3, 4, 12, 5, 13, 6, 14, 7, 15); \
    nlll0 = (__u32x4)wasm_i32x4_shuffle(mll0, mll1, 0, 4, 1, 5); \
    nllh0 = (__u32x4)wasm_i32x4_shuffle(mll0, mll1, 2, 6, 3, 7); \
    nlhl0 = (__u32x4)wasm_i32x4_shuffle(mlh0, mlh1, 0, 4, 1, 5); \
    nlhh0 = (__u32x4)wasm_i32x4_shuffle(mlh0, mlh1, 2, 6, 3, 7); \
    __u8x16 th0 = (__u8x16)wasm_i8x16_shuffle(p3, p2, 8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31); \
    __u8x16 th1 = (__u8x16)wasm_i8x16_shuffle(p1, p0, 8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31); \
    __u8x16 th2 = (__u8x16)wasm_i8x16_shuffle(q0, q1, 8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31); \
    __u8x16 th3 = (__u8x16)wasm_i8x16_shuffle(q2, q3, 8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31); \
    __u16x8 mhl0 = (__u16x8)wasm_i16x8_shuffle(th0, th1, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mhl1 = (__u16x8)wasm_i16x8_shuffle(th2, th3, 0, 8, 1, 9, 2, 10, 3, 11); \
    __u16x8 mhh0 = (__u16x8)wasm_i16x8_shuffle(th0, th1, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u16x8 mhh1 = (__u16x8)wasm_i16x8_shuffle(th2, th3, 4, 12, 5, 13, 6, 14, 7, 15); \
    __u32x4 nhll0 = (__u32x4)wasm_i32x4_shuffle(mhl0, mhl1, 0, 4, 1, 5); \
    __u32x4 nhlh0 = (__u32x4)wasm_i32x4_shuffle(mhl0, mhl1, 2, 6, 3, 7); \
    __u32x4 nhhl0 = (__u32x4)wasm_i32x4_shuffle(mhh0, mhh1, 0, 4, 1, 5); \
    __u32x4 nhhh0 = (__u32x4)wasm_i32x4_shuffle(mhh0, mhh1, 2, 6, 3, 7); \


#define IN_INVERSE_VECTOR_8x8 \
    mll0 = (__u16x8)wasm_i16x8_shuffle(p3, p2, 0, 8, 1, 9, 2, 10, 3, 11); \
    mll1 = (__u16x8)wasm_i16x8_shuffle(p1, p0, 0, 8, 1, 9, 2, 10, 3, 11); \
    mll2 = (__u16x8)wasm_i16x8_shuffle(q0, q1, 0, 8, 1, 9, 2, 10, 3, 11); \
    mll3 = (__u16x8)wasm_i16x8_shuffle(q2, q3, 0, 8, 1, 9, 2, 10, 3, 11); \
    mlh0 = (__u16x8)wasm_i16x8_shuffle(p3, p2, 4, 12, 5, 13, 6, 14, 7, 15); \
    mlh1 = (__u16x8)wasm_i16x8_shuffle(p1, p0, 4, 12, 5, 13, 6, 14, 7, 15); \
    mlh2 = (__u16x8)wasm_i16x8_shuffle(q0, q1, 4, 12, 5, 13, 6, 14, 7, 15); \
    mlh3 = (__u16x8)wasm_i16x8_shuffle(q3, q3, 4, 12, 5, 13, 6, 14, 7, 15); \
    nlll0 = (__u32x4)wasm_i32x4_shuffle(mll0, mll1, 0, 4, 1, 5); \
    nlll1 = (__u32x4)wasm_i32x4_shuffle(mll2, mll3, 0, 4, 1, 5); \
    nllh0 = (__u32x4)wasm_i32x4_shuffle(mll0, mll1, 2, 6, 3, 7); \
    nllh1 = (__u32x4)wasm_i32x4_shuffle(mll2, mll3, 2, 6, 3, 7); \
    nlhl0 = (__u32x4)wasm_i32x4_shuffle(mlh0, mlh1, 0, 4, 1, 5); \
    nlhl1 = (__u32x4)wasm_i32x4_shuffle(mlh2, mlh3, 0, 4, 1, 5); \
    nlhh0 = (__u32x4)wasm_i32x4_shuffle(mlh0, mlh1, 2, 6, 3, 7); \
    nlhh1 = (__u32x4)wasm_i32x4_shuffle(mlh2, mlh3, 2, 6, 3, 7); \
    p3 = (__u16x8)wasm_i64x2_shuffle(nlll0, nlll1, 0, 2); \
    p2 = (__u16x8)wasm_i64x2_shuffle(nlll0, nlll1, 1, 3); \
    p1 = (__u16x8)wasm_i64x2_shuffle(nllh0, nllh1, 0, 2); \
    p0 = (__u16x8)wasm_i64x2_shuffle(nllh0, nllh1, 1, 3); \
    q0 = (__u16x8)wasm_i64x2_shuffle(nlhl0, nlhl1, 0, 2); \
    q1 = (__u16x8)wasm_i64x2_shuffle(nlhl0, nlhl1, 1, 3); \
    q2 = (__u16x8)wasm_i64x2_shuffle(nlhh0, nlhh1, 0, 2); \
    q3 = (__u16x8)wasm_i64x2_shuffle(nlhh0, nlhh1, 1, 3); \
    row0 = wasm_u8x16_narrow_i16x8(p3, p2); \
    row1 = wasm_u8x16_narrow_i16x8(p1, p0); \
    row2 = wasm_u8x16_narrow_i16x8(q0, q1); \
    row3 = wasm_u8x16_narrow_i16x8(q2, q3); \


#define LOOP_FILTER_LUMA_16 \
    v128_t zero = {0}; \
    v128_t one = {0x01010101, 0x01010101, 0x01010101, 0x01010101}; \
    int8_t _tc0 = tc0[0]; \
    int8_t _tc1 = tc0[1]; \
    int8_t _tc2 = tc0[2]; \
    int8_t _tc3 = tc0[3]; \
    __i8x16 tc_vector = (__i8x16)wasm_i8x16_make( \
        _tc0, _tc0, _tc0, _tc0, \
        _tc1, _tc1, _tc1, _tc1, \
        _tc2, _tc2, _tc2, _tc2, \
        _tc3, _tc3, _tc3, _tc3 \
    ); \
    __i8x16 tc_vector_neg = (__i8x16)wasm_i8x16_neg((v128_t)tc_vector); \
    __i8x16 tc_not_zero = (__u8x16)wasm_i8x16_ne((v128_t)tc_vector, zero); \
    __u8x16 alpha_vector = (__u8x16)wasm_u8x16_splat(alpha); \
    __u8x16 beta_vector = (__u8x16)wasm_u8x16_splat(beta); \
    __u8x16 p0_asub_q0 = (__u8x16)wasm_u8x16_sub_sat( \
        wasm_u8x16_max((v128_t)p0, (v128_t)q0), \
        wasm_u8x16_min((v128_t)p0, (v128_t)q0) \
    ); \
    __u8x16 p1_asub_p0 = (__u8x16)wasm_u8x16_sub_sat( \
        wasm_u8x16_max((v128_t)p0, (v128_t)p1), \
        wasm_u8x16_min((v128_t)p0, (v128_t)p1) \
    ); \
    __u8x16 q1_asub_q0 = (__u8x16)wasm_u8x16_sub_sat( \
        wasm_u8x16_max((v128_t)q1, (v128_t)q0), \
        wasm_u8x16_min((v128_t)q1, (v128_t)q0) \
    ); \
    __u8x16 p2_asub_p0 = (__u8x16)wasm_u8x16_sub_sat( \
        wasm_u8x16_max((v128_t)p0, (v128_t)p2), \
        wasm_u8x16_min((v128_t)p0, (v128_t)p2) \
    ); \
    __u8x16 q2_asub_q0 = (__u8x16)wasm_u8x16_sub_sat( \
        wasm_u8x16_max((v128_t)q2, (v128_t)q0), \
        wasm_u8x16_min((v128_t)q2, (v128_t)q0) \
    ); \
    /* 计算滤波条件 */ \
    __u8x16 p0_asub_q0_less_then_alpha = (__u8x16)wasm_u8x16_lt((v128_t)p0_asub_q0, (v128_t)alpha_vector); \
    __u8x16 p1_asub_p0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)p1_asub_p0, (v128_t)beta_vector); \
    __u8x16 q1_asub_q0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)q1_asub_q0, (v128_t)beta_vector); \
    __u8x16 tc_orig_less_then_zero = (__u8x16)wasm_i8x16_ge((v128_t)tc_vector, (v128_t)zero); \
    /* p0,q0 是否滤波条件 */ \
    __u8x16 p0_q0_filter_bool = p0_asub_q0_less_then_alpha & p1_asub_p0_less_then_beta & q1_asub_q0_less_then_beta & tc_orig_less_then_zero; \
    __u8x16 p2_asub_p0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)p2_asub_p0, (v128_t)beta_vector); \
    __u8x16 q2_asub_q0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)q2_asub_q0, (v128_t)beta_vector); \
    /* p1 是否滤波条件 */ \
    __u8x16 p1_filter_bool = p0_q0_filter_bool & p2_asub_p0_less_then_beta & tc_not_zero; \
    /* q1 是否滤波条件 */ \
    __u8x16 q1_filter_bool = p0_q0_filter_bool & q2_asub_q0_less_then_beta & tc_not_zero; \
    __i8x16 p1_filter_tc = (__i8x16)wasm_v128_bitselect(one, zero, (v128_t)p2_asub_p0_less_then_beta); \
    __i8x16 q1_filter_tc = (__i8x16)wasm_v128_bitselect(one, zero, (v128_t)q2_asub_q0_less_then_beta); \
    __i8x16 p0_q0_filter_tc = tc_vector + p1_filter_tc + q1_filter_tc; \
    __i8x16 p0_q0_filter_tc_neg = (__i8x16)wasm_i8x16_neg((v128_t)p0_q0_filter_tc); \
    __i16x8 p0_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)p0); \
    __i16x8 p0_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)p0); \
    __i16x8 q0_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)q0); \
    __i16x8 q0_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)q0); \
    __i16x8 p1_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)p1); \
    __i16x8 p1_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)p1); \
    __i16x8 q1_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)q1); \
    __i16x8 q1_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)q1); \
    __i16x8 p2_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)p2); \
    __i16x8 p2_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)p2); \
    __i16x8 q2_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)q2); \
    __i16x8 q2_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)q2); \
    __i16x8 px_low; \
    __i16x8 px_high; \
    __i16x8 avgr_low; \
    __i16x8 avgr_high; \
    /* 计算 p1 */ \
    avgr_low = ((p2_low + ((p0_low + q0_low + 1) >> 1)) >> 1) - p1_low; \
    avgr_low = wasm_i16x8_max((v128_t)avgr_low, (v128_t)wasm_i16x8_extend_low_i8x16(tc_vector_neg)); \
    avgr_low = wasm_i16x8_min((v128_t)avgr_low, (v128_t)wasm_i16x8_extend_low_i8x16(tc_vector)); \
    px_low = p1_low + avgr_low; \
    avgr_high = ((p2_high + ((p0_high + q0_high + 1) >> 1)) >> 1) - p1_high; \
    avgr_high = wasm_i16x8_max((v128_t)avgr_high, (v128_t)wasm_i16x8_extend_high_i8x16(tc_vector_neg)); \
    avgr_high = wasm_i16x8_min((v128_t)avgr_high, (v128_t)wasm_i16x8_extend_high_i8x16(tc_vector)); \
    px_high = p1_high + avgr_high; \
    __u8x16 p1_filter = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high); \
    /* 计算 q1 */ \
    avgr_low = ((q2_low + ((p0_low + q0_low + 1) >> 1)) >> 1) - q1_low; \
    avgr_low = wasm_i16x8_max((v128_t)avgr_low, (v128_t)wasm_i16x8_extend_low_i8x16(tc_vector_neg)); \
    avgr_low = wasm_i16x8_min((v128_t)avgr_low, (v128_t)wasm_i16x8_extend_low_i8x16(tc_vector)); \
    px_low = q1_low + avgr_low; \
    avgr_high = ((q2_high + ((p0_high + q0_high + 1) >> 1)) >> 1) - q1_high; \
    avgr_high = wasm_i16x8_max((v128_t)avgr_high, (v128_t)wasm_i16x8_extend_high_i8x16(tc_vector_neg)); \
    avgr_high = wasm_i16x8_min((v128_t)avgr_high, (v128_t)wasm_i16x8_extend_high_i8x16(tc_vector)); \
    px_high = q1_high + avgr_high; \
    __u8x16 q1_filter = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high); \
    px_low = ((q0_low - p0_low) * 4 + (p1_low - q1_low) + 4) >> 3; \
    px_low = wasm_i16x8_max((v128_t)px_low, (v128_t)wasm_i16x8_extend_low_i8x16(p0_q0_filter_tc_neg)); \
    px_low = wasm_i16x8_min((v128_t)px_low, (v128_t)wasm_i16x8_extend_low_i8x16(p0_q0_filter_tc)); \
    px_high = ((q0_high - p0_high) * 4 + (p1_high - q1_high) + 4) >> 3; \
    px_high = wasm_i16x8_max((v128_t)px_high, (v128_t)wasm_i16x8_extend_high_i8x16(p0_q0_filter_tc_neg)); \
    px_high = wasm_i16x8_min((v128_t)px_high, (v128_t)wasm_i16x8_extend_high_i8x16(p0_q0_filter_tc)); \
    __u8x16 p0_filter = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)(p0_low + px_low), (v128_t)(p0_high + px_high)); \
    __u8x16 q0_filter = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)(q0_low - px_low), (v128_t)(q0_high - px_high)); \
    p0 = wasm_v128_bitselect((v128_t)p0_filter, (v128_t)p0, (v128_t)p0_q0_filter_bool); \
    q0 = wasm_v128_bitselect((v128_t)q0_filter, (v128_t)q0, (v128_t)p0_q0_filter_bool); \
    p1 = wasm_v128_bitselect((v128_t)p1_filter, (v128_t)p1, (v128_t)p1_filter_bool); \
    q1 = wasm_v128_bitselect((v128_t)q1_filter, (v128_t)q1, (v128_t)q1_filter_bool);


#define LOOP_FILTER_LUMA_8 \
    v128_t zero = {0}; \
    v128_t one = {0x01010101, 0x01010101, 0x01010101, 0x01010101}; \
    int8_t _tc0 = tc0[0]; \
    int8_t _tc1 = tc0[1]; \
    int8_t _tc2 = tc0[2]; \
    int8_t _tc3 = tc0[3]; \
    __i16x8 tc_vector = (__i16x8)wasm_i16x8_make( \
        _tc0, _tc0, _tc1, _tc1, \
        _tc2, _tc2, _tc3, _tc3 \
    ); \
    __i16x8 tc_vector_neg = (__i16x8)wasm_i16x8_neg((v128_t)tc_vector); \
    __i16x8 tc_not_zero = (__u16x8)wasm_i16x8_ne((v128_t)tc_vector, zero); \
    __u16x8 alpha_vector = (__u16x8)wasm_u16x8_splat(alpha); \
    __u16x8 beta_vector = (__u16x8)wasm_u16x8_splat(beta); \
    __u16x8 p0_asub_q0 = (__u16x8)wasm_u16x8_sub_sat( \
        wasm_u16x8_max((v128_t)p0, (v128_t)q0), \
        wasm_u16x8_min((v128_t)p0, (v128_t)q0) \
    ); \
    __u16x8 p1_asub_p0 = (__u16x8)wasm_u16x8_sub_sat( \
        wasm_u16x8_max((v128_t)p0, (v128_t)p1), \
        wasm_u16x8_min((v128_t)p0, (v128_t)p1) \
    ); \
    __u16x8 q1_asub_q0 = (__u16x8)wasm_u16x8_sub_sat( \
        wasm_u16x8_max((v128_t)q1, (v128_t)q0), \
        wasm_u16x8_min((v128_t)q1, (v128_t)q0) \
    ); \
    __u16x8 p2_asub_p0 = (__u16x8)wasm_u16x8_sub_sat( \
        wasm_u16x8_max((v128_t)p0, (v128_t)p2), \
        wasm_u16x8_min((v128_t)p0, (v128_t)p2) \
    ); \
    __u16x8 q2_asub_q0 = (__u16x8)wasm_u16x8_sub_sat( \
        wasm_u16x8_max((v128_t)q2, (v128_t)q0), \
        wasm_u16x8_min((v128_t)q2, (v128_t)q0) \
    ); \
    /* 计算滤波条件 */ \
    __u16x8 p0_asub_q0_less_then_alpha = (__u16x8)wasm_u16x8_lt((v128_t)p0_asub_q0, (v128_t)alpha_vector); \
    __u16x8 p1_asub_p0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)p1_asub_p0, (v128_t)beta_vector); \
    __u16x8 q1_asub_q0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)q1_asub_q0, (v128_t)beta_vector); \
    __u16x8 tc_orig_less_then_zero = (__u16x8)wasm_i16x8_ge((v128_t)tc_vector, (v128_t)zero); \
    /* p0,q0 是否滤波条件 */ \
    __u16x8 p0_q0_filter_bool = p0_asub_q0_less_then_alpha & p1_asub_p0_less_then_beta & q1_asub_q0_less_then_beta & tc_orig_less_then_zero; \
    __u16x8 p2_asub_p0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)p2_asub_p0, (v128_t)beta_vector); \
    __u16x8 q2_asub_q0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)q2_asub_q0, (v128_t)beta_vector); \
    /* p1 是否滤波条件 */ \
    __u16x8 p1_filter_bool = p0_q0_filter_bool & p2_asub_p0_less_then_beta & tc_not_zero; \
    /* q1 是否滤波条件 */ \
    __u16x8 q1_filter_bool = p0_q0_filter_bool & q2_asub_q0_less_then_beta & tc_not_zero; \
    __i16x8 p1_filter_tc = (__i16x8)wasm_v128_bitselect(one, zero, (v128_t)p2_asub_p0_less_then_beta); \
    __i16x8 q1_filter_tc = (__i16x8)wasm_v128_bitselect(one, zero, (v128_t)q2_asub_q0_less_then_beta); \
    __i16x8 p0_q0_filter_tc = tc_vector + p1_filter_tc + q1_filter_tc; \
    __i16x8 p0_q0_filter_tc_neg = (__i16x8)wasm_i16x8_neg((v128_t)p0_q0_filter_tc); \
    __u16x8 avgr; \
    /* 计算 p1 */ \
    avgr = ((p2 + ((p0 + q0 + 1) >> 1)) >> 1) - p1; \
    avgr = wasm_i16x8_max((v128_t)avgr, (v128_t)tc_vector_neg); \
    avgr = wasm_i16x8_min((v128_t)avgr, (v128_t)tc_vector); \
    __u16x8 p1_filter = p1 + avgr; \
    /* 计算 q1 */ \
    avgr = ((q2 + ((p0 + q0 + 1) >> 1)) >> 1) - q1; \
    avgr = wasm_i16x8_max((v128_t)avgr, (v128_t)tc_vector_neg); \
    avgr = wasm_i16x8_min((v128_t)avgr, (v128_t)tc_vector); \
    __u16x8 q1_filter = q1 + avgr; \
    avgr = ((q0 - p0) * 4 + (p1 - q1) + 4) >> 3; \
    avgr = wasm_i16x8_max((v128_t)avgr, (v128_t)p0_q0_filter_tc_neg); \
    avgr = wasm_i16x8_min((v128_t)avgr, (v128_t)p0_q0_filter_tc); \
    __u16x8 p0_filter = p0 + avgr; \
    __u16x8 q0_filter = q0 - avgr; \
    p0 = wasm_v128_bitselect((v128_t)p0_filter, (v128_t)p0, (v128_t)p0_q0_filter_bool); \
    q0 = wasm_v128_bitselect((v128_t)q0_filter, (v128_t)q0, (v128_t)p0_q0_filter_bool); \
    p1 = wasm_v128_bitselect((v128_t)p1_filter, (v128_t)p1, (v128_t)p1_filter_bool); \
    q1 = wasm_v128_bitselect((v128_t)q1_filter, (v128_t)q1, (v128_t)q1_filter_bool);

void ff_h264_v_loop_filter_luma_8_websimd128(uint8_t *pix /*align 16*/, ptrdiff_t stride, int alpha, int beta, int8_t *tc0) {
    __u8x16 p0 = (__u8x16)wasm_v128_load(pix - stride);
    __u8x16 p1 = (__u8x16)wasm_v128_load(pix - 2 * stride);
    __u8x16 p2 = (__u8x16)wasm_v128_load(pix - 3 * stride);
    __u8x16 q0 = (__u8x16)wasm_v128_load(pix);
    __u8x16 q1 = (__u8x16)wasm_v128_load(pix + stride);
    __u8x16 q2 = (__u8x16)wasm_v128_load(pix + 2 * stride);

    LOOP_FILTER_LUMA_16;

    wasm_v128_store(pix - stride, (v128_t)p0);
    wasm_v128_store(pix, (v128_t)q0);
    wasm_v128_store(pix - 2 * stride, (v128_t)p1);
    wasm_v128_store(pix + stride, (v128_t)q1);
}
void ff_h264_h_loop_filter_luma_8_websimd128(uint8_t *pix /*align 4 */, ptrdiff_t stride, int alpha, int beta, int8_t *tc0) {
    uint8_t* start = pix - 4;
    __u8x16 row0 = (__u8x16)wasm_v128_load64_zero(start);
    __u8x16 row1 = (__u8x16)wasm_v128_load64_zero(start + stride);
    __u8x16 row2 = (__u8x16)wasm_v128_load64_zero(start + 2 * stride);
    __u8x16 row3 = (__u8x16)wasm_v128_load64_zero(start + 3 * stride);
    __u8x16 row4 = (__u8x16)wasm_v128_load64_zero(start + 4 * stride);
    __u8x16 row5 = (__u8x16)wasm_v128_load64_zero(start + 5 * stride);
    __u8x16 row6 = (__u8x16)wasm_v128_load64_zero(start + 6 * stride);
    __u8x16 row7 = (__u8x16)wasm_v128_load64_zero(start + 7 * stride);
    __u8x16 row8 = (__u8x16)wasm_v128_load64_zero(start + 8 * stride);
    __u8x16 row9 = (__u8x16)wasm_v128_load64_zero(start + 9 * stride);
    __u8x16 row10 = (__u8x16)wasm_v128_load64_zero(start + 10 * stride);
    __u8x16 row11 = (__u8x16)wasm_v128_load64_zero(start + 11 * stride);
    __u8x16 row12 = (__u8x16)wasm_v128_load64_zero(start + 12 * stride);
    __u8x16 row13 = (__u8x16)wasm_v128_load64_zero(start + 13 * stride);
    __u8x16 row14 = (__u8x16)wasm_v128_load64_zero(start + 14 * stride);
    __u8x16 row15 = (__u8x16)wasm_v128_load64_zero(start + 15 * stride);

    INVERSE_VECTOR_16x16;

    LOOP_FILTER_LUMA_16;

    IN_INVERSE_VECTOR_16x16;

    nlll0 = (__i64x2)nlll0 >> 16;
    nllh0 = (__i64x2)nllh0 >> 16;
    nlhl0 = (__i64x2)nlhl0 >> 16;
    nlhh0 = (__i64x2)nlhh0 >> 16;
    nhll0 = (__i64x2)nhll0 >> 16;
    nhlh0 = (__i64x2)nhlh0 >> 16;
    nhhl0 = (__i64x2)nhhl0 >> 16;
    nhhh0 = (__i64x2)nhhh0 >> 16;
   
    wasm_v128_store32_lane(pix - 2, nlll0, 0);
    wasm_v128_store32_lane(pix - 2 + stride, nlll0, 2);

    wasm_v128_store32_lane(pix - 2 + 2 * stride, nllh0, 0);
    wasm_v128_store32_lane(pix - 2 + 3 * stride, nllh0, 2);

    wasm_v128_store32_lane(pix - 2 + 4 * stride, nlhl0, 0);
    wasm_v128_store32_lane(pix - 2 + 5 * stride, nlhl0, 2);

    wasm_v128_store32_lane(pix - 2 + 6 * stride, nlhh0, 0);
    wasm_v128_store32_lane(pix - 2 + 7 * stride, nlhh0, 2);

    wasm_v128_store32_lane(pix - 2 + 8 * stride, nhll0, 0);
    wasm_v128_store32_lane(pix - 2 + 9 * stride, nhll0, 2);

    wasm_v128_store32_lane(pix - 2 + 10 * stride, nhlh0, 0);
    wasm_v128_store32_lane(pix - 2 + 11 * stride, nhlh0, 2);

    wasm_v128_store32_lane(pix - 2 + 12 * stride, nhhl0, 0);
    wasm_v128_store32_lane(pix - 2 + 13 * stride, nhhl0, 2);

    wasm_v128_store32_lane(pix - 2 + 14 * stride, nhhh0, 0);
    wasm_v128_store32_lane(pix - 2 + 15 * stride, nhhh0, 2);
}

void ff_h264_h_loop_filter_luma_mbaff_8_websimd128(uint8_t *pix /*align 16*/, ptrdiff_t stride, int alpha, int beta, int8_t *tc0) {
    uint8_t* start = pix - 4;
    __u8x16 row0 = (__u8x16)wasm_u16x8_load8x8(start);
    __u8x16 row1 = (__u8x16)wasm_u16x8_load8x8(start + stride);
    __u8x16 row2 = (__u8x16)wasm_u16x8_load8x8(start + 2 * stride);
    __u8x16 row3 = (__u8x16)wasm_u16x8_load8x8(start + 3 * stride);
    __u8x16 row4 = (__u8x16)wasm_u16x8_load8x8(start + 4 * stride);
    __u8x16 row5 = (__u8x16)wasm_u16x8_load8x8(start + 5 * stride);
    __u8x16 row6 = (__u8x16)wasm_u16x8_load8x8(start + 6 * stride);
    __u8x16 row7 = (__u8x16)wasm_u16x8_load8x8(start + 7 * stride);

    INVERSE_VECTOR_8x8;

    LOOP_FILTER_LUMA_8;

    IN_INVERSE_VECTOR_8x8;

    row0 = (__i64x2)row0 >> 16;
    row1 = (__i64x2)row1 >> 16;
    row2 = (__i64x2)row2 >> 16;
    row3 = (__i64x2)row3 >> 16;

    wasm_v128_store32_lane(pix - 2, (v128_t)row0, 0);
    wasm_v128_store32_lane(pix - 2 + stride, (v128_t)row0, 1);
    wasm_v128_store32_lane(pix - 2 + 2 * stride, (v128_t)row1, 0);
    wasm_v128_store32_lane(pix - 2 + 3 * stride, (v128_t)row1, 1);
    wasm_v128_store32_lane(pix - 2 + 4 * stride, (v128_t)row2, 0);
    wasm_v128_store32_lane(pix - 2 + 5 * stride, (v128_t)row2, 1);
    wasm_v128_store32_lane(pix - 2 + 6 * stride, (v128_t)row3, 0);
    wasm_v128_store32_lane(pix - 2 + 7 * stride, (v128_t)row3, 1);
}

#define LOOP_FILTER_LUMA_INTRA_16 \
    __u8x16 alpha_vector = (__u8x16)wasm_u8x16_splat(alpha); \
    __u8x16 alpha_2_vector = (alpha_vector >> 2) + 2; \
    __u8x16 beta_vector = (__u8x16)wasm_u8x16_splat(beta); \
\
    __u8x16 p0_asub_q0 = (__u8x16)wasm_u8x16_sub_sat( \
        wasm_u8x16_max((v128_t)p0, (v128_t)q0), \
        wasm_u8x16_min((v128_t)p0, (v128_t)q0) \
    ); \
    __u8x16 p1_asub_p0 = (__u8x16)wasm_u8x16_sub_sat( \
        wasm_u8x16_max((v128_t)p0, (v128_t)p1), \
        wasm_u8x16_min((v128_t)p0, (v128_t)p1) \
    ); \
    __u8x16 q1_asub_q0 = (__u8x16)wasm_u8x16_sub_sat( \
        wasm_u8x16_max((v128_t)q1, (v128_t)q0), \
        wasm_u8x16_min((v128_t)q1, (v128_t)q0) \
    ); \
    __u8x16 p2_asub_p0 = (__u8x16)wasm_u8x16_sub_sat( \
        wasm_u8x16_max((v128_t)p0, (v128_t)p2), \
        wasm_u8x16_min((v128_t)p0, (v128_t)p2) \
    ); \
    __u8x16 q2_asub_q0 = (__u8x16)wasm_u8x16_sub_sat( \
        wasm_u8x16_max((v128_t)q2, (v128_t)q0), \
        wasm_u8x16_min((v128_t)q2, (v128_t)q0) \
    ); \
    /* 计算滤波条件 */ \
    __u8x16 p0_asub_q0_less_then_alpha = (__u8x16)wasm_u8x16_lt((v128_t)p0_asub_q0, (v128_t)alpha_vector); \
    __u8x16 p0_asub_q0_less_then_alpha_2 = (__u8x16)wasm_u8x16_lt((v128_t)p0_asub_q0, (v128_t)alpha_2_vector); \
    __u8x16 p1_asub_p0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)p1_asub_p0, (v128_t)beta_vector); \
    __u8x16 p2_asub_p0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)p2_asub_p0, (v128_t)beta_vector); \
    __u8x16 q1_asub_q0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)q1_asub_q0, (v128_t)beta_vector); \
    __u8x16 q2_asub_q0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)q2_asub_q0, (v128_t)beta_vector); \
    __u8x16 filter_bool = p0_asub_q0_less_then_alpha & p1_asub_p0_less_then_beta & q1_asub_q0_less_then_beta; \
    __u8x16 filter_bool_2 = filter_bool & p0_asub_q0_less_then_alpha_2; \
    __u8x16 filter_bool_p1_p2 = filter_bool_2 & p2_asub_p0_less_then_beta; \
    __u8x16 filter_bool_q1_q2 = filter_bool_2 & q2_asub_q0_less_then_beta; \
\
    __i16x8 p0_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)p0); \
    __i16x8 p0_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)p0); \
    __i16x8 q0_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)q0); \
    __i16x8 q0_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)q0); \
    __i16x8 p1_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)p1); \
    __i16x8 p1_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)p1); \
    __i16x8 q1_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)q1); \
    __i16x8 q1_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)q1); \
    __i16x8 p2_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)p2); \
    __i16x8 p2_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)p2); \
    __i16x8 q2_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)q2); \
    __i16x8 q2_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)q2); \
    __i16x8 p3_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)p3); \
    __i16x8 p3_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)p3); \
    __i16x8 q3_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)q3); \
    __i16x8 q3_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)q3); \
\
    __i16x8 px_low; \
    __i16x8 px_high; \
\
    /* 计算 p2-p0 条件的 p0 */ \
    px_low = (p2_low + 2 * p1_low + 2 * p0_low + 2 * q0_low + q1_low + 4) >> 3; \
    px_high = (p2_high + 2 * p1_high + 2 * p0_high + 2 * q0_high + q1_high + 4) >> 3; \
    __u8x16 p0_filter_p2_p0 = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high); \
\
    /* 计算 p2-p0 条件的 p1 */ \
    px_low = (p2_low + p1_low + p0_low + q0_low + 2) >> 2; \
    px_high = (p2_high + p1_high + p0_high + q0_high + 2) >> 2; \
    __u8x16 p1_filter_p2_p0 = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high); \
\
    /* 计算 p2-p0 条件的 p2 */ \
    px_low = (2 * p3_low + 3 * p2_low + p1_low + p0_low + q0_low + 4) >> 3; \
    px_high = (2 * p3_high + 3 * p2_high + p1_high + p0_high + q0_high + 4) >> 3; \
    __u8x16 p2_filter_p2_p0 = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high); \
\
    /* 计算非 p2-p0 条件的 p0 */ \
    px_low = (2 * p1_low + p0_low + q1_low + 2) >> 2; \
    px_high = (2 * p1_high + p0_high + q1_high + 2) >> 2; \
    __u8x16 p0_filter_p2_p0_not = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high); \
\
    /* 计算 q2-q0 条件的 q0 */ \
    px_low = (p1_low + 2 * p0_low + 2 * q0_low + 2 * q1_low + q2_low + 4) >> 3; \
    px_high = (p1_high + 2 * p0_high + 2 * q0_high + 2 * q1_high + q2_high + 4) >> 3; \
    __u8x16 q0_filter_q2_q0 = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high); \
\
    /* 计算 q2-q0 条件的 q1 */ \
    px_low = (p0_low + q0_low + q1_low + q2_low + 2) >> 2; \
    px_high = (p0_high + q0_high + q1_high + q2_high + 2) >> 2; \
    __u8x16 q1_filter_q2_q0 = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high); \
\
    /* 计算 q2-q0 条件的 p2 */ \
    px_low = (2 * q3_low + 3 * q2_low + q1_low + q0_low + p0_low + 4) >> 3; \
    px_high = (2 * q3_high + 3 * q2_high + q1_high + q0_high + p0_high + 4) >> 3; \
    __u8x16 q2_filter_q2_q0 = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high); \
\
    /* 计算非 q2-q0 条件的 q0 */ \
    px_low = (2 * q1_low + q0_low + p1_low + 2) >> 2; \
    px_high = (2 * q1_high + q0_high + p1_high + 2) >> 2; \
    __u8x16 q0_filter_q2_q0_not = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high); \
\
    p0 = wasm_v128_bitselect( \
        wasm_v128_bitselect( \
            wasm_v128_bitselect( \
                (v128_t)p0_filter_p2_p0, \
                (v128_t)p0_filter_p2_p0_not, \
                (v128_t)filter_bool_p1_p2 \
            ), \
            (v128_t)p0_filter_p2_p0_not, \
            (v128_t)filter_bool_2 \
        ), \
        (v128_t)p0, \
        (v128_t)filter_bool \
    ); \
    q0 = wasm_v128_bitselect( \
        wasm_v128_bitselect( \
            wasm_v128_bitselect( \
                (v128_t)q0_filter_q2_q0, \
                (v128_t)q0_filter_q2_q0_not, \
                (v128_t)filter_bool_q1_q2 \
            ), \
            (v128_t)q0_filter_q2_q0_not, \
            (v128_t)filter_bool_2 \
        ), \
        (v128_t)q0, \
        (v128_t)filter_bool \
    ); \
    p1 = wasm_v128_bitselect((v128_t)p1_filter_p2_p0, (v128_t)p1, (v128_t)filter_bool_p1_p2); \
    p2 = wasm_v128_bitselect((v128_t)p2_filter_p2_p0, (v128_t)p2, (v128_t)filter_bool_p1_p2); \
    q1 = wasm_v128_bitselect((v128_t)q1_filter_q2_q0, (v128_t)q1, (v128_t)filter_bool_q1_q2); \
    q2 = wasm_v128_bitselect((v128_t)q2_filter_q2_q0, (v128_t)q2, (v128_t)filter_bool_q1_q2);

/* v/h_loop_filter_luma_intra: align 16 */
void ff_h264_v_loop_filter_luma_intra_8_websimd128(uint8_t *pix, ptrdiff_t stride, int alpha, int beta) {
    __u8x16 p0 = (__u8x16)wasm_v128_load(pix - stride);
    __u8x16 p1 = (__u8x16)wasm_v128_load(pix - 2 * stride);
    __u8x16 p2 = (__u8x16)wasm_v128_load(pix - 3 * stride);
    __u8x16 p3 = (__u8x16)wasm_v128_load(pix - 4 * stride);
    __u8x16 q0 = (__u8x16)wasm_v128_load(pix);
    __u8x16 q1 = (__u8x16)wasm_v128_load(pix + stride);
    __u8x16 q2 = (__u8x16)wasm_v128_load(pix + 2 * stride);
    __u8x16 q3 = (__u8x16)wasm_v128_load(pix + 3 * stride);

    LOOP_FILTER_LUMA_INTRA_16;

    wasm_v128_store(pix - stride, (v128_t)p0);
    wasm_v128_store(pix - 2 * stride, (v128_t)p1);
    wasm_v128_store(pix - 3 * stride, (v128_t)p2);
    wasm_v128_store(pix, (v128_t)q0);
    wasm_v128_store(pix + stride, (v128_t)q1);
    wasm_v128_store(pix + 2 * stride, (v128_t)q2);
}
void ff_h264_h_loop_filter_luma_intra_8_websimd128(uint8_t *pix, ptrdiff_t stride, int alpha, int beta) {
    uint8_t* start = pix - 4;
    __u8x16 row0 = (__u8x16)wasm_v128_load64_zero(start);
    __u8x16 row1 = (__u8x16)wasm_v128_load64_zero(start + stride);
    __u8x16 row2 = (__u8x16)wasm_v128_load64_zero(start + 2 * stride);
    __u8x16 row3 = (__u8x16)wasm_v128_load64_zero(start + 3 * stride);
    __u8x16 row4 = (__u8x16)wasm_v128_load64_zero(start + 4 * stride);
    __u8x16 row5 = (__u8x16)wasm_v128_load64_zero(start + 5 * stride);
    __u8x16 row6 = (__u8x16)wasm_v128_load64_zero(start + 6 * stride);
    __u8x16 row7 = (__u8x16)wasm_v128_load64_zero(start + 7 * stride);
    __u8x16 row8 = (__u8x16)wasm_v128_load64_zero(start + 8 * stride);
    __u8x16 row9 = (__u8x16)wasm_v128_load64_zero(start + 9 * stride);
    __u8x16 row10 = (__u8x16)wasm_v128_load64_zero(start + 10 * stride);
    __u8x16 row11 = (__u8x16)wasm_v128_load64_zero(start + 11 * stride);
    __u8x16 row12 = (__u8x16)wasm_v128_load64_zero(start + 12 * stride);
    __u8x16 row13 = (__u8x16)wasm_v128_load64_zero(start + 13 * stride);
    __u8x16 row14 = (__u8x16)wasm_v128_load64_zero(start + 14 * stride);
    __u8x16 row15 = (__u8x16)wasm_v128_load64_zero(start + 15 * stride);

    INVERSE_VECTOR_16x16;

    LOOP_FILTER_LUMA_INTRA_16;

    IN_INVERSE_VECTOR_16x16;

    nlll0 = (__i64x2)nlll0 >> 8;
    nllh0 = (__i64x2)nllh0 >> 8;
    nlhl0 = (__i64x2)nlhl0 >> 8;
    nlhh0 = (__i64x2)nlhh0 >> 8;
    nhll0 = (__i64x2)nhll0 >> 8;
    nhlh0 = (__i64x2)nhlh0 >> 8;
    nhhl0 = (__i64x2)nhhl0 >> 8;
    nhhh0 = (__i64x2)nhhh0 >> 8;
    
   
    wasm_v128_store32_lane(pix - 3, nlll0, 0);
    wasm_v128_store16_lane(pix + 1, nlll0, 2);
    wasm_v128_store32_lane(pix - 3 + stride, nlll0, 2);
    wasm_v128_store16_lane(pix + 1 + stride, nlll0, 6);

    wasm_v128_store32_lane(pix - 3 + 2 * stride, nllh0, 0);
    wasm_v128_store16_lane(pix + 1 + 2 * stride, nllh0, 2);
    wasm_v128_store32_lane(pix - 3 + 3 * stride, nllh0, 2);
    wasm_v128_store16_lane(pix + 1 + 3 * stride, nllh0, 6);

    wasm_v128_store32_lane(pix - 3 + 4 * stride, nlhl0, 0);
    wasm_v128_store16_lane(pix + 1 + 4 * stride, nlhl0, 2);
    wasm_v128_store32_lane(pix - 3 + 5 * stride, nlhl0, 2);
    wasm_v128_store16_lane(pix + 1 + 5 * stride, nlhl0, 6);

    wasm_v128_store32_lane(pix - 3 + 6 * stride, nlhh0, 0);
    wasm_v128_store16_lane(pix + 1 + 6 * stride, nlhh0, 2);
    wasm_v128_store32_lane(pix - 3 + 7 * stride, nlhh0, 2);
    wasm_v128_store16_lane(pix + 1 + 7 * stride, nlhh0, 6);

    wasm_v128_store32_lane(pix - 3 + 8 * stride, nhll0, 0);
    wasm_v128_store16_lane(pix + 1 + 8 * stride, nhll0, 2);
    wasm_v128_store32_lane(pix - 3 + 9 * stride, nhll0, 2);
    wasm_v128_store16_lane(pix + 1 + 9 * stride, nhll0, 6);

    wasm_v128_store32_lane(pix - 3 + 10 * stride, nhlh0, 0);
    wasm_v128_store16_lane(pix + 1 + 10 * stride, nhlh0, 2);
    wasm_v128_store32_lane(pix - 3 + 11 * stride, nhlh0, 2);
    wasm_v128_store16_lane(pix + 1 + 11 * stride, nhlh0, 6);

    wasm_v128_store32_lane(pix - 3 + 12 * stride, nhhl0, 0);
    wasm_v128_store16_lane(pix + 1 + 12 * stride, nhhl0, 2);
    wasm_v128_store32_lane(pix - 3 + 13 * stride, nhhl0, 2);
    wasm_v128_store16_lane(pix + 1 + 13 * stride, nhhl0, 6);

    wasm_v128_store32_lane(pix - 3 + 14 * stride, nhhh0, 0);
    wasm_v128_store16_lane(pix + 1 + 14 * stride, nhhh0, 2);
    wasm_v128_store32_lane(pix - 3 + 15 * stride, nhhh0, 2);
    wasm_v128_store16_lane(pix + 1 + 15 * stride, nhhh0, 6);

}
void ff_h264_h_loop_filter_luma_mbaff_intra_8_websimd128(uint8_t *pix /*align 16*/, ptrdiff_t stride, int alpha, int beta) {
    uint8_t* start = pix - 4;
    __u16x8 row0 = (__u16x8)wasm_u16x8_load8x8(start);
    __u16x8 row1 = (__u16x8)wasm_u16x8_load8x8(start + stride);
    __u16x8 row2 = (__u16x8)wasm_u16x8_load8x8(start + 2 * stride);
    __u16x8 row3 = (__u16x8)wasm_u16x8_load8x8(start + 3 * stride);
    __u16x8 row4 = (__u16x8)wasm_u16x8_load8x8(start + 4 * stride);
    __u16x8 row5 = (__u16x8)wasm_u16x8_load8x8(start + 5 * stride);
    __u16x8 row6 = (__u16x8)wasm_u16x8_load8x8(start + 6 * stride);
    __u16x8 row7 = (__u16x8)wasm_u16x8_load8x8(start + 7 * stride);

    INVERSE_VECTOR_8x8;

    __u16x8 alpha_vector = (__u16x8)wasm_u16x8_splat(alpha);
    __u16x8 alpha_2_vector = alpha_vector >> 2 + 2;
    __u16x8 beta_vector = (__u16x8)wasm_u16x8_splat(beta);

    __u16x8 p0_asub_q0 = (__u16x8)wasm_u16x8_sub_sat(
        wasm_u16x8_max((v128_t)p0, (v128_t)q0),
        wasm_u16x8_min((v128_t)p0, (v128_t)q0)
    );
    __u16x8 p1_asub_p0 = (__u16x8)wasm_u16x8_sub_sat(
        wasm_u16x8_max((v128_t)p0, (v128_t)p1),
        wasm_u16x8_min((v128_t)p0, (v128_t)p1)
    );
    __u16x8 q1_asub_q0 = (__u16x8)wasm_u16x8_sub_sat(
        wasm_u16x8_max((v128_t)q1, (v128_t)q0),
        wasm_u16x8_min((v128_t)q1, (v128_t)q0)
    );
    __u16x8 p2_asub_p0 = (__u16x8)wasm_u16x8_sub_sat(
        wasm_u16x8_max((v128_t)p0, (v128_t)p2),
        wasm_u16x8_min((v128_t)p0, (v128_t)p2)
    );
    __u16x8 q2_asub_q0 = (__u16x8)wasm_u16x8_sub_sat(
        wasm_u16x8_max((v128_t)q2, (v128_t)q0),
        wasm_u16x8_min((v128_t)q2, (v128_t)q0)
    );
    /* 计算滤波条件 */
    __u16x8 p0_asub_q0_less_then_alpha = (__u16x8)wasm_u16x8_lt((v128_t)p0_asub_q0, (v128_t)alpha_vector);
    __u16x8 p0_asub_q0_less_then_alpha_2 = (__u16x8)wasm_u16x8_lt((v128_t)p0_asub_q0, (v128_t)alpha_2_vector);
    __u16x8 p1_asub_p0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)p1_asub_p0, (v128_t)beta_vector);
    __u16x8 p2_asub_p0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)p2_asub_p0, (v128_t)beta_vector);
    __u16x8 q1_asub_q0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)q1_asub_q0, (v128_t)beta_vector);
    __u16x8 q2_asub_q0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)q2_asub_q0, (v128_t)beta_vector);
    __u16x8 filter_bool = p0_asub_q0_less_then_alpha & p1_asub_p0_less_then_beta & q1_asub_q0_less_then_beta;
    __u16x8 filter_bool_2 = filter_bool & p0_asub_q0_less_then_alpha_2;
    __u16x8 filter_bool_p1_p2 = filter_bool_2 & p2_asub_p0_less_then_beta;
    __u16x8 filter_bool_q1_q2 = filter_bool_2 & q2_asub_q0_less_then_beta;

    /* 计算 p2-p0 条件的 p0 */
    __u16x8 p0_filter_p2_p0 = (p2 + 2 * p1 + 2 * p0 + 2 * q0 + q1 + 4) >> 3;

    /* 计算 p2-p0 条件的 p1 */
    __u16x8 p1_filter_p2_p0 = (p2 + p1 + p0 + q0 + 2) >> 2;

    /* 计算 p2-p0 条件的 p2 */
    __u16x8 p2_filter_p2_p0 = (2 * p3 + 3 * p2 + p1 + p0 + q0 + 4) >> 3;

    /* 计算非 p2-p0 条件的 p0 */
    __u16x8 p0_filter_p2_p0_not = (2 * p1 + p0 + q1 + 2) >> 2;

    /* 计算 q2-q0 条件的 q0 */
    __u16x8 q0_filter_q2_q0 = (p1 + 2 * p0 + 2 * q0 + 2 * q1 + q2 + 4) >> 3;

    /* 计算 q2-q0 条件的 q1 */
    __u16x8 q1_filter_q2_q0 = (p0 + q0 + q1 + q2 + 2) >> 2;

    /* 计算 q2-q0 条件的 p2 */
    __u16x8 q2_filter_q2_q0 = (2 * q3 + 3 * q2 + q1 + q0 + p0 + 4) >> 3;

    /* 计算非 q2-q0 条件的 q0 */
    __u16x8 q0_filter_q2_q0_not = (2 * q1 + q0 + p1 + 2) >> 2;

    p0 = wasm_v128_bitselect(
        wasm_v128_bitselect(
            wasm_v128_bitselect(
                (v128_t)p0_filter_p2_p0,
                (v128_t)p0_filter_p2_p0_not,
                (v128_t)filter_bool_p1_p2
            ),
            (v128_t)p0_filter_p2_p0_not,
            (v128_t)filter_bool_2
        ),
        (v128_t)p0,
        (v128_t)filter_bool
    );
    q0 = wasm_v128_bitselect(
        wasm_v128_bitselect(
            wasm_v128_bitselect(
                (v128_t)q0_filter_q2_q0,
                (v128_t)q0_filter_q2_q0_not,
                (v128_t)filter_bool_q1_q2
            ),
            (v128_t)q0_filter_q2_q0_not,
            (v128_t)filter_bool_2
        ),
        (v128_t)q0,
        (v128_t)filter_bool
    );
    p1 = wasm_v128_bitselect((v128_t)p1_filter_p2_p0, (v128_t)p1, (v128_t)filter_bool_p1_p2);
    p2 = wasm_v128_bitselect((v128_t)p2_filter_p2_p0, (v128_t)p2, (v128_t)filter_bool_p1_p2);
    q1 = wasm_v128_bitselect((v128_t)q1_filter_q2_q0, (v128_t)q1, (v128_t)filter_bool_q1_q2);
    q2 = wasm_v128_bitselect((v128_t)q2_filter_q2_q0, (v128_t)q2, (v128_t)filter_bool_q1_q2);

    IN_INVERSE_VECTOR_8x8;

    row0 = (__i64x2)row0 >> 8;
    row1 = (__i64x2)row1 >> 8;
    row2 = (__i64x2)row2 >> 8;
    row3 = (__i64x2)row3 >> 8;
   
    wasm_v128_store32_lane(pix - 3, (v128_t)row0, 0);
    wasm_v128_store16_lane(pix + 1, (v128_t)row0, 2);
    wasm_v128_store32_lane(pix - 3 + stride, (v128_t)row0, 2);
    wasm_v128_store16_lane(pix + 1 + stride, (v128_t)row0, 6);

    wasm_v128_store32_lane(pix - 3 + 2 * stride, (v128_t)row1, 0);
    wasm_v128_store16_lane(pix + 1 + 2 * stride, (v128_t)row1, 2);
    wasm_v128_store32_lane(pix - 3 + 3 * stride, (v128_t)row1, 2);
    wasm_v128_store16_lane(pix + 1 + 3 * stride, (v128_t)row1, 6);

    wasm_v128_store32_lane(pix - 3 + 4 * stride, (v128_t)row2, 0);
    wasm_v128_store16_lane(pix + 1 + 4 * stride, (v128_t)row2, 2);
    wasm_v128_store32_lane(pix - 3 + 5 * stride, (v128_t)row2, 2);
    wasm_v128_store16_lane(pix + 1 + 5 * stride, (v128_t)row2, 6);

    wasm_v128_store32_lane(pix - 3 + 6 * stride, (v128_t)row3, 0);
    wasm_v128_store16_lane(pix + 1 + 6 * stride, (v128_t)row3, 2);
    wasm_v128_store32_lane(pix - 3 + 7 * stride, (v128_t)row3, 2);
    wasm_v128_store16_lane(pix + 1 + 7 * stride, (v128_t)row3, 6);
}

#define LOOP_FILTER_CHROMA_8 \
    v128_t zero = {0}; \
    int8_t _tc0 = tc0[0]; \
    int8_t _tc1 = tc0[1]; \
    int8_t _tc2 = tc0[2]; \
    int8_t _tc3 = tc0[3]; \
    __i16x8 tc_vector = (__i16x8)wasm_i16x8_make( \
        _tc0, _tc0, _tc1, _tc1, \
        _tc2, _tc2, _tc3, _tc3 \
    ); \
    __i16x8 tc_vector_neg = (__i16x8)wasm_i16x8_neg((v128_t)tc_vector); \
    __u16x8 alpha_vector = (__u16x8)wasm_u16x8_splat(alpha); \
    __u16x8 beta_vector = (__u16x8)wasm_u16x8_splat(beta); \
    __u16x8 p0_asub_q0 = (__u16x8)wasm_u16x8_sub_sat( \
        wasm_u16x8_max((v128_t)p0, (v128_t)q0), \
        wasm_u16x8_min((v128_t)p0, (v128_t)q0) \
    ); \
    __u16x8 p1_asub_p0 = (__u16x8)wasm_u16x8_sub_sat( \
        wasm_u16x8_max((v128_t)p0, (v128_t)p1), \
        wasm_u16x8_min((v128_t)p0, (v128_t)p1) \
    ); \
    __u16x8 q1_asub_q0 = (__u16x8)wasm_u16x8_sub_sat( \
        wasm_u16x8_max((v128_t)q1, (v128_t)q0), \
        wasm_u16x8_min((v128_t)q1, (v128_t)q0) \
    ); \
    /* 计算滤波条件 */ \
    __u16x8 p0_asub_q0_less_then_alpha = (__u16x8)wasm_u16x8_lt((v128_t)p0_asub_q0, (v128_t)alpha_vector); \
    __u16x8 p1_asub_p0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)p1_asub_p0, (v128_t)beta_vector); \
    __u16x8 q1_asub_q0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)q1_asub_q0, (v128_t)beta_vector); \
    __u16x8 tc_orig_less_then_zero = (__u16x8)wasm_i16x8_gt((v128_t)tc_vector, (v128_t)zero); \
    /* p0,q0 是否滤波条件 */ \
    __u16x8 p0_q0_filter_bool = p0_asub_q0_less_then_alpha & p1_asub_p0_less_then_beta & q1_asub_q0_less_then_beta & tc_orig_less_then_zero; \
    __i16x8 avgr; \
    avgr = (((__i16x8)q0 - (__i16x8)p0) * 4 + ((__i16x8)p1 - (__i16x8)q1) + 4) >> 3; \
    avgr = wasm_i16x8_max((v128_t)avgr, (v128_t)tc_vector_neg); \
    avgr = wasm_i16x8_min((v128_t)avgr, (v128_t)tc_vector); \
    __i16x8 p0_filter = (__i16x8)p0 + avgr; \
    __i16x8 q0_filter = (__i16x8)q0 - avgr; \
    p0 = wasm_v128_bitselect((v128_t)p0_filter, (v128_t)p0, (v128_t)p0_q0_filter_bool); \
    q0 = wasm_v128_bitselect((v128_t)q0_filter, (v128_t)q0, (v128_t)p0_q0_filter_bool); \

void ff_h264_v_loop_filter_chroma_8_websimd128(uint8_t *pix /*align 8*/, ptrdiff_t stride, int alpha, int beta, int8_t *tc0) {
    __u16x8 p0 = (__u16x8)wasm_u16x8_load8x8(pix - stride);
    __u16x8 p1 = (__u16x8)wasm_u16x8_load8x8(pix - 2 * stride);
    __u16x8 q0 = (__u16x8)wasm_u16x8_load8x8(pix);
    __u16x8 q1 = (__u16x8)wasm_u16x8_load8x8(pix + stride);

    LOOP_FILTER_CHROMA_8;

    p0 = wasm_u8x16_narrow_i16x8((v128_t)p0, (v128_t)q0);
    wasm_v128_store64_lane(pix - stride, (v128_t)p0, 0);
    wasm_v128_store64_lane(pix, (v128_t)p0, 1);
}
void ff_h264_h_loop_filter_chroma_8_websimd128(uint8_t *pix /*align 4*/, ptrdiff_t stride, int alpha, int beta, int8_t *tc0) {
    uint8_t* start = pix - 4;
    __u16x8 row0 = (__u16x8)wasm_u16x8_load8x8(start);
    __u16x8 row1 = (__u16x8)wasm_u16x8_load8x8(start + stride);
    __u16x8 row2 = (__u16x8)wasm_u16x8_load8x8(start + 2 * stride);
    __u16x8 row3 = (__u16x8)wasm_u16x8_load8x8(start + 3 * stride);
    __u16x8 row4 = (__u16x8)wasm_u16x8_load8x8(start + 4 * stride);
    __u16x8 row5 = (__u16x8)wasm_u16x8_load8x8(start + 5 * stride);
    __u16x8 row6 = (__u16x8)wasm_u16x8_load8x8(start + 6 * stride);
    __u16x8 row7 = (__u16x8)wasm_u16x8_load8x8(start + 7 * stride);

    INVERSE_VECTOR_8x8;

    LOOP_FILTER_CHROMA_8;

    IN_INVERSE_VECTOR_8x8;

    wasm_v128_store8_lane(pix - 1, (v128_t)row0, 3);
    wasm_v128_store8_lane(pix, (v128_t)row0, 4);
    wasm_v128_store8_lane(pix - 1 + stride, (v128_t)row0, 11);
    wasm_v128_store8_lane(pix + stride, (v128_t)row0, 12);

    wasm_v128_store8_lane(pix - 1 + 2 * stride, (v128_t)row1, 3);
    wasm_v128_store8_lane(pix + 2 * stride, (v128_t)row1, 4);
    wasm_v128_store8_lane(pix - 1 + 3 * stride, (v128_t)row1, 11);
    wasm_v128_store8_lane(pix + 3 * stride, (v128_t)row1, 12);

    wasm_v128_store8_lane(pix - 1 + 4 * stride, (v128_t)row2, 3);
    wasm_v128_store8_lane(pix + 4 * stride, (v128_t)row2, 4);
    wasm_v128_store8_lane(pix - 1 + 5 * stride, (v128_t)row2, 11);
    wasm_v128_store8_lane(pix + 5 * stride, (v128_t)row2, 12);

    wasm_v128_store8_lane(pix - 1 + 6 * stride, (v128_t)row3, 3);
    wasm_v128_store8_lane(pix + 6 * stride, (v128_t)row3, 4);
    wasm_v128_store8_lane(pix - 1 + 7 * stride, (v128_t)row3, 11);
    wasm_v128_store8_lane(pix + 7 * stride, (v128_t)row3, 12);
}

#define INVERSE_VECTOR_4x4 \
    __u32x4 nl0 = (__u32x4)wasm_i32x4_shuffle(row0, row1, 0, 4, 1, 5); \
    __u32x4 nl1 = (__u32x4)wasm_i32x4_shuffle(row2, row3, 0, 4, 1, 5); \
    __u32x4 nh0 = (__u32x4)wasm_i32x4_shuffle(row0, row1, 2, 6, 3, 7); \
    __u32x4 nh1 = (__u32x4)wasm_i32x4_shuffle(row2, row3, 2, 6, 3, 7); \
    __u32x4 p1 = (__u32x4)wasm_i32x4_shuffle(nl0, nh0, 0, 4, 1, 5); \
    __u32x4 p0 = (__u32x4)wasm_i32x4_shuffle(nl0, nh0, 2, 6, 3, 7); \
    __u32x4 q0 = (__u32x4)wasm_i32x4_shuffle(nl1, nh1, 0, 4, 1, 5); \
    __u32x4 q1 = (__u32x4)wasm_i32x4_shuffle(nl1, nh1, 2, 6, 3, 7); \

#define IN_INVERSE_VECTOR_4x4 \
    nl0 = (__u32x4)wasm_i32x4_shuffle(p1, p0, 0, 4, 1, 5); \
    nl1 = (__u32x4)wasm_i32x4_shuffle(q0, q1, 0, 4, 1, 5); \
    nh0 = (__u32x4)wasm_i32x4_shuffle(p1, p0, 2, 6, 3, 7); \
    nh1 = (__u32x4)wasm_i32x4_shuffle(q0, q1, 2, 6, 3, 7); \
    row0 = (__u32x4)wasm_i32x4_shuffle(nl0, nh0, 0, 4, 1, 5); \
    row1 = (__u32x4)wasm_i32x4_shuffle(nl0, nh0, 2, 6, 3, 7); \
    row2 = (__u32x4)wasm_i32x4_shuffle(nl1, nh1, 0, 4, 1, 5); \
    row3 = (__u32x4)wasm_i32x4_shuffle(nl1, nh1, 2, 6, 3, 7); \
    row0 = wasm_u16x8_narrow_i32x4(row0, row1); \
    row1 = wasm_u16x8_narrow_i32x4(row2, row3); \
    row0 = wasm_u8x16_narrow_i16x8(row0, row1);

void ff_h264_h_loop_filter_chroma_mbaff_8_websimd128(uint8_t *pix /*align 8*/, ptrdiff_t stride, int alpha, int beta, int8_t *tc0) {
    uint8_t* start = pix - 2;
    v128_t row0 = wasm_v128_load32_zero(start);
    v128_t row1 = wasm_v128_load32_zero(start + stride);
    v128_t row2 = wasm_v128_load32_zero(start + 2 * stride);
    v128_t row3 = wasm_v128_load32_zero(start + 3 * stride);
    row0 = wasm_u16x8_extend_low_u8x16(row0);
    row1 = wasm_u16x8_extend_low_u8x16(row1);
    row2 = wasm_u16x8_extend_low_u8x16(row2);
    row3 = wasm_u16x8_extend_low_u8x16(row3);
    row0 = wasm_u32x4_extend_low_u16x8(row0);
    row1 = wasm_u32x4_extend_low_u16x8(row1);
    row2 = wasm_u32x4_extend_low_u16x8(row2);
    row3 = wasm_u32x4_extend_low_u16x8(row3);

    INVERSE_VECTOR_4x4;

    v128_t zero = {0};
    int8_t _tc0 = tc0[0];
    int8_t _tc1 = tc0[1];
    int8_t _tc2 = tc0[2];
    int8_t _tc3 = tc0[3];
    __i32x4 tc_vector = (__i32x4)wasm_i32x4_make(
        _tc0, _tc1, _tc2, _tc3
    );
    __i32x4 tc_vector_neg = (__i32x4)wasm_i32x4_neg((v128_t)tc_vector);
    __u32x4 alpha_vector = (__u32x4)wasm_u32x4_splat(alpha);
    __u32x4 beta_vector = (__u32x4)wasm_u32x4_splat(beta);
    __u32x4 p0_asub_q0 = (__u32x4)wasm_i32x4_abs(
        wasm_i32x4_sub(p0, q0)
    );
    __u32x4 p1_asub_p0 = (__u32x4)wasm_i32x4_abs(
        wasm_i32x4_sub(p1, p0)
    );
    __u32x4 q1_asub_q0 = (__u32x4)wasm_i32x4_abs(
        wasm_i32x4_sub(q1, q0)
    );
    /* 计算滤波条件 */
    __u32x4 p0_asub_q0_less_then_alpha = (__u32x4)wasm_u32x4_lt((v128_t)p0_asub_q0, (v128_t)alpha_vector);
    __u32x4 p1_asub_p0_less_then_beta = (__u32x4)wasm_u32x4_lt((v128_t)p1_asub_p0, (v128_t)beta_vector);
    __u32x4 q1_asub_q0_less_then_beta = (__u32x4)wasm_u32x4_lt((v128_t)q1_asub_q0, (v128_t)beta_vector);
    __u32x4 tc_orig_less_then_zero = (__u32x4)wasm_i32x4_gt((v128_t)tc_vector, (v128_t)zero);
    /* p0,q0 是否滤波条件 */
    __u32x4 p0_q0_filter_bool = p0_asub_q0_less_then_alpha & p1_asub_p0_less_then_beta & q1_asub_q0_less_then_beta & tc_orig_less_then_zero;
    __i32x4 avgr;
    avgr = (((__i32x4)q0 - (__i32x4)p0) * 4 + ((__i32x4)p1 - (__i32x4)q1) + 4) >> 3;
    avgr = wasm_i32x4_max((v128_t)avgr, (v128_t)tc_vector_neg);
    avgr = wasm_i32x4_min((v128_t)avgr, (v128_t)tc_vector);
    __i32x4 p0_filter = (__i32x4)p0 + avgr;
    __i32x4 q0_filter = (__i32x4)q0 - avgr;
    p0 = wasm_v128_bitselect((v128_t)p0_filter, (v128_t)p0, (v128_t)p0_q0_filter_bool);
    q0 = wasm_v128_bitselect((v128_t)q0_filter, (v128_t)q0, (v128_t)p0_q0_filter_bool);

    IN_INVERSE_VECTOR_4x4;

    row0 = (__i32x4)row0 >> 8;

    wasm_v128_store16_lane(pix - 1, (v128_t)row0, 0);
    wasm_v128_store16_lane(pix - 1 + stride, (v128_t)row0, 2);
    wasm_v128_store16_lane(pix - 1 + 2 * stride, (v128_t)row0, 4);
    wasm_v128_store16_lane(pix - 1 + 3 * stride, (v128_t)row0, 6);
}

void ff_h264_h_loop_filter_chroma422_8_websimd128(uint8_t *pix /*align 4*/, ptrdiff_t stride, int alpha, int beta, int8_t *tc0) {
    uint8_t* start = pix - 2;
    __u8x16 row0 = (__u8x16)wasm_v128_load32_zero(start);
    __u8x16 row1 = (__u8x16)wasm_v128_load32_zero(start + stride);
    __u8x16 row2 = (__u8x16)wasm_v128_load32_zero(start + 2 * stride);
    __u8x16 row3 = (__u8x16)wasm_v128_load32_zero(start + 3 * stride);
    __u8x16 row4 = (__u8x16)wasm_v128_load32_zero(start + 4 * stride);
    __u8x16 row5 = (__u8x16)wasm_v128_load32_zero(start + 5 * stride);
    __u8x16 row6 = (__u8x16)wasm_v128_load32_zero(start + 6 * stride);
    __u8x16 row7 = (__u8x16)wasm_v128_load32_zero(start + 7 * stride);
    __u8x16 row8 = (__u8x16)wasm_v128_load32_zero(start + 8 * stride);
    __u8x16 row9 = (__u8x16)wasm_v128_load32_zero(start + 9 * stride);
    __u8x16 row10 = (__u8x16)wasm_v128_load32_zero(start + 10 * stride);
    __u8x16 row11 = (__u8x16)wasm_v128_load32_zero(start + 11 * stride);
    __u8x16 row12 = (__u8x16)wasm_v128_load32_zero(start + 12 * stride);
    __u8x16 row13 = (__u8x16)wasm_v128_load32_zero(start + 13 * stride);
    __u8x16 row14 = (__u8x16)wasm_v128_load32_zero(start + 14 * stride);
    __u8x16 row15 = (__u8x16)wasm_v128_load32_zero(start + 15 * stride);

    __u8x16 p1 = (__u8x16)wasm_u8x16_make(
        row0[0], row1[0], row2[0], row3[0],
        row4[0], row5[0], row6[0], row7[0],
        row8[0], row9[0], row10[0], row11[0],
        row12[0], row13[0], row14[0], row15[0] 
    );
    __u8x16 p0 = (__u8x16)wasm_u8x16_make(
        row0[1], row1[1], row2[1], row3[1],
        row4[1], row5[1], row6[1], row7[1],
        row8[1], row9[1], row10[1], row11[1],
        row12[1], row13[1], row14[1], row15[1] 
    );
    __u8x16 q0 = (__u8x16)wasm_u8x16_make(
        row0[2], row1[2], row2[2], row3[2],
        row4[2], row5[2], row6[2], row7[2],
        row8[2], row9[2], row10[2], row11[2],
        row12[2], row13[2], row14[2], row15[2] 
    );
    __u8x16 q1 = (__u8x16)wasm_u8x16_make(
        row0[3], row1[3], row2[3], row3[3],
        row4[3], row5[3], row6[3], row7[3],
        row8[3], row9[3], row10[3], row11[3],
        row12[3], row13[3], row14[3], row15[3] 
    );

    v128_t zero = {0};
    int8_t _tc0 = tc0[0];
    int8_t _tc1 = tc0[1];
    int8_t _tc2 = tc0[2];
    int8_t _tc3 = tc0[3];
    __i8x16 tc_vector = (__i8x16)wasm_i8x16_make(
        _tc0, _tc0, _tc0, _tc0,
        _tc1, _tc1, _tc1, _tc1,
        _tc2, _tc2, _tc2, _tc2,
        _tc3, _tc3, _tc3, _tc3
    );
    __i8x16 tc_vector_neg = (__i8x16)wasm_i8x16_neg((v128_t)tc_vector);
    __u8x16 alpha_vector = (__u8x16)wasm_u8x16_splat(alpha);
    __u8x16 beta_vector = (__u8x16)wasm_u8x16_splat(beta);
    __u8x16 p0_asub_q0 = (__u8x16)wasm_u8x16_sub_sat(
        wasm_u8x16_max((v128_t)p0, (v128_t)q0),
        wasm_u8x16_min((v128_t)p0, (v128_t)q0)
    );
    __u8x16 p1_asub_p0 = (__u8x16)wasm_u8x16_sub_sat(
        wasm_u8x16_max((v128_t)p0, (v128_t)p1),
        wasm_u8x16_min((v128_t)p0, (v128_t)p1)
    );
    __u8x16 q1_asub_q0 = (__u8x16)wasm_u8x16_sub_sat(
        wasm_u8x16_max((v128_t)q1, (v128_t)q0),
        wasm_u8x16_min((v128_t)q1, (v128_t)q0)
    );
    /* 计算滤波条件 */
    __u8x16 p0_asub_q0_less_then_alpha = (__u8x16)wasm_u8x16_lt((v128_t)p0_asub_q0, (v128_t)alpha_vector);
    __u8x16 p1_asub_p0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)p1_asub_p0, (v128_t)beta_vector);
    __u8x16 q1_asub_q0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)q1_asub_q0, (v128_t)beta_vector);
    __u8x16 tc_orig_less_then_zero = (__u8x16)wasm_i8x16_gt((v128_t)tc_vector, (v128_t)zero);
    /* p0,q0 是否滤波条件 */
    __u8x16 p0_q0_filter_bool = p0_asub_q0_less_then_alpha & p1_asub_p0_less_then_beta & q1_asub_q0_less_then_beta & tc_orig_less_then_zero;

    __i16x8 p0_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)p0);
    __i16x8 p0_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)p0);
    __i16x8 q0_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)q0);
    __i16x8 q0_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)q0);
    __i16x8 p1_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)p1);
    __i16x8 p1_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)p1);
    __i16x8 q1_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)q1);
    __i16x8 q1_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)q1);
    __i16x8 avgr_low;
    __i16x8 avgr_high;

    avgr_low = ((q0_low - p0_low) * 4 + (p1_low - q1_low) + 4) >> 3;
    avgr_low = wasm_i8x16_max((v128_t)avgr_low, (v128_t)wasm_i16x8_extend_low_i8x16((v128_t)tc_vector_neg));
    avgr_low = wasm_i8x16_min((v128_t)avgr_low, (v128_t)wasm_i16x8_extend_low_i8x16((v128_t)tc_vector));

    avgr_high = ((q0_high - p0_high) * 4 + (p1_high - q1_high) + 4) >> 3;
    avgr_high = wasm_i8x16_max((v128_t)avgr_high, (v128_t)wasm_i16x8_extend_high_i8x16((v128_t)tc_vector_neg));
    avgr_high = wasm_i8x16_min((v128_t)avgr_high, (v128_t)wasm_i16x8_extend_high_i8x16((v128_t)tc_vector));

    __u8x16 p0_filter = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)(p0_low + avgr_low), (v128_t)(p0_high + avgr_high));
    __u8x16 q0_filter = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)(q0_low - avgr_low), (v128_t)(q0_high - avgr_high));
    p0 = wasm_v128_bitselect((v128_t)p0_filter, (v128_t)p0, (v128_t)p0_q0_filter_bool);
    q0 = wasm_v128_bitselect((v128_t)q0_filter, (v128_t)q0, (v128_t)p0_q0_filter_bool);

    row0 = wasm_u8x16_make(
        p1[0], p0[0], q0[0], q1[0],
        p1[1], p0[1], q0[1], q1[1],
        p1[2], p0[2], q0[2], q1[2],
        p1[3], p0[3], q0[3], q1[3]
    );
    row1 = wasm_u8x16_make(
        p1[4], p0[4], q0[4], q1[4],
        p1[5], p0[5], q0[5], q1[5],
        p1[6], p0[6], q0[6], q1[6],
        p1[7], p0[7], q0[7], q1[7]
    );
    row2 = wasm_u8x16_make(
        p1[8], p0[8], q0[8], q1[8],
        p1[9], p0[9], q0[9], q1[9],
        p1[10], p0[10], q0[10], q1[10],
        p1[11], p0[11], q0[11], q1[11]
    );
    row2 = wasm_u8x16_make(
        p1[12], p0[12], q0[12], q1[12],
        p1[13], p0[13], q0[13], q1[13],
        p1[14], p0[14], q0[14], q1[14],
        p1[15], p0[15], q0[15], q1[15]
    );

    row0 = (__i32x4)row0 >> 8;
    row1 = (__i32x4)row1 >> 8;
    row2 = (__i32x4)row2 >> 8;
    row3 = (__i32x4)row3 >> 8;

    wasm_v128_store16_lane(pix - 1, (v128_t)row0, 0);
    wasm_v128_store16_lane(pix - 1 + stride, (v128_t)row0, 2);
    wasm_v128_store16_lane(pix - 1 + 2 * stride, (v128_t)row0, 4);
    wasm_v128_store16_lane(pix - 1 + 3 * stride, (v128_t)row0, 6);
    wasm_v128_store16_lane(pix - 1 + 4 * stride, (v128_t)row1, 0);
    wasm_v128_store16_lane(pix - 1 + 5 * stride, (v128_t)row1, 2);
    wasm_v128_store16_lane(pix - 1 + 6 * stride, (v128_t)row1, 4);
    wasm_v128_store16_lane(pix - 1 + 7 * stride, (v128_t)row1, 6);
    wasm_v128_store16_lane(pix - 1 + 8 * stride, (v128_t)row2, 0);
    wasm_v128_store16_lane(pix - 1 + 9 * stride, (v128_t)row2, 2);
    wasm_v128_store16_lane(pix - 1 + 10 * stride, (v128_t)row2, 4);
    wasm_v128_store16_lane(pix - 1 + 11 * stride, (v128_t)row2, 6);
    wasm_v128_store16_lane(pix - 1 + 12 * stride, (v128_t)row3, 0);
    wasm_v128_store16_lane(pix - 1 + 13 * stride, (v128_t)row3, 2);
    wasm_v128_store16_lane(pix - 1 + 14 * stride, (v128_t)row3, 4);
    wasm_v128_store16_lane(pix - 1 + 15 * stride, (v128_t)row3, 6);
}

void ff_h264_h_loop_filter_chroma422_mbaff_8_websimd128(uint8_t *pix /*align 8*/, ptrdiff_t stride, int alpha, int beta, int8_t *tc0) {
    uint8_t* start = pix - 4;
    __u16x8 row0 = (__u16x8)wasm_u16x8_load8x8(start);
    __u16x8 row1 = (__u16x8)wasm_u16x8_load8x8(start + stride);
    __u16x8 row2 = (__u16x8)wasm_u16x8_load8x8(start + 2 * stride);
    __u16x8 row3 = (__u16x8)wasm_u16x8_load8x8(start + 3 * stride);
    __u16x8 row4 = (__u16x8)wasm_u16x8_load8x8(start + 4 * stride);
    __u16x8 row5 = (__u16x8)wasm_u16x8_load8x8(start + 5 * stride);
    __u16x8 row6 = (__u16x8)wasm_u16x8_load8x8(start + 6 * stride);
    __u16x8 row7 = (__u16x8)wasm_u16x8_load8x8(start + 7 * stride);

    INVERSE_VECTOR_8x8;

    LOOP_FILTER_CHROMA_8;

    IN_INVERSE_VECTOR_8x8;

    wasm_v128_store8_lane(pix - 1, (v128_t)row0, 3);
    wasm_v128_store8_lane(pix, (v128_t)row0, 4);
    wasm_v128_store8_lane(pix - 1 + stride, (v128_t)row0, 11);
    wasm_v128_store8_lane(pix + stride, (v128_t)row0, 12);

    wasm_v128_store8_lane(pix - 1 + 2 * stride, (v128_t)row1, 3);
    wasm_v128_store8_lane(pix + 2 * stride, (v128_t)row1, 4);
    wasm_v128_store8_lane(pix - 1 + 3 * stride, (v128_t)row1, 11);
    wasm_v128_store8_lane(pix + 3 * stride, (v128_t)row1, 12);

    wasm_v128_store8_lane(pix - 1 + 4 * stride, (v128_t)row2, 3);
    wasm_v128_store8_lane(pix + 4 * stride, (v128_t)row2, 4);
    wasm_v128_store8_lane(pix - 1 + 5 * stride, (v128_t)row2, 11);
    wasm_v128_store8_lane(pix + 5 * stride, (v128_t)row2, 12);

    wasm_v128_store8_lane(pix - 1 + 6 * stride, (v128_t)row3, 3);
    wasm_v128_store8_lane(pix + 6 * stride, (v128_t)row3, 4);
    wasm_v128_store8_lane(pix - 1 + 7 * stride, (v128_t)row3, 11);
    wasm_v128_store8_lane(pix + 7 * stride, (v128_t)row3, 12);
}

#define LOOP_FILTER_CHROMA_INTRA_8 \
    __u16x8 alpha_vector = (__u16x8)wasm_u16x8_splat(alpha); \
    __u16x8 beta_vector = (__u16x8)wasm_u16x8_splat(beta); \
    __u16x8 p0_asub_q0 = (__u16x8)wasm_u16x8_sub_sat( \
        wasm_u16x8_max((v128_t)p0, (v128_t)q0), \
        wasm_u16x8_min((v128_t)p0, (v128_t)q0) \
    ); \
    __u16x8 p1_asub_p0 = (__u16x8)wasm_u16x8_sub_sat( \
        wasm_u16x8_max((v128_t)p0, (v128_t)p1), \
        wasm_u16x8_min((v128_t)p0, (v128_t)p1) \
    ); \
    __u16x8 q1_asub_q0 = (__u16x8)wasm_u16x8_sub_sat( \
        wasm_u16x8_max((v128_t)q1, (v128_t)q0), \
        wasm_u16x8_min((v128_t)q1, (v128_t)q0) \
    ); \
    /* 计算滤波条件 */ \
    __u16x8 p0_asub_q0_less_then_alpha = (__u16x8)wasm_u16x8_lt((v128_t)p0_asub_q0, (v128_t)alpha_vector); \
    __u16x8 p1_asub_p0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)p1_asub_p0, (v128_t)beta_vector); \
    __u16x8 q1_asub_q0_less_then_beta = (__u16x8)wasm_u16x8_lt((v128_t)q1_asub_q0, (v128_t)beta_vector); \
    /* p0,q0 是否滤波条件 */ \
    __u16x8 p0_q0_filter_bool = p0_asub_q0_less_then_alpha & p1_asub_p0_less_then_beta & q1_asub_q0_less_then_beta; \
    __i16x8 p0_filter = (2 * (__i16x8)p1 + (__i16x8)p0 + (__i16x8)q1 + 2) >> 2; \
    __i16x8 q0_filter = (2 * (__i16x8)q1 + (__i16x8)q0 + (__i16x8)p1 + 2) >> 2; \
    p0 = wasm_v128_bitselect((v128_t)p0_filter, (v128_t)p0, (v128_t)p0_q0_filter_bool); \
    q0 = wasm_v128_bitselect((v128_t)q0_filter, (v128_t)q0, (v128_t)p0_q0_filter_bool); \

void ff_h264_v_loop_filter_chroma_intra_8_websimd128(uint8_t *pix /*align 8*/, ptrdiff_t stride, int alpha, int beta) {
    __u16x8 p0 = (__u16x8)wasm_u16x8_load8x8(pix - stride);
    __u16x8 p1 = (__u16x8)wasm_u16x8_load8x8(pix - 2 * stride);
    __u16x8 q0 = (__u16x8)wasm_u16x8_load8x8(pix);
    __u16x8 q1 = (__u16x8)wasm_u16x8_load8x8(pix + stride);

    LOOP_FILTER_CHROMA_INTRA_8;

    p0 = wasm_u8x16_narrow_i16x8((v128_t)p0, (v128_t)q0);
    wasm_v128_store64_lane(pix - stride, (v128_t)p0, 0);
    wasm_v128_store64_lane(pix, (v128_t)p0, 1);
}
void ff_h264_h_loop_filter_chroma_intra_8_websimd128(uint8_t *pix /*align 8*/, ptrdiff_t stride, int alpha, int beta) {
    uint8_t* start = pix - 4;
    __u16x8 row0 = (__u16x8)wasm_u16x8_load8x8(start);
    __u16x8 row1 = (__u16x8)wasm_u16x8_load8x8(start + stride);
    __u16x8 row2 = (__u16x8)wasm_u16x8_load8x8(start + 2 * stride);
    __u16x8 row3 = (__u16x8)wasm_u16x8_load8x8(start + 3 * stride);
    __u16x8 row4 = (__u16x8)wasm_u16x8_load8x8(start + 4 * stride);
    __u16x8 row5 = (__u16x8)wasm_u16x8_load8x8(start + 5 * stride);
    __u16x8 row6 = (__u16x8)wasm_u16x8_load8x8(start + 6 * stride);
    __u16x8 row7 = (__u16x8)wasm_u16x8_load8x8(start + 7 * stride);

    INVERSE_VECTOR_8x8;

    LOOP_FILTER_CHROMA_INTRA_8;

    IN_INVERSE_VECTOR_8x8;

    wasm_v128_store8_lane(pix - 1, (v128_t)row0, 3);
    wasm_v128_store8_lane(pix, (v128_t)row0, 4);
    wasm_v128_store8_lane(pix - 1 + stride, (v128_t)row0, 11);
    wasm_v128_store8_lane(pix + stride, (v128_t)row0, 12);

    wasm_v128_store8_lane(pix - 1 + 2 * stride, (v128_t)row1, 3);
    wasm_v128_store8_lane(pix + 2 * stride, (v128_t)row1, 4);
    wasm_v128_store8_lane(pix - 1 + 3 * stride, (v128_t)row1, 11);
    wasm_v128_store8_lane(pix + 3 * stride, (v128_t)row1, 12);

    wasm_v128_store8_lane(pix - 1 + 4 * stride, (v128_t)row2, 3);
    wasm_v128_store8_lane(pix + 4 * stride, (v128_t)row2, 4);
    wasm_v128_store8_lane(pix - 1 + 5 * stride, (v128_t)row2, 11);
    wasm_v128_store8_lane(pix + 5 * stride, (v128_t)row2, 12);

    wasm_v128_store8_lane(pix - 1 + 6 * stride, (v128_t)row3, 3);
    wasm_v128_store8_lane(pix + 6 * stride, (v128_t)row3, 4);
    wasm_v128_store8_lane(pix - 1 + 7 * stride, (v128_t)row3, 11);
    wasm_v128_store8_lane(pix + 7 * stride, (v128_t)row3, 12);
}
void ff_h264_h_loop_filter_chroma_mbaff_intra_8_websimd128(uint8_t *pix /*align 8*/, ptrdiff_t stride, int alpha, int beta) {
    uint8_t* start = pix - 2;
    v128_t row0 = wasm_v128_load32_zero(start);
    v128_t row1 = wasm_v128_load32_zero(start + stride);
    v128_t row2 = wasm_v128_load32_zero(start + 2 * stride);
    v128_t row3 = wasm_v128_load32_zero(start + 3 * stride);
    row0 = wasm_u16x8_extend_low_u8x16(row0);
    row1 = wasm_u16x8_extend_low_u8x16(row1);
    row2 = wasm_u16x8_extend_low_u8x16(row2);
    row3 = wasm_u16x8_extend_low_u8x16(row3);
    row0 = wasm_u32x4_extend_low_u16x8(row0);
    row1 = wasm_u32x4_extend_low_u16x8(row1);
    row2 = wasm_u32x4_extend_low_u16x8(row2);
    row3 = wasm_u32x4_extend_low_u16x8(row3);

    INVERSE_VECTOR_4x4;

    __u32x4 alpha_vector = (__u32x4)wasm_u32x4_splat(alpha);
    __u32x4 beta_vector = (__u32x4)wasm_u32x4_splat(beta);
    
    __u32x4 p0_asub_q0 = (__u32x4)wasm_i32x4_abs(
        wasm_i32x4_sub(p0, q0)
    );
    __u32x4 p1_asub_p0 = (__u32x4)wasm_i32x4_abs(
        wasm_i32x4_sub(p1, p0)
    );
    __u32x4 q1_asub_q0 = (__u32x4)wasm_i32x4_abs(
        wasm_i32x4_sub(q1, q0)
    );
    /* 计算滤波条件 */
    __u32x4 p0_asub_q0_less_then_alpha = (__u32x4)wasm_u32x4_lt((v128_t)p0_asub_q0, (v128_t)alpha_vector);
    __u32x4 p1_asub_p0_less_then_beta = (__u32x4)wasm_u32x4_lt((v128_t)p1_asub_p0, (v128_t)beta_vector);
    __u32x4 q1_asub_q0_less_then_beta = (__u32x4)wasm_u32x4_lt((v128_t)q1_asub_q0, (v128_t)beta_vector);
    /* p0,q0 是否滤波条件 */
    __u32x4 p0_q0_filter_bool = p0_asub_q0_less_then_alpha & p1_asub_p0_less_then_beta & q1_asub_q0_less_then_beta;
    __i32x4 p0_filter = (2 * (__i32x4)p1 + (__i32x4)p0 + (__i32x4)q1 + 2) >> 2;
    __i32x4 q0_filter = (2 * (__i32x4)q1 + (__i32x4)q0 + (__i32x4)p1 + 2) >> 2;
    p0 = wasm_v128_bitselect((v128_t)p0_filter, (v128_t)p0, (v128_t)p0_q0_filter_bool);
    q0 = wasm_v128_bitselect((v128_t)q0_filter, (v128_t)q0, (v128_t)p0_q0_filter_bool);

    IN_INVERSE_VECTOR_4x4;

    row0 = (__i32x4)row0 >> 8;

    wasm_v128_store16_lane(start, (v128_t)row0, 0);
    wasm_v128_store16_lane(start + stride, (v128_t)row0, 2);
    wasm_v128_store16_lane(start + 2 * stride, (v128_t)row0, 4);
    wasm_v128_store16_lane(start + 3 * stride, (v128_t)row0, 6);
}
void ff_h264_h_loop_filter_chroma422_intra_8_websimd128(uint8_t *pix /*align 8*/, ptrdiff_t stride, int alpha, int beta) {
    uint8_t* start = pix - 2;
    __u8x16 row0 = (__u8x16)wasm_v128_load32_zero(start);
    __u8x16 row1 = (__u8x16)wasm_v128_load32_zero(start + stride);
    __u8x16 row2 = (__u8x16)wasm_v128_load32_zero(start + 2 * stride);
    __u8x16 row3 = (__u8x16)wasm_v128_load32_zero(start + 3 * stride);
    __u8x16 row4 = (__u8x16)wasm_v128_load32_zero(start + 4 * stride);
    __u8x16 row5 = (__u8x16)wasm_v128_load32_zero(start + 5 * stride);
    __u8x16 row6 = (__u8x16)wasm_v128_load32_zero(start + 6 * stride);
    __u8x16 row7 = (__u8x16)wasm_v128_load32_zero(start + 7 * stride);
    __u8x16 row8 = (__u8x16)wasm_v128_load32_zero(start + 8 * stride);
    __u8x16 row9 = (__u8x16)wasm_v128_load32_zero(start + 9 * stride);
    __u8x16 row10 = (__u8x16)wasm_v128_load32_zero(start + 10 * stride);
    __u8x16 row11 = (__u8x16)wasm_v128_load32_zero(start + 11 * stride);
    __u8x16 row12 = (__u8x16)wasm_v128_load32_zero(start + 12 * stride);
    __u8x16 row13 = (__u8x16)wasm_v128_load32_zero(start + 13 * stride);
    __u8x16 row14 = (__u8x16)wasm_v128_load32_zero(start + 14 * stride);
    __u8x16 row15 = (__u8x16)wasm_v128_load32_zero(start + 15 * stride);

    __u8x16 p1 = (__u8x16)wasm_u8x16_make(
        row0[0], row1[0], row2[0], row3[0],
        row4[0], row5[0], row6[0], row7[0],
        row8[0], row9[0], row10[0], row11[0],
        row12[0], row13[0], row14[0], row15[0] 
    );
    __u8x16 p0 = (__u8x16)wasm_u8x16_make(
        row0[1], row1[1], row2[1], row3[1],
        row4[1], row5[1], row6[1], row7[1],
        row8[1], row9[1], row10[1], row11[1],
        row12[1], row13[1], row14[1], row15[1] 
    );
    __u8x16 q0 = (__u8x16)wasm_u8x16_make(
        row0[2], row1[2], row2[2], row3[2],
        row4[2], row5[2], row6[2], row7[2],
        row8[2], row9[2], row10[2], row11[2],
        row12[2], row13[2], row14[2], row15[2] 
    );
    __u8x16 q1 = (__u8x16)wasm_u8x16_make(
        row0[3], row1[3], row2[3], row3[3],
        row4[3], row5[3], row6[3], row7[3],
        row8[3], row9[3], row10[3], row11[3],
        row12[3], row13[3], row14[3], row15[3] 
    );

    __u8x16 alpha_vector = (__u8x16)wasm_u8x16_splat(alpha);
    __u8x16 beta_vector = (__u8x16)wasm_u8x16_splat(beta);
    __u8x16 p0_asub_q0 = (__u8x16)wasm_u8x16_sub_sat(
        wasm_u8x16_max((v128_t)p0, (v128_t)q0),
        wasm_u8x16_min((v128_t)p0, (v128_t)q0)
    );
    __u8x16 p1_asub_p0 = (__u8x16)wasm_u8x16_sub_sat(
        wasm_u8x16_max((v128_t)p0, (v128_t)p1),
        wasm_u8x16_min((v128_t)p0, (v128_t)p1)
    );
    __u8x16 q1_asub_q0 = (__u8x16)wasm_u8x16_sub_sat(
        wasm_u8x16_max((v128_t)q1, (v128_t)q0),
        wasm_u8x16_min((v128_t)q1, (v128_t)q0)
    );
    /* 计算滤波条件 */
    __u8x16 p0_asub_q0_less_then_alpha = (__u8x16)wasm_u8x16_lt((v128_t)p0_asub_q0, (v128_t)alpha_vector);
    __u8x16 p1_asub_p0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)p1_asub_p0, (v128_t)beta_vector);
    __u8x16 q1_asub_q0_less_then_beta = (__u8x16)wasm_u8x16_lt((v128_t)q1_asub_q0, (v128_t)beta_vector);
    /* p0,q0 是否滤波条件 */
    __u8x16 p0_q0_filter_bool = p0_asub_q0_less_then_alpha & p1_asub_p0_less_then_beta & q1_asub_q0_less_then_beta;

    __i16x8 p0_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)p0); \
    __i16x8 p0_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)p0); \
    __i16x8 q0_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)q0); \
    __i16x8 q0_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)q0); \
    __i16x8 p1_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)p1); \
    __i16x8 p1_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)p1); \
    __i16x8 q1_low = (__i16x8)wasm_u16x8_extend_low_u8x16((v128_t)q1); \
    __i16x8 q1_high = (__i16x8)wasm_u16x8_extend_high_u8x16((v128_t)q1); \
    __i16x8 px_low; \
    __i16x8 px_high; \

    px_low = (2 * p1_low + p0_low + q1_low + 2) >> 2;
    px_high = (2 * p1_high + p0_high + q1_high + 2) >> 2;
    __u8x16 p0_filter = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high);

    px_low = (2 * q1_low + q0_low + p1_low + 2) >> 2;
    px_high = (2 * q1_high + q0_high + p1_high + 2) >> 2;
    __u8x16 q0_filter = (__u8x16)wasm_u8x16_narrow_i16x8((v128_t)px_low, (v128_t)px_high);

    p0 = wasm_v128_bitselect((v128_t)p0_filter, (v128_t)p0, (v128_t)p0_q0_filter_bool);
    q0 = wasm_v128_bitselect((v128_t)q0_filter, (v128_t)q0, (v128_t)p0_q0_filter_bool);

    row0 = wasm_u8x16_make(
        p1[0], p0[0], q0[0], q1[0],
        p1[1], p0[1], q0[1], q1[1],
        p1[2], p0[2], q0[2], q1[2],
        p1[3], p0[3], q0[3], q1[3]
    );
    row1 = wasm_u8x16_make(
        p1[4], p0[4], q0[4], q1[4],
        p1[5], p0[5], q0[5], q1[5],
        p1[6], p0[6], q0[6], q1[6],
        p1[7], p0[7], q0[7], q1[7]
    );
    row2 = wasm_u8x16_make(
        p1[8], p0[8], q0[8], q1[8],
        p1[9], p0[9], q0[9], q1[9],
        p1[10], p0[10], q0[10], q1[10],
        p1[11], p0[11], q0[11], q1[11]
    );
    row2 = wasm_u8x16_make(
        p1[12], p0[12], q0[12], q1[12],
        p1[13], p0[13], q0[13], q1[13],
        p1[14], p0[14], q0[14], q1[14],
        p1[15], p0[15], q0[15], q1[15]
    );

    row0 = (__i32x4)row0 >> 8;
    row1 = (__i32x4)row1 >> 8;
    row2 = (__i32x4)row2 >> 8;
    row3 = (__i32x4)row3 >> 8;

    wasm_v128_store16_lane(pix - 1, (v128_t)row0, 0);
    wasm_v128_store16_lane(pix - 1 + stride, (v128_t)row0, 2);
    wasm_v128_store16_lane(pix - 1 + 2 * stride, (v128_t)row0, 4);
    wasm_v128_store16_lane(pix - 1 + 3 * stride, (v128_t)row0, 6);
    wasm_v128_store16_lane(pix - 1 + 4 * stride, (v128_t)row1, 0);
    wasm_v128_store16_lane(pix - 1 + 5 * stride, (v128_t)row1, 2);
    wasm_v128_store16_lane(pix - 1 + 6 * stride, (v128_t)row1, 4);
    wasm_v128_store16_lane(pix - 1 + 7 * stride, (v128_t)row1, 6);
    wasm_v128_store16_lane(pix - 1 + 8 * stride, (v128_t)row2, 0);
    wasm_v128_store16_lane(pix - 1 + 9 * stride, (v128_t)row2, 2);
    wasm_v128_store16_lane(pix - 1 + 10 * stride, (v128_t)row2, 4);
    wasm_v128_store16_lane(pix - 1 + 11 * stride, (v128_t)row2, 6);
    wasm_v128_store16_lane(pix - 1 + 12 * stride, (v128_t)row3, 0);
    wasm_v128_store16_lane(pix - 1 + 13 * stride, (v128_t)row3, 2);
    wasm_v128_store16_lane(pix - 1 + 14 * stride, (v128_t)row3, 4);
    wasm_v128_store16_lane(pix - 1 + 15 * stride, (v128_t)row3, 6);
}
void ff_h264_h_loop_filter_chroma422_mbaff_intra_8_websimd128(uint8_t *pix /*align 8*/, ptrdiff_t stride, int alpha, int beta) {
    uint8_t* start = pix - 4;
    __u16x8 row0 = (__u16x8)wasm_u16x8_load8x8(start);
    __u16x8 row1 = (__u16x8)wasm_u16x8_load8x8(start + stride);
    __u16x8 row2 = (__u16x8)wasm_u16x8_load8x8(start + 2 * stride);
    __u16x8 row3 = (__u16x8)wasm_u16x8_load8x8(start + 3 * stride);
    __u16x8 row4 = (__u16x8)wasm_u16x8_load8x8(start + 4 * stride);
    __u16x8 row5 = (__u16x8)wasm_u16x8_load8x8(start + 5 * stride);
    __u16x8 row6 = (__u16x8)wasm_u16x8_load8x8(start + 6 * stride);
    __u16x8 row7 = (__u16x8)wasm_u16x8_load8x8(start + 7 * stride);

    INVERSE_VECTOR_8x8;

    LOOP_FILTER_CHROMA_INTRA_8;

    IN_INVERSE_VECTOR_8x8;

    wasm_v128_store8_lane(pix - 1, (v128_t)row0, 3);
    wasm_v128_store8_lane(pix, (v128_t)row0, 4);
    wasm_v128_store8_lane(pix - 1 + stride, (v128_t)row0, 11);
    wasm_v128_store8_lane(pix + stride, (v128_t)row0, 12);

    wasm_v128_store8_lane(pix - 1 + 2 * stride, (v128_t)row1, 3);
    wasm_v128_store8_lane(pix + 2 * stride, (v128_t)row1, 4);
    wasm_v128_store8_lane(pix - 1 + 3 * stride, (v128_t)row1, 11);
    wasm_v128_store8_lane(pix + 3 * stride, (v128_t)row1, 12);

    wasm_v128_store8_lane(pix - 1 + 4 * stride, (v128_t)row2, 3);
    wasm_v128_store8_lane(pix + 4 * stride, (v128_t)row2, 4);
    wasm_v128_store8_lane(pix - 1 + 5 * stride, (v128_t)row2, 11);
    wasm_v128_store8_lane(pix + 5 * stride, (v128_t)row2, 12);

    wasm_v128_store8_lane(pix - 1 + 6 * stride, (v128_t)row3, 3);
    wasm_v128_store8_lane(pix + 6 * stride, (v128_t)row3, 4);
    wasm_v128_store8_lane(pix - 1 + 7 * stride, (v128_t)row3, 11);
    wasm_v128_store8_lane(pix + 7 * stride, (v128_t)row3, 12);
}

// h264_loop_filter_strength: simd only. the C version is inlined in h264_loopfilter.c
void ff_h264_loop_filter_strength_8_websimd128(int16_t bS[2][4][4], uint8_t nnz[40],
    int8_t ref[2][40], int16_t mv[2][40][2],
    int bidir, int edges, int step,
    int mask_mv0, int mask_mv1, int field) {

}

/* IDCT */
void ff_h264_idct_add_8_websimd128(uint8_t *dst /*align 4*/, int16_t *block /*align 16*/, int stride) {
    block[0] += 1 << 5;

    __i16x8 block_low = (__i16x8)wasm_v128_load(block);
    __i16x8 block_high = (__i16x8)wasm_v128_load(block + 8);

    __i16x8 b0 = (__i16x8)wasm_i16x8_shuffle(block_low, block_high, 0, 1, 2, 3, 0, 0, 0, 0);
    __i16x8 b1 = (__i16x8)wasm_i16x8_shuffle(block_low, block_high, 4, 5, 6, 7, 0, 0, 0, 0);
    __i16x8 b2 = (__i16x8)wasm_i16x8_shuffle(block_low, block_high, 8, 9, 10, 11, 0, 0, 0, 0);
    __i16x8 b3 = (__i16x8)wasm_i16x8_shuffle(block_low, block_high, 12, 13, 14, 15, 0, 0, 0, 0);

    __i16x8 z0 = b0 + b2;
    __i16x8 z1 = b0 - b2;
    __i16x8 z2 = (b1 >> 1) - b3;
    __i16x8 z3 = b1 + (b3 >> 1);

    block_low = (__i16x8)wasm_i16x8_shuffle((z0 + z3), (z1 + z2), 0, 1, 2, 3, 8, 9, 10, 11);
    block_high = (__i16x8)wasm_i16x8_shuffle((z1 - z2), (z0 - z3), 0, 1, 2, 3, 8, 9, 10, 11);

    b0 = (__i16x8)wasm_i16x8_shuffle(block_low, block_high, 0, 4, 8, 12, 0, 0, 0, 0);
    b1 = (__i16x8)wasm_i16x8_shuffle(block_low, block_high, 1, 5, 9, 13, 0, 0, 0, 0);
    b2 = (__i16x8)wasm_i16x8_shuffle(block_low, block_high, 2, 6, 10, 14, 0, 0, 0, 0);
    b3 = (__i16x8)wasm_i16x8_shuffle(block_low, block_high, 3, 7, 11, 15, 0, 0, 0, 0);

    z0 = b0 + b2;
    z1 = b0 - b2;
    z2 = (b1 >> 1) - b3;
    z3 = b1 + (b3 >> 1);

    __i16x8 d0 = (__i16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst));
    __i16x8 d1 = (__i16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst + 1 * stride));
    __i16x8 d2 = (__i16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst + 2 * stride));
    __i16x8 d3 = (__i16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst + 3 * stride));

    __i16x8 sum0 = (__i16x8)wasm_i16x8_shuffle(
        (d0 + ((z0 + z3) >> 6)),
        (d1 + ((z1 + z2) >> 6)),
        0, 1, 2, 3, 8, 9, 10, 11
    );
    __i16x8 sum1 = (__i16x8)wasm_i16x8_shuffle(
        (d2 + ((z1 - z2) >> 6)),
        (d3 + ((z0 - z3) >> 6)),
        0, 1, 2, 3, 8, 9, 10, 11
    );

    v128_t sum = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum1);

    wasm_v128_store32_lane(dst, sum, 0);
    wasm_v128_store32_lane(dst + stride, sum, 1);
    wasm_v128_store32_lane(dst + 2 * stride, sum, 2);
    wasm_v128_store32_lane(dst + 3 * stride, sum, 3);
    
    wasm_v128_store(block, (v128_t){0});
    wasm_v128_store(block + 8, (v128_t){0});
}
void ff_h264_idct8_add_8_websimd128(uint8_t *dst /*align 8*/, int16_t *block /*align 16*/, int stride) {
    block[0] += 32;

    __i16x8 l0 = (__i16x8)wasm_v128_load(block);
    __i16x8 l1 = (__i16x8)wasm_v128_load(block + 8);
    __i16x8 l2 = (__i16x8)wasm_v128_load(block + 16);
    __i16x8 l3 = (__i16x8)wasm_v128_load(block + 24);
    __i16x8 l4 = (__i16x8)wasm_v128_load(block + 32);
    __i16x8 l5 = (__i16x8)wasm_v128_load(block + 40);
    __i16x8 l6 = (__i16x8)wasm_v128_load(block + 48);
    __i16x8 l7 = (__i16x8)wasm_v128_load(block + 56);

    __i16x8 a0 = l0 + l4;
    __i16x8 a2 = l0 - l4;
    __i16x8 a4 = (l2 >> 1) + l6;
    __i16x8 a6 = (l6 >> 1) + l2;

    __i16x8 b0 = a0 + a6;
    __i16x8 b2 = a2 + a4;
    __i16x8 b4 = a2 - a4;
    __i16x8 b6 = a0 - a6;

    __i16x8 a1 = -l3 + l5 - l7 - (l7 >> 1);
    __i16x8 a3 = l1 + l7 - l3 - (l3 >> 1);
    __i16x8 a5 = -l1 + l7 + l5 + (l5 >> 1);
    __i16x8 a7 = l3 + l5 + l1 + (l1 >> 1);

    __i16x8 b1 = (a7 >> 2) + a1;
    __i16x8 b3 = a3 + (a5 >> 2);
    __i16x8 b5 = (a3 >> 2) - a5;
    __i16x8 b7 = a7 - (a1 >> 2);

    l0 = b0 + b7;
    l7 = b0 - b7;
    l1 = b2 + b5;
    l6 = b2 - b5;
    l2 = b4 + b3;
    l5 = b4 - b3;
    l3 = b6 + b1;
    l4 = b6 - b1;

    __i16x8 mll0 = (__i16x8)wasm_i16x8_shuffle(l0, l1, 0, 8, 1, 9, 2, 10, 3, 11);
    __i16x8 mll1 = (__i16x8)wasm_i16x8_shuffle(l2, l3, 0, 8, 1, 9, 2, 10, 3, 11);
    __i16x8 mll2 = (__i16x8)wasm_i16x8_shuffle(l4, l5, 0, 8, 1, 9, 2, 10, 3, 11);
    __i16x8 mll3 = (__i16x8)wasm_i16x8_shuffle(l6, l7, 0, 8, 1, 9, 2, 10, 3, 11);
    __i16x8 mlh0 = (__i16x8)wasm_i16x8_shuffle(l0, l1, 4, 12, 5, 13, 6, 14, 7, 15);
    __i16x8 mlh1 = (__i16x8)wasm_i16x8_shuffle(l2, l3, 4, 12, 5, 13, 6, 14, 7, 15);
    __i16x8 mlh2 = (__i16x8)wasm_i16x8_shuffle(l4, l5, 4, 12, 5, 13, 6, 14, 7, 15);
    __i16x8 mlh3 = (__i16x8)wasm_i16x8_shuffle(l6, l7, 4, 12, 5, 13, 6, 14, 7, 15);
    __i32x4 nlll0 = (__i32x4)wasm_i32x4_shuffle(mll0, mll1, 0, 4, 1, 5);
    __i32x4 nlll1 = (__i32x4)wasm_i32x4_shuffle(mll2, mll3, 0, 4, 1, 5);
    __i32x4 nllh0 = (__i32x4)wasm_i32x4_shuffle(mll0, mll1, 2, 6, 3, 7);
    __i32x4 nllh1 = (__i32x4)wasm_i32x4_shuffle(mll2, mll3, 2, 6, 3, 7);
    __i32x4 nlhl0 = (__i32x4)wasm_i32x4_shuffle(mlh0, mlh1, 0, 4, 1, 5);
    __i32x4 nlhl1 = (__i32x4)wasm_i32x4_shuffle(mlh2, mlh3, 0, 4, 1, 5);
    __i32x4 nlhh0 = (__i32x4)wasm_i32x4_shuffle(mlh0, mlh1, 2, 6, 3, 7);
    __i32x4 nlhh1 = (__i32x4)wasm_i32x4_shuffle(mlh2, mlh3, 2, 6, 3, 7);
    __i16x8 k0 = (__i16x8)wasm_i64x2_shuffle(nlll0, nlll1, 0, 2);
    __i16x8 k1 = (__i16x8)wasm_i64x2_shuffle(nlll0, nlll1, 1, 3);
    __i16x8 k2 = (__i16x8)wasm_i64x2_shuffle(nllh0, nllh1, 0, 2);
    __i16x8 k3 = (__i16x8)wasm_i64x2_shuffle(nllh0, nllh1, 1, 3);
    __i16x8 k4 = (__i16x8)wasm_i64x2_shuffle(nlhl0, nlhl1, 0, 2);
    __i16x8 k5 = (__i16x8)wasm_i64x2_shuffle(nlhl0, nlhl1, 1, 3);
    __i16x8 k6 = (__i16x8)wasm_i64x2_shuffle(nlhh0, nlhh1, 0, 2);
    __i16x8 k7 = (__i16x8)wasm_i64x2_shuffle(nlhh0, nlhh1, 1, 3);

    a0 = k0 + k4;
    a2 = k0 - k4;
    a4 = (k2 >> 1) - k6;
    a6 = (k6 >> 1) + k2;

    b0 = a0 + a6;
    b2 = a2 + a4;
    b4 = a2 - a4;
    b6 = a0 - a6;

    a1 = -k3 + k5 - k7 - (k7 >> 1);
    a3 = k1 + k7 - k3 - (k3 >> 1);
    a5 = -k1 + k7 + k5 + (k5 >> 1);
    a7 = k3 + k5 + k1 + (k1 >> 1);

    b1 = (a7 >> 2) + a1;
    b3 = a3 + (a5 >> 2);
    b5 = (a3 >> 2) - a5;
    b7 = a7 - (a1 >> 2);

    __i16x8 d0 = (__i16x8)wasm_u16x8_load8x8(dst + 0 * stride);
    __i16x8 d1 = (__i16x8)wasm_u16x8_load8x8(dst + 1 * stride);
    __i16x8 d2 = (__i16x8)wasm_u16x8_load8x8(dst + 2 * stride);
    __i16x8 d3 = (__i16x8)wasm_u16x8_load8x8(dst + 3 * stride);
    __i16x8 d4 = (__i16x8)wasm_u16x8_load8x8(dst + 4 * stride);
    __i16x8 d5 = (__i16x8)wasm_u16x8_load8x8(dst + 5 * stride);
    __i16x8 d6 = (__i16x8)wasm_u16x8_load8x8(dst + 6 * stride);
    __i16x8 d7 = (__i16x8)wasm_u16x8_load8x8(dst + 7 * stride);

    v128_t sum0 = wasm_u8x16_narrow_i16x8(
        (v128_t)(d0 + ((b0 + b7) >> 6)),
        (v128_t)(d1 + ((b2 + b5) >> 6))
    );
    v128_t sum1 = wasm_u8x16_narrow_i16x8(
        (v128_t)(d2 + ((b4 + b3) >> 6)),
        (v128_t)(d3 + ((b6 + b1) >> 6))
    );
    v128_t sum2 = wasm_u8x16_narrow_i16x8(
        (v128_t)(d4 + ((b6 - b1) >> 6)),
        (v128_t)(d5 + ((b4 - b3) >> 6))
    );
    v128_t sum3 = wasm_u8x16_narrow_i16x8(
        (v128_t)(d6 + ((b2 - b5) >> 6)),
        (v128_t)(d7 + ((b0 - b7) >> 6))
    );

    wasm_v128_store64_lane(dst + 0 * stride, sum0, 0);
    wasm_v128_store64_lane(dst + 1 * stride, sum0, 1);
    wasm_v128_store64_lane(dst + 2 * stride, sum1, 0);
    wasm_v128_store64_lane(dst + 3 * stride, sum1, 1);
    wasm_v128_store64_lane(dst + 4 * stride, sum2, 0);
    wasm_v128_store64_lane(dst + 5 * stride, sum2, 1);
    wasm_v128_store64_lane(dst + 6 * stride, sum3, 0);
    wasm_v128_store64_lane(dst + 7 * stride, sum3, 1);

    wasm_v128_store(block, (v128_t){0});
    wasm_v128_store(block + 8, (v128_t){0});
    wasm_v128_store(block + 16, (v128_t){0});
    wasm_v128_store(block + 24, (v128_t){0});
    wasm_v128_store(block + 32, (v128_t){0});
    wasm_v128_store(block + 40, (v128_t){0});
    wasm_v128_store(block + 48, (v128_t){0});
    wasm_v128_store(block + 56, (v128_t){0});
}
void ff_h264_idct_dc_add_8_websimd128(uint8_t *dst /*align 4*/, int16_t *block /*align 16*/, int stride) {
    int i;
    int dc = (block[0] + 32) >> 6;
    block[0] = 0;

    v128_t dc_vector = wasm_i16x8_splat(dc);

    __i16x8 d0 = (__i16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst));
    __i16x8 d1 = (__i16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst + 1 * stride));
    __i16x8 d2 = (__i16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst + 2 * stride));
    __i16x8 d3 = (__i16x8)wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst + 3 * stride));

    __i16x8 sum0 = (__i16x8)wasm_i16x8_shuffle(
        d0 + dc_vector,
        d1 + dc_vector,
        0, 1, 2, 3, 8, 9, 10, 11
    );
    __i16x8 sum1 = (__i16x8)wasm_i16x8_shuffle(
        d2 + dc_vector,
        d3 + dc_vector,
        0, 1, 2, 3, 8, 9, 10, 11
    );

    v128_t sum = wasm_u8x16_narrow_i16x8((v128_t)sum0, (v128_t)sum1);

    wasm_v128_store32_lane(dst, sum, 0);
    wasm_v128_store32_lane(dst + stride, sum, 1);
    wasm_v128_store32_lane(dst + 2 * stride, sum, 2);
    wasm_v128_store32_lane(dst + 3 * stride, sum, 3);
}
void ff_h264_idct8_dc_add_8_websimd128(uint8_t *dst /*align 8*/, int16_t *block /*align 16*/, int stride) {
    int i;
    int dc = (block[0] + 32) >> 6;
    block[0] = 0;

    __i16x8 dc_vector = (__i16x8)wasm_i16x8_splat(dc);

    __i16x8 d0 = (__i16x8)wasm_u16x8_load8x8(dst + 0 * stride);
    __i16x8 d1 = (__i16x8)wasm_u16x8_load8x8(dst + 1 * stride);
    __i16x8 d2 = (__i16x8)wasm_u16x8_load8x8(dst + 2 * stride);
    __i16x8 d3 = (__i16x8)wasm_u16x8_load8x8(dst + 3 * stride);
    __i16x8 d4 = (__i16x8)wasm_u16x8_load8x8(dst + 4 * stride);
    __i16x8 d5 = (__i16x8)wasm_u16x8_load8x8(dst + 5 * stride);
    __i16x8 d6 = (__i16x8)wasm_u16x8_load8x8(dst + 6 * stride);
    __i16x8 d7 = (__i16x8)wasm_u16x8_load8x8(dst + 7 * stride);

    v128_t sum0 = wasm_u8x16_narrow_i16x8(
        (v128_t)(d0 + dc_vector),
        (v128_t)(d1 + dc_vector)
    );
    v128_t sum1 = wasm_u8x16_narrow_i16x8(
        (v128_t)(d2 + dc_vector),
        (v128_t)(d3 + dc_vector)
    );
    v128_t sum2 = wasm_u8x16_narrow_i16x8(
        (v128_t)(d4 + dc_vector),
        (v128_t)(d5 + dc_vector)
    );
    v128_t sum3 = wasm_u8x16_narrow_i16x8(
        (v128_t)(d6 + dc_vector),
        (v128_t)(d7 + dc_vector)
    );

    wasm_v128_store64_lane(dst + 0 * stride, sum0, 0);
    wasm_v128_store64_lane(dst + 1 * stride, sum0, 1);
    wasm_v128_store64_lane(dst + 2 * stride, sum1, 0);
    wasm_v128_store64_lane(dst + 3 * stride, sum1, 1);
    wasm_v128_store64_lane(dst + 4 * stride, sum2, 0);
    wasm_v128_store64_lane(dst + 5 * stride, sum2, 1);
    wasm_v128_store64_lane(dst + 6 * stride, sum3, 0);
    wasm_v128_store64_lane(dst + 7 * stride, sum3, 1);
}
void ff_h264_idct_add16_8_websimd128(uint8_t *dst /*align 16*/, const int *block_offset, int16_t *block /*align 16*/, int stride, const uint8_t nnzc[15 * 8]) {
    int i;
    for(i=0; i<16; i++){
        int nnz = nnzc[ scan8[i] ];
        if(nnz){
            if(nnz==1 && ((int16_t*)block)[i*16]) ff_h264_idct_dc_add_8_websimd128(dst + block_offset[i], block + i*16*sizeof(uint8_t), stride);
            else ff_h264_idct_add_8_websimd128(dst + block_offset[i], block + i*16*sizeof(uint8_t), stride);
        }
    }
}
void ff_h264_idct8_add4_8_websimd128(uint8_t *dst /*align 16*/, const int *block_offset, int16_t *block /*align 16*/, int stride, const uint8_t nnzc[15 * 8]) {
    int i;
    for(i=0; i<16; i+=4){
        int nnz = nnzc[ scan8[i] ];
        if(nnz){
            if(nnz==1 && ((int16_t*)block)[i*16]) ff_h264_idct8_dc_add_8_websimd128(dst + block_offset[i], block + i*16*sizeof(uint8_t), stride);
            else ff_h264_idct8_add_8_websimd128(dst + block_offset[i], block + i*16*sizeof(uint8_t), stride);
        }
    }
}
void ff_h264_idct_add8_8_websimd128(uint8_t **dest /*align 16*/, const int *block_offset, int16_t *block /*align 16*/, int stride, const uint8_t nnzc[15 * 8]) {
    int i, j;
    for(j=1; j<3; j++){
        for(i=j*16; i<j*16+4; i++){
            if(nnzc[ scan8[i] ])
                ff_h264_idct_add_8_websimd128(dest[j-1] + block_offset[i], block + i*16*sizeof(uint8_t), stride);
            else if(((int16_t*)block)[i*16])
                ff_h264_idct_dc_add_8_websimd128(dest[j-1] + block_offset[i], block + i*16*sizeof(uint8_t), stride);
        }
    }
}
void ff_h264_idct_add16intra_8_websimd128(uint8_t *dst /*align 16*/, const int *block_offset, int16_t *block /*align 16*/, int stride, const uint8_t nnzc[15 * 8]) {
    int i;
    for(i=0; i<16; i++){
        if(nnzc[ scan8[i] ]) ff_h264_idct_add_8_websimd128(dst + block_offset[i], block + i*16*sizeof(uint8_t), stride);
        else if(((int16_t*)block)[i*16]) ff_h264_idct_dc_add_8_websimd128(dst + block_offset[i], block + i*16*sizeof(uint8_t), stride);
    }
}
void ff_h264_idct_add8_422_8_websimd128(uint8_t **dest, const int *block_offset, int16_t *block, int stride, const uint8_t nnzc[15*8]) {
    int i, j;

    for(j=1; j<3; j++){
        for(i=j*16; i<j*16+4; i++){
            if(nnzc[ scan8[i] ])
                ff_h264_idct_add_8_websimd128(dest[j-1] + block_offset[i], block + i*16*sizeof(uint8_t), stride);
            else if(((int16_t*)block)[i*16])
                ff_h264_idct_dc_add_8_websimd128(dest[j-1] + block_offset[i], block + i*16*sizeof(uint8_t), stride);
        }
    }

    for(j=1; j<3; j++){
        for(i=j*16+4; i<j*16+8; i++){
            if(nnzc[ scan8[i+4] ])
                ff_h264_idct_add_8_websimd128(dest[j-1] + block_offset[i+4], block + i*16*sizeof(uint8_t), stride);
            else if(((int16_t*)block)[i*16])
                ff_h264_idct_dc_add_8_websimd128(dest[j-1] + block_offset[i+4], block + i*16*sizeof(uint8_t), stride);
        }
    }
}

void ff_h264_luma_dc_dequant_idct_8_websimd128(int16_t *output, int16_t *input /*align 16*/, int qmul) {
    int i;
    static const uint8_t x_offset[4]={0, 2*16, 8*16, 10*16};

    v128_t b0 = wasm_i32x4_make(input[0 + 4*0], input[0 + 4*1], input[0 + 4*2], input[0 + 4*3]);
    v128_t b1 = wasm_i32x4_make(input[1 + 4*0], input[1 + 4*1], input[1 + 4*2], input[1 + 4*3]);
    v128_t b2 = wasm_i32x4_make(input[2 + 4*0], input[2 + 4*1], input[2 + 4*2], input[2 + 4*3]);
    v128_t b3 = wasm_i32x4_make(input[3 + 4*0], input[3 + 4*1], input[3 + 4*2], input[3 + 4*3]);

    v128_t z0 = (v128_t)((__i32x4)b0 + (__i32x4)b1);
    v128_t z1 = (v128_t)((__i32x4)b0 - (__i32x4)b1);
    v128_t z2 = (v128_t)((__i32x4)b2 - (__i32x4)b3);
    v128_t z3 = (v128_t)((__i32x4)b2 + (__i32x4)b3);

    v128_t temp0 = (v128_t)((__i32x4)z0 + (__i32x4)z3);
    v128_t temp1 = (v128_t)((__i32x4)z0 - (__i32x4)z3);
    v128_t temp2 = (v128_t)((__i32x4)z1 - (__i32x4)z2);
    v128_t temp3 = (v128_t)((__i32x4)z1 + (__i32x4)z2);

    b0 = wasm_i32x4_make(((__i32x4)temp0)[0], ((__i32x4)temp1)[0], ((__i32x4)temp2)[0], ((__i32x4)temp3)[0]);
    b1 = wasm_i32x4_make(((__i32x4)temp0)[1], ((__i32x4)temp1)[1], ((__i32x4)temp2)[1], ((__i32x4)temp3)[1]);
    b2 = wasm_i32x4_make(((__i32x4)temp0)[2], ((__i32x4)temp1)[2], ((__i32x4)temp2)[2], ((__i32x4)temp3)[2]);
    b3 = wasm_i32x4_make(((__i32x4)temp0)[3], ((__i32x4)temp1)[3], ((__i32x4)temp2)[3], ((__i32x4)temp3)[3]);

    z0 = (v128_t)((__i32x4)b0 + (__i32x4)b2);
    z1 = (v128_t)((__i32x4)b0 - (__i32x4)b2);
    z2 = (v128_t)((__i32x4)b1 - (__i32x4)b3);
    z3 = (v128_t)((__i32x4)b1 + (__i32x4)b3);

    v128_t o0 = (v128_t)((((__i32x4)z0 + (__i32x4)z3) * qmul + 128) >> 8);
    v128_t o1 = (v128_t)((((__i32x4)z1 + (__i32x4)z2) * qmul + 128) >> 8);
    v128_t o2 = (v128_t)((((__i32x4)z1 - (__i32x4)z2) * qmul + 128) >> 8);
    v128_t o3 = (v128_t)((((__i32x4)z0 - (__i32x4)z3) * qmul + 128) >> 8);

    int32_t* pointer = &o0;
    for(i = 0; i < 4; i++) {
        const int offset= x_offset[i];
        output[16 * 0 + offset] = *(pointer++);
    }
    pointer = &o1;
    for(i = 0; i < 4; i++) {
        const int offset= x_offset[i];
        output[16 * 1 + offset] = *(pointer++);
    }
    pointer = &o2;
    for(i = 0; i < 4; i++) {
        const int offset= x_offset[i];
        output[16 * 4 + offset] = *(pointer++);
    }
    pointer = &o3;
    for(i = 0; i < 4; i++) {
        const int offset= x_offset[i];
        output[16 * 5 + offset] = *(pointer++);
    }
}

void ff_h264_chroma_dc_dequant_idct_8_websimd128(int16_t *block, int qmul) {
    const int stride= 16 * 2;
    const int xStride= 16;
    int16_t a,b,c,d,e;

    a= block[stride * 0 + xStride * 0];
    b= block[stride * 0 + xStride * 1];
    c= block[stride * 1 + xStride * 0];
    d= block[stride * 1 + xStride * 1];

    e= a - b;
    a= a + b;
    b= c - d;
    c= c + d;

    v128_t z0 = wasm_i32x4_make(a, e, a, e);
    v128_t z1 = wasm_i32x4_make(c, b, -c, -b);

    __i32x4 result = ((z0 + z1) * qmul) >> 7;

    block[stride*0 + xStride*0]= result[0];
    block[stride*0 + xStride*1]= result[1];
    block[stride*1 + xStride*0]= result[2];
    block[stride*1 + xStride*1]= result[3];
}

void ff_h264_chroma422_dc_dequant_idct_8_websimd128(int16_t *block, int qmul) {
    const int stride= 16 * 2;
    const int xStride= 16;
    int i;
    static const uint8_t x_offset[2] = {0, 16};

    v128_t b0 = wasm_i32x4_make(block[stride * 0 + xStride * 0], block[stride * 1 + xStride * 0], block[stride * 2 + xStride * 0], block[stride * 3 + xStride * 0]);
    v128_t b1 = wasm_i32x4_make(block[stride * 0 + xStride * 1], block[stride * 1 + xStride * 1], block[stride * 2 + xStride * 1], block[stride * 3 + xStride * 1]);

    v128_t temp0 = (v128_t)((__i32x4)b0 + (__i32x4)b1);
    v128_t temp1 = (v128_t)((__i32x4)b0 - (__i32x4)b1);

    b0 = wasm_i64x2_make(temp0[0], temp1[0]);
    b1 = wasm_i64x2_make(temp0[1], temp1[1]);
    v128_t b2 = wasm_i64x2_make(temp0[2], temp1[2]);
    v128_t b3 = wasm_i64x2_make(temp0[3], temp1[3]);

    v128_t z0 = (v128_t)((__i64x2)b0 + (__i64x2)b2);
    v128_t z1 = (v128_t)((__i64x2)b0 - (__i64x2)b2);
    v128_t z2 = (v128_t)((__i64x2)b1 - (__i64x2)b3);
    v128_t z3 = (v128_t)((__i64x2)b1 + (__i64x2)b3);

    v128_t o0 = (v128_t)((((__i64x2)z0 + (__i64x2)z3) * qmul + 128) >> 8);
    v128_t o1 = (v128_t)((((__i64x2)z1 + (__i64x2)z2) * qmul + 128) >> 8);
    v128_t o2 = (v128_t)((((__i64x2)z1 - (__i64x2)z2) * qmul + 128) >> 8);
    v128_t o3 = (v128_t)((((__i64x2)z0 - (__i64x2)z3) * qmul + 128) >> 8);

    int64_t* pointer = &o0;
    for(i = 0; i < 2; i++) {
        const int offset= x_offset[i];
        block[stride * 0 + offset] = *(pointer++);
    }
    pointer = &o1;
    for(i = 0; i < 2; i++) {
        const int offset= x_offset[i];
        block[stride * 1 + offset] = *(pointer++);
    }
    pointer = &o2;
    for(i = 0; i < 2; i++) {
        const int offset= x_offset[i];
        block[stride * 2 + offset] = *(pointer++);
    }
    pointer = &o3;
    for(i = 0; i < 2; i++) {
        const int offset= x_offset[i];
        block[stride * 3 + offset] = *(pointer++);
    }
}

void ff_h264_weight_pixels16_8_websimd128(uint8_t *block, ptrdiff_t stride, int height, int log2_denom, int weight, int offset) {
    int y;
    offset = (unsigned)offset << log2_denom;
    if (log2_denom) {
        offset += 1 << (log2_denom - 1);
    }

    __i16x8 weight_vector = (__i16x8)wasm_i16x8_splat(weight);
    __i16x8 offset_vector = (__i16x8)wasm_i16x8_splat(offset);
    __i16x8 log2_denom_vector = (__i16x8)wasm_i16x8_splat(log2_denom);

    for (y = 0; y < height; y++, block += stride) {
        v128_t b0 = wasm_u16x8_load8x8(block);
        v128_t b1 = wasm_u16x8_load8x8(block + 8);
        __i16x8 s0 = (((__i16x8)b0 * weight_vector + offset_vector) >> log2_denom_vector);
        __i16x8 s1 = (((__i16x8)b1 * weight_vector + offset_vector) >> log2_denom_vector);
        v128_t s = wasm_u8x16_narrow_i16x8((v128_t)s0, (v128_t)s1);
        wasm_v128_store(block, s);
    }
}

void ff_h264_biweight_pixels16_8_websimd128(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int height, int log2_denom, int weightd, int weights, int offset) {
    int y;
    offset = (unsigned)((offset + 1) | 1) << log2_denom;

    __i16x8 weights_vector = (__i16x8)wasm_i16x8_splat(weights);
    __i16x8 offset_vector = (__i16x8)wasm_i16x8_splat(offset);
    __i16x8 weightd_vector = (__i16x8)wasm_i16x8_splat(weightd);
    __i16x8 log2_denom_vector = (__i16x8)wasm_i16x8_splat(log2_denom + 1);

    for (y = 0; y < height; y++, dst += stride, src += stride) {
        v128_t dst0 = wasm_u16x8_load8x8(dst);
        v128_t dst1 = wasm_u16x8_load8x8(dst + 8);
        v128_t src0 = wasm_u16x8_load8x8(src);
        v128_t src1 = wasm_u16x8_load8x8(src + 8);
        __i16x8 s0 = (((__i16x8)src0 * weights_vector + (__i16x8)dst0 * weightd_vector + offset_vector) >> log2_denom_vector);
        __i16x8 s1 = (((__i16x8)src1 * weights_vector + (__i16x8)dst1 * weightd_vector + offset_vector) >> log2_denom_vector);
        v128_t s = wasm_u8x16_narrow_i16x8((v128_t)s0, (v128_t)s1);
        wasm_v128_store(dst, s);
    }
}

void ff_h264_weight_pixels8_8_websimd128(uint8_t *block, ptrdiff_t stride, int height, int log2_denom, int weight, int offset) {
    int y;
    offset = (unsigned)offset << log2_denom;
    if (log2_denom) {
        offset += 1 << (log2_denom - 1);
    }

    __i16x8 weight_vector = (__i16x8)wasm_i16x8_splat(weight);
    __i16x8 offset_vector = (__i16x8)wasm_i16x8_splat(offset);
    __i16x8 log2_denom_vector = (__i16x8)wasm_i16x8_splat(log2_denom);

    for (y = 0; y < height; y++, block += stride) {
        v128_t b0 = wasm_u16x8_load8x8(block);
        __i16x8 s0 = (((__i16x8)b0 * weight_vector + offset_vector) >> log2_denom_vector);
        v128_t s = wasm_u8x16_narrow_i16x8((v128_t)s0, (v128_t)s0);
        wasm_v128_store64_lane(block, s, 0);
    }
}

void ff_h264_biweight_pixels8_8_websimd128(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int height, int log2_denom, int weightd, int weights, int offset) {
    int y;
    offset = (unsigned)((offset + 1) | 1) << log2_denom;

    __i16x8 weights_vector = (__i16x8)wasm_i16x8_splat(weights);
    __i16x8 offset_vector = (__i16x8)wasm_i16x8_splat(offset);
    __i16x8 weightd_vector = (__i16x8)wasm_i16x8_splat(weightd);
    __i16x8 log2_denom_vector = (__i16x8)wasm_i16x8_splat(log2_denom + 1);

    for (y = 0; y < height; y++, dst += stride, src += stride) {
        v128_t dst0 = wasm_u16x8_load8x8(dst);
        v128_t src0 = wasm_u16x8_load8x8(src);
        __i16x8 s0 = (((__i16x8)src0 * weights_vector + (__i16x8)dst0 * weightd_vector + offset_vector) >> log2_denom_vector);
        v128_t s = wasm_u8x16_narrow_i16x8((v128_t)s0, (v128_t)s0);
        wasm_v128_store64_lane(dst, s, 0);
    }
}

void ff_h264_weight_pixels4_8_websimd128(uint8_t *block, ptrdiff_t stride, int height, int log2_denom, int weight, int offset) {
    int y;
    offset = (unsigned)offset << log2_denom;
    if (log2_denom) {
        offset += 1 << (log2_denom - 1);
    }

    __i16x8 weight_vector = (__i16x8)wasm_u16x8_splat(weight);
    __i16x8 offset_vector = (__i16x8)wasm_u16x8_splat(offset);
    __i16x8 log2_denom_vector = (__i16x8)wasm_i16x8_splat(log2_denom);

    for (y = 0; y < height; y++, block += stride) {
        v128_t b0 = wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(block));
        __i16x8 s0 = (((__i16x8)b0 * weight_vector + offset_vector) >> log2_denom_vector);
        v128_t s = wasm_u8x16_narrow_i16x8((v128_t)s0, (v128_t)s0);
        wasm_v128_store32_lane(block, s, 0);
    }
}

void ff_h264_biweight_pixels4_8_websimd128(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int height, int log2_denom, int weightd, int weights, int offset) {
    int y;
    offset = (unsigned)((offset + 1) | 1) << log2_denom;

    __i16x8 weights_vector = (__i16x8)wasm_u16x8_splat(weights);
    __i16x8 offset_vector = (__i16x8)wasm_u16x8_splat(offset);
    __i16x8 weightd_vector = (__i16x8)wasm_u16x8_splat(weightd);
    __i16x8 log2_denom_vector = (__i16x8)wasm_i16x8_splat(log2_denom + 1);

    for (y = 0; y < height; y++, dst += stride, src += stride) {
        v128_t dst0 = wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(dst));
        v128_t src0 = wasm_u16x8_extend_low_u8x16(wasm_v128_load32_zero(src));
        __i16x8 s0 = ((__i16x8)src0 * weights_vector + (__i16x8)dst0 * weightd_vector + offset_vector) >> log2_denom_vector;
        v128_t s = wasm_u8x16_narrow_i16x8((v128_t)s0, (v128_t)s0);
        wasm_v128_store32_lane(dst, s, 0);
    }
}

/* bypass-transform */
void ff_h264_add_pixels8_clear_8_websimd128(uint8_t *dst, int16_t *src, int stride) {
    v128_t d = wasm_u8x16_make(
        dst[0], dst[1], dst[2], dst[3],
        dst[0 + stride], dst[1 + stride], dst[2 + stride], dst[3 + stride],
        dst[0 + stride * 2], dst[1 + stride * 2], dst[2 + stride * 2], dst[3 + stride * 2],
        dst[0 + stride * 3], dst[1 + stride * 3], dst[2 + stride * 3], dst[3 + stride * 3]
    );

    v128_t s = wasm_v128_load(src);

    d = wasm_u8x16_add_sat(d, s);

    wasm_v128_store32_lane(dst, d, 0);
    wasm_v128_store32_lane(dst + stride, d, 1);
    wasm_v128_store32_lane(dst + 2 * stride, d, 2);
    wasm_v128_store32_lane(dst + 3 * stride, d, 3);

    wasm_v128_store(src, (v128_t){0});
    wasm_v128_store(src + 8, (v128_t){0});
}
void ff_h264_add_pixels4_clear_8_websimd128(uint8_t *dst, int16_t *src, int stride) {
    int i;
    for (i = 0; i < 4; i++) {
        v128_t d0 = wasm_u16x8_load8x8(dst);
        v128_t d1 = wasm_u16x8_load8x8(dst + stride);
        v128_t s =  wasm_v128_load(src);

        v128_t d = wasm_u8x16_narrow_i16x8(d0, d1);

        d = wasm_u8x16_add_sat(d, s);

        wasm_v128_store64_lane(dst, d, 0);
        wasm_v128_store64_lane(dst + stride, d, 1);

        dst += stride * 2;
    }

    wasm_v128_store(src, (v128_t){0});
    wasm_v128_store(src + 8, (v128_t){0});
    wasm_v128_store(src + 16, (v128_t){0});
    wasm_v128_store(src + 24, (v128_t){0});
    wasm_v128_store(src + 32, (v128_t){0});
    wasm_v128_store(src + 40, (v128_t){0});
    wasm_v128_store(src + 48, (v128_t){0});
    wasm_v128_store(src + 56, (v128_t){0});
}

/**
 * Search buf from the start for up to size bytes. Return the index
 * of a zero byte, or >= size if not found. Ideally, use lookahead
 * to filter out any zero bytes that are known to not be followed by
 * one or more further zero bytes and a one byte. Better still, filter
 * out any bytes that form the trailing_zero_8bits syntax element too.
 */
// int startcode_find_candidate_8_websimd128(const uint8_t *buf, int size) {

// }