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

#include "libavutil/float_dsp.h"
#include <wasm_simd128.h>
#include <stdio.h>

static void ff_vector_fmul_webassembly(float *dst, const float *src0, const float *src1, int len) {
    int length = (len >> 2) << 2;
    int i;
    for (i = 0; i < length; i += 4) {
        wasm_v128_store(
            &dst[i],
            wasm_f32x4_mul(
                wasm_v128_load(&src0[i]),
                wasm_v128_load(&src1[i])
            )
        );
    }
    for (i = length; i < len; i++) {
        dst[i] = src0[i] * src1[i];
    }
}

static void ff_vector_dmul_webassembly(double *dst, const double *src0, const double *src1, int len) {
    int length = (len >> 1) << 1;
    int i;
    for (i = 0; i < length; i += 2) {
        wasm_v128_store(
            &dst[i],
            wasm_f64x2_mul(
                wasm_v128_load(&src0[i]),
                wasm_v128_load(&src1[i])
            )
        );
    }
    for (i = length; i < len; i++) {
        dst[i] = src0[i] * src1[i];
    }
}

static void ff_vector_fmac_scalar_webassembly(float *dst, const float *src, float mul, int len) {
    int length = (len >> 2) << 2;
    int i;
    v128_t v128mul = wasm_f32x4_splat(mul);

    for (i = 0; i < length; i += 4) {
        wasm_v128_store(
            &dst[i],
            wasm_f32x4_mul(
                wasm_v128_load(&src[i]),
                v128mul
            )
        );
    }
    for (i = length; i < len; i++) {
        dst[i] += src[i] * mul;
    }
}

static void ff_vector_fmul_scalar_webassembly(float *dst, const float *src, float mul, int len) {
    int length = (len >> 2) << 2;
    int i;
    v128_t v128mul = wasm_f32x4_splat(mul);

    for (i = 0; i < length; i += 4) {
        wasm_v128_store(
            &dst[i],
            wasm_f32x4_mul(
                wasm_v128_load(&src[i]),
                v128mul
            )
        );
    }

    for (i = length; i < len; i++) {
        dst[i] = src[i] * mul;
    }
}

static void ff_vector_dmac_scalar_webassembly(double *dst, const double *src, double mul, int len) {
    int length = (len >> 1) << 1;
    int i;
    v128_t v128mul = wasm_f64x2_splat(mul);

    for (i = 0; i < length; i += 2) {
        wasm_v128_store(
            &dst[i],
            wasm_f64x2_mul(
                wasm_v128_load(&src[i]),
                v128mul
            )
        );
    }
    for (i = length; i < len; i++) {
        dst[i] += src[i] * mul;
    }
}

static void ff_vector_dmul_scalar_webassembly(double *dst, const double *src, double mul, int len) {
    int length = (len >> 1) << 1;
    int i;
    v128_t v128mul = wasm_f64x2_splat(mul);

    for (i = 0; i < length; i += 2) {
        wasm_v128_store(
            &dst[i],
            wasm_f64x2_mul(
                wasm_v128_load(&src[i]),
                v128mul
            )
        );
    }
    for (i = length; i < len; i++) {
        dst[i] = src[i] * mul;
    }
}


static void ff_vector_fmul_window_webassembly(float *dst, const float *src0, const float *src1, const float *win, int len) {
    
}

static void ff_vector_fmul_add_webassembly(float *dst, const float *src0, const float *src1, const float *src2, int len) {
    int length = (len >> 2) << 2;
    int i;
    for (i = 0; i < length; i += 4) {
        wasm_v128_store(
            &dst[i],
            wasm_f32x4_add(
                wasm_f32x4_mul(
                    wasm_v128_load(&src0[i]),
                    wasm_v128_load(&src1[i])
                ),
                wasm_v128_load(&src2[i])
            ) 
        );
    }
    for (i = length; i < len; i++) {
        dst[i] = src0[i] * src1[i] + src2[i];
    }
}

static void ff_vector_fmul_reverse_webassembly(float *dst, const float *src0, int len) {

}

static float ff_scalarproduct_float_webassembly(const float *v1, const float *v2, int len) {
    int length = (len >> 2) << 2;
    int i;
    v128_t sum = wasm_f32x4_splat(0.0);

    for (i = 0; i < length; i += 4) {
        sum = wasm_f32x4_add(
            sum,
            wasm_f32x4_mul(
                wasm_v128_load(&v1[i]),
                wasm_v128_load(&v2[i])
            )
        );
    }
    float p = ((__f32x4)sum)[0] + ((__f32x4)sum)[1] + ((__f32x4)sum)[2] + ((__f32x4)sum)[3];

    for (i = length; i < len; i++) {
        p += v1[i] * v2[i];
    }

    return p;
}

static void ff_butterflies_float_webassembly(float* src0, float* src1, int len) {
    int length = (len >> 2) << 2;
    int i;

    for (i = 0; i < length; i += 4) {
        v128_t t = wasm_f32x4_sub(wasm_v128_load(&src0[i]), wasm_v128_load(&src1[i]));
        wasm_v128_store(
            &src0[i],
            wasm_f32x4_add(wasm_v128_load(&src0[i]), wasm_v128_load(&src1[i]))
        );
        wasm_v128_store(&src1[i], t);
    }

    for (i = length; i < len; i++) {
        float t = src0[i] - src1[i];
        src0[i] += src1[i];
        src1[i] = t;
    }
}

void ff_float_dsp_init_webassembly(AVFloatDSPContext *fdsp)
{
    #if HAVE_WEBSIMD128

    fdsp->vector_fmul = ff_vector_fmul_webassembly;
    fdsp->vector_dmul = ff_vector_dmul_webassembly;
    fdsp->vector_fmac_scalar = ff_vector_fmac_scalar_webassembly;
    fdsp->vector_fmul_scalar = ff_vector_fmul_scalar_webassembly;

    fdsp->vector_dmac_scalar = ff_vector_dmac_scalar_webassembly;
    fdsp->vector_dmul_scalar = ff_vector_dmul_scalar_webassembly;
    fdsp->vector_fmul_add = ff_vector_fmul_add_webassembly;
    fdsp->scalarproduct_float = ff_scalarproduct_float_webassembly;
    fdsp->butterflies_float = ff_butterflies_float_webassembly;
    
    #endif
}
