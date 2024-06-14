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

#ifndef AVCODEC_WEBASSEMBLY_H264PRED_H
#define AVCODEC_WEBASSEMBLY_H264PRED_H

#include "libavcodec/h264pred.h"

void ff_h264_intra_pred_vert_8x8_websimd128(uint8_t *_src, ptrdiff_t _stride);

void ff_h264_intra_pred_horiz_8x8_websimd128(uint8_t *_src, ptrdiff_t stride);

void ff_h264_intra_predict_plane_8x8_websimd128(uint8_t *_src, ptrdiff_t _stride);

void ff_h264_intra_predict_dc_4blk_8x8_websimd128(uint8_t *_src, ptrdiff_t stride);

void ff_h264_intra_predict_hor_dc_8x8_websimd128(uint8_t *_src, ptrdiff_t stride);

void ff_h264_intra_predict_vert_dc_8x8_websimd128(uint8_t *_src, ptrdiff_t stride);

void ff_h264_intra_predict_mad_cow_dc_l0t_8x8_websimd128(uint8_t *src, ptrdiff_t stride);

void ff_h264_intra_predict_mad_cow_dc_0lt_8x8_websimd128(uint8_t *src, ptrdiff_t stride);

void ff_h264_intra_predict_mad_cow_dc_l00_8x8_websimd128(uint8_t *src, ptrdiff_t stride);

void ff_h264_intra_predict_mad_cow_dc_0l0_8x8_websimd128(uint8_t *src, ptrdiff_t stride);

void ff_h264_intra_pred_dc_128_8x8_websimd128(uint8_t *_src, ptrdiff_t stride);

void ff_h264_intra_pred_dc_16x16_websimd128(uint8_t *_src, ptrdiff_t stride);

void ff_h264_intra_pred_vert_16x16_websimd128(uint8_t *_src, ptrdiff_t _stride);

void ff_h264_intra_pred_horiz_16x16_websimd128(uint8_t *_src, ptrdiff_t stride);

void ff_h264_intra_predict_plane_16x16_websimd128(uint8_t *src, ptrdiff_t stride);

void ff_h264_intra_pred_dc_left_16x16_websimd128(uint8_t *_src, ptrdiff_t stride);

void ff_h264_intra_pred_dc_top_16x16_websimd128(uint8_t *_src, ptrdiff_t stride);

void ff_h264_intra_pred_dc_128_16x16_websimd128(uint8_t *_src, ptrdiff_t stride);

#endif  /* AVCODEC_WEBASSEMBLY_H264PRED_H */
