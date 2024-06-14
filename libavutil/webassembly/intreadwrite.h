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

#ifndef AVUTIL_WEBASSEMBLY_INTREADWRITE_H
#define AVUTIL_WEBASSEMBLY_INTREADWRITE_H

#include "config.h"
#include "libavutil/attributes.h"
#include <wasm_simd128.h>

#if HAVE_WEBSIMD128

#define AV_COPY64 AV_COPY64
static av_always_inline void AV_COPY64(void *d, const void *s) {
    wasm_v128_store64_lane(d, wasm_v128_load64_zero(s), 0);
}

#define AV_SWAP64 AV_SWAP64
static av_always_inline void AV_SWAP64(void *a, void *b) {
    v128_t s = wasm_v128_load64_zero(a);
    wasm_v128_store64_lane(a, wasm_v128_load64_zero(b), 0);
    wasm_v128_store64_lane(b, s, 0);
}

#define AV_ZERO64 AV_ZERO64
static av_always_inline void AV_ZERO64(void *d) {
    wasm_v128_store64_lane(d, (v128_t){0}, 0);
}

#define AV_COPY128 AV_COPY128
static av_always_inline void AV_COPY128(void *d, const void *s) {
    wasm_v128_store(d, wasm_v128_load(s));
}

#define AV_ZERO128 AV_ZERO128
static av_always_inline void AV_ZERO128(void *d) {
    wasm_v128_store(d, (v128_t){0});
}
#endif /* HAVE_WEBSIMD128 */

#endif /* AVUTIL_WEBASSEMBLY_INTMATH_H */
