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

#ifndef AVUTIL_WEBASSEMBLY_INTMATH_H
#define AVUTIL_WEBASSEMBLY_INTMATH_H

#include "config.h"
#include "attributes.h"
#include <wasm_simd128.h>

#if HAVE_WEBSIMD128

#define av_clipd av_clipd_webassembly
static av_always_inline av_const double av_clipd_webassembly(double a, double amin, double amax) {
    __f64x2 maxp = (__f64x2)wasm_f64x2_splat(amax);
    __f64x2 minp = (__f64x2)wasm_f64x2_splat(amin);
    __f64x2 ap = (__f64x2)wasm_f64x2_splat(a);
    ap = wasm_f64x2_pmax((v128_t)ap, (v128_t)maxp);
    ap = wasm_f64x2_pmin((v128_t)ap, (v128_t)minp);
    return ap[0];
}

#define av_clipf av_clipf_webassembly
static av_always_inline av_const float av_clipf_webassembly(float a, float amin, float amax) {
    __f32x4 maxp = (__f32x4)wasm_f32x4_splat(amax);
    __f32x4 minp = (__f32x4)wasm_f32x4_splat(amin);
    __f32x4 ap = (__f32x4)wasm_f32x4_splat(a);
    ap = wasm_f32x4_pmax((v128_t)ap, (v128_t)maxp);
    ap = wasm_f32x4_pmin((v128_t)ap, (v128_t)minp);
    return ap[0];
}

#endif /* HAVE_WEBSIMD128 */

#endif /* AVUTIL_WEBASSEMBLY_INTMATH_H */
