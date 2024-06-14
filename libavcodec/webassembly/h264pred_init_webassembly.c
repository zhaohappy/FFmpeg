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
#include "h264pred_webassembly.h"

#if HAVE_WEBSIMD128
static av_cold void h264_pred_init_websimd128(H264PredContext *h, int codec_id,
    const int bit_depth,
    const int chroma_format_idc)
{
    if (8 == bit_depth) {
        if (chroma_format_idc == 1) {
            h->pred8x8[VERT_PRED8x8] = ff_h264_intra_pred_vert_8x8_websimd128;
            h->pred8x8[HOR_PRED8x8] = ff_h264_intra_pred_horiz_8x8_websimd128;
        }

        if (codec_id != AV_CODEC_ID_VP7 && codec_id != AV_CODEC_ID_VP8) {
            if (chroma_format_idc == 1) {
                h->pred8x8[PLANE_PRED8x8] = ff_h264_intra_predict_plane_8x8_websimd128;
            }
        }
        if (codec_id != AV_CODEC_ID_RV40 && codec_id != AV_CODEC_ID_VP7
            && codec_id != AV_CODEC_ID_VP8) {
            if (chroma_format_idc == 1) {
                h->pred8x8[DC_PRED8x8] = ff_h264_intra_predict_dc_4blk_8x8_websimd128;
                h->pred8x8[LEFT_DC_PRED8x8] =
                    ff_h264_intra_predict_hor_dc_8x8_websimd128;
                h->pred8x8[TOP_DC_PRED8x8] =
                    ff_h264_intra_predict_vert_dc_8x8_websimd128;
                h->pred8x8[ALZHEIMER_DC_L0T_PRED8x8] =
                    ff_h264_intra_predict_mad_cow_dc_l0t_8x8_websimd128;
                h->pred8x8[ALZHEIMER_DC_0LT_PRED8x8] =
                    ff_h264_intra_predict_mad_cow_dc_0lt_8x8_websimd128;
                h->pred8x8[ALZHEIMER_DC_L00_PRED8x8] =
                    ff_h264_intra_predict_mad_cow_dc_l00_8x8_websimd128;
                h->pred8x8[ALZHEIMER_DC_0L0_PRED8x8] =
                    ff_h264_intra_predict_mad_cow_dc_0l0_8x8_websimd128;
            }
        }

        if (chroma_format_idc == 1) {
            h->pred8x8[DC_128_PRED8x8] = ff_h264_intra_pred_dc_128_8x8_websimd128;
        }

        h->pred16x16[DC_PRED8x8] = ff_h264_intra_pred_dc_16x16_websimd128;
        h->pred16x16[VERT_PRED8x8] = ff_h264_intra_pred_vert_16x16_websimd128;
        h->pred16x16[HOR_PRED8x8] = ff_h264_intra_pred_horiz_16x16_websimd128;

        switch (codec_id) {
        case AV_CODEC_ID_SVQ3:
        case AV_CODEC_ID_RV40:
            break;
        case AV_CODEC_ID_VP7:
        case AV_CODEC_ID_VP8:
            break;
        default:
            h->pred16x16[PLANE_PRED8x8] =
                ff_h264_intra_predict_plane_16x16_websimd128;
            break;
        }

        h->pred16x16[LEFT_DC_PRED8x8] = ff_h264_intra_pred_dc_left_16x16_websimd128;
        h->pred16x16[TOP_DC_PRED8x8] = ff_h264_intra_pred_dc_top_16x16_websimd128;
        h->pred16x16[DC_128_PRED8x8] = ff_h264_intra_pred_dc_128_16x16_websimd128;
    }
}
#endif  // #if HAVE_WEBSIMD128

av_cold void ff_h264_pred_init_webassembly(H264PredContext *h, int codec_id,
    int bit_depth,
    const int chroma_format_idc)
{
#if HAVE_WEBSIMD128
    h264_pred_init_websimd128(h, codec_id, bit_depth, chroma_format_idc);
#endif  // #if HAVE_WEBSIMD128
}
