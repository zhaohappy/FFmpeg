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

#include "libavutil/fixed_dsp.h"
#include <wasm_simd128.h>

static void ff_butterflies_fixed_webassembly(int *v1, int *v2, int len) {
    for (int i = 0; i < len; i += 4) {
        v128_t a = wasm_v128_load(&v1[i]);
        v128_t b = wasm_v128_load(&v2[i]);
        wasm_v128_store(&v1[i], wasm_i32x4_add(a, b));
        wasm_v128_store(&v2[i], wasm_i32x4_sub(a, b));
    }
}

void ff_fixed_dsp_init_webassembly(AVFixedDSPContext *fdsp)
{

    #if HAVE_WEBSIMD128
        fdsp->butterflies_fixed = ff_butterflies_fixed_webassembly;
    #endif
}
