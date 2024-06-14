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
#include "h264dsp_webassembly.h"

#if HAVE_WEBSIMD128
static av_cold void h264dsp_init_websimd128(
    H264DSPContext *c,
    const int bit_depth,
    const int chroma_format_idc)
{
    if (bit_depth == 8) {
        c->h264_idct_add = ff_h264_idct_add_8_websimd128;
        c->h264_idct_dc_add = ff_h264_idct_dc_add_8_websimd128;

        c->h264_idct8_add = ff_h264_idct8_add_8_websimd128;
        c->h264_idct8_dc_add = ff_h264_idct8_dc_add_8_websimd128;

        c->h264_idct_add16 = ff_h264_idct_add16_8_websimd128;
        c->h264_idct8_add4 = ff_h264_idct8_add4_8_websimd128;

        if (chroma_format_idc <= 1) {
            c->h264_idct_add8 = ff_h264_idct_add8_8_websimd128;
        }
        else {
            c->h264_idct_add8 = ff_h264_idct_add8_422_8_websimd128;
        }
        c->h264_idct_add16intra = ff_h264_idct_add16intra_8_websimd128;
        c->h264_luma_dc_dequant_idct = ff_h264_luma_dc_dequant_idct_8_websimd128;

        if (chroma_format_idc <= 1) {
            c->h264_chroma_dc_dequant_idct = ff_h264_chroma_dc_dequant_idct_8_websimd128;
        }
        else {
            c->h264_chroma_dc_dequant_idct = ff_h264_chroma422_dc_dequant_idct_8_websimd128;
        }

        c->weight_h264_pixels_tab[0] = ff_h264_weight_pixels16_8_websimd128;
        c->weight_h264_pixels_tab[1] = ff_h264_weight_pixels8_8_websimd128;
        c->weight_h264_pixels_tab[2] = ff_h264_weight_pixels4_8_websimd128;

        c->biweight_h264_pixels_tab[0] = ff_h264_biweight_pixels16_8_websimd128;
        c->biweight_h264_pixels_tab[1] = ff_h264_biweight_pixels8_8_websimd128;
        c->biweight_h264_pixels_tab[2] = ff_h264_biweight_pixels4_8_websimd128;

        c->h264_add_pixels4_clear = ff_h264_add_pixels4_clear_8_websimd128;
        c->h264_add_pixels8_clear = ff_h264_add_pixels8_clear_8_websimd128;

        c->h264_v_loop_filter_luma = ff_h264_v_loop_filter_luma_8_websimd128;
        c->h264_h_loop_filter_luma = ff_h264_h_loop_filter_luma_8_websimd128;
        c->h264_h_loop_filter_luma_mbaff = ff_h264_h_loop_filter_luma_mbaff_8_websimd128;

        c->h264_v_loop_filter_luma_intra = ff_h264_v_loop_filter_luma_intra_8_websimd128;
        c->h264_h_loop_filter_luma_intra = ff_h264_h_loop_filter_luma_intra_8_websimd128;
        c->h264_h_loop_filter_luma_mbaff_intra = ff_h264_h_loop_filter_luma_mbaff_intra_8_websimd128;

        c->h264_v_loop_filter_chroma = ff_h264_v_loop_filter_chroma_8_websimd128;
        if (chroma_format_idc <= 1) {
            c->h264_h_loop_filter_chroma = ff_h264_h_loop_filter_chroma_8_websimd128;
        }
        else {
            c->h264_h_loop_filter_chroma = ff_h264_h_loop_filter_chroma422_8_websimd128;
        }

        if (chroma_format_idc > 1) {
            c->h264_h_loop_filter_chroma_mbaff = ff_h264_h_loop_filter_chroma422_mbaff_8_websimd128;
        }
            
        c->h264_v_loop_filter_chroma_intra = ff_h264_v_loop_filter_chroma_intra_8_websimd128;

        if (chroma_format_idc <= 1) {
            c->h264_h_loop_filter_chroma_intra = ff_h264_h_loop_filter_chroma_intra_8_websimd128;
        }
    }
}
#endif  // #if HAVE_WEBSIMD128

av_cold void ff_h264dsp_init_webassembly(
    H264DSPContext *c,
    const int bit_depth,
    const int chroma_format_idc)
{
#if HAVE_WEBSIMD128
    h264dsp_init_websimd128(c, bit_depth, chroma_format_idc);
#endif /* HAVE_WEBSIMD128 */
}
