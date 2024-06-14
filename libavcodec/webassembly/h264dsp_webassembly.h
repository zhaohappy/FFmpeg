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

#ifndef AVCODEC_WEBASSEMBLY_H264DSP_H
#define AVCODEC_WEBASSEMBLY_H264DSP_H

#include "libavcodec/h264dec.h"

void ff_h264_v_loop_filter_luma_8_websimd128(uint8_t *pix /*align 16*/, ptrdiff_t stride,
                                    int alpha, int beta, int8_t *tc0);
void ff_h264_h_loop_filter_luma_8_websimd128(uint8_t *pix /*align 4 */, ptrdiff_t stride,
                                int alpha, int beta, int8_t *tc0);
void ff_h264_h_loop_filter_luma_mbaff_8_websimd128(uint8_t *pix /*align 16*/, ptrdiff_t stride,
                                        int alpha, int beta, int8_t *tc0);
/* v/h_loop_filter_luma_intra: align 16 */
void ff_h264_v_loop_filter_luma_intra_8_websimd128(uint8_t *pix, ptrdiff_t stride,
                                        int alpha, int beta);
void ff_h264_h_loop_filter_luma_intra_8_websimd128(uint8_t *pix, ptrdiff_t stride,
                                        int alpha, int beta);
void ff_h264_h_loop_filter_luma_mbaff_intra_8_websimd128(uint8_t *pix /*align 16*/,
                                        ptrdiff_t stride, int alpha, int beta);
void ff_h264_v_loop_filter_chroma_8_websimd128(uint8_t *pix /*align 8*/, ptrdiff_t stride,
                                int alpha, int beta, int8_t *tc0);
void ff_h264_h_loop_filter_chroma_8_websimd128(uint8_t *pix /*align 4*/, ptrdiff_t stride,
                                int alpha, int beta, int8_t *tc0);
void ff_h264_h_loop_filter_chroma422_8_websimd128(uint8_t *pix /*align 4*/, ptrdiff_t stride,
                                int alpha, int beta, int8_t *tc0);
void ff_h264_h_loop_filter_chroma_mbaff_8_websimd128(uint8_t *pix /*align 8*/,
                                        ptrdiff_t stride, int alpha, int beta,
                                        int8_t *tc0);
void ff_h264_h_loop_filter_chroma422_mbaff_8_websimd128(uint8_t *pix /*align 8*/,
                                        ptrdiff_t stride, int alpha, int beta,
                                        int8_t *tc0);
void ff_h264_v_loop_filter_chroma_intra_8_websimd128(uint8_t *pix /*align 8*/,
                                        ptrdiff_t stride, int alpha, int beta);
void ff_h264_h_loop_filter_chroma_intra_8_websimd128(uint8_t *pix /*align 8*/,
                                        ptrdiff_t stride, int alpha, int beta);
void ff_h264_h_loop_filter_chroma_mbaff_intra_8_websimd128(uint8_t *pix /*align 8*/,
                                                ptrdiff_t stride, int alpha, int beta);
void ff_h264_h_loop_filter_chroma422_mbaff_intra_8_websimd128(uint8_t *pix /*align 8*/, ptrdiff_t stride, int alpha, int beta);
void ff_h264_h_loop_filter_chroma422_intra_8_websimd128(uint8_t *pix /*align 8*/, ptrdiff_t stride, int alpha, int beta);
// h264_loop_filter_strength: simd only. the C version is inlined in h264_loopfilter.c
void ff_h264_loop_filter_strength_8_websimd128(int16_t bS[2][4][4], uint8_t nnz[40],
                                int8_t ref[2][40], int16_t mv[2][40][2],
                                int bidir, int edges, int step,
                                int mask_mv0, int mask_mv1, int field);

/* IDCT */
void ff_h264_idct_add_8_websimd128(uint8_t *dst /*align 4*/,
                        int16_t *block /*align 16*/, int stride);
void ff_h264_idct8_add_8_websimd128(uint8_t *dst /*align 8*/,
                        int16_t *block /*align 16*/, int stride);
void ff_h264_idct_dc_add_8_websimd128(uint8_t *dst /*align 4*/,
                        int16_t *block /*align 16*/, int stride);
void ff_h264_idct8_dc_add_8_websimd128(uint8_t *dst /*align 8*/,
                        int16_t *block /*align 16*/, int stride);

void ff_h264_idct_add16_8_websimd128(uint8_t *dst /*align 16*/, const int *block_offset,
                        int16_t *block /*align 16*/, int stride,
                        const uint8_t nnzc[15 * 8]);
void ff_h264_idct8_add4_8_websimd128(uint8_t *dst /*align 16*/, const int *block_offset,
                        int16_t *block /*align 16*/, int stride,
                        const uint8_t nnzc[15 * 8]);
void ff_h264_idct_add8_8_websimd128(uint8_t **dst /*align 16*/, const int *block_offset,
                        int16_t *block /*align 16*/, int stride,
                        const uint8_t nnzc[15 * 8]);
void ff_h264_idct_add16intra_8_websimd128(uint8_t *dst /*align 16*/, const int *block_offset,
                                int16_t *block /*align 16*/,
                                int stride, const uint8_t nnzc[15 * 8]);

void ff_h264_idct_add8_422_8_websimd128(uint8_t **dest, const int *block_offset, int16_t *block, int stride, const uint8_t nnzc[15*8]);

void ff_h264_luma_dc_dequant_idct_8_websimd128(int16_t *output,
                                int16_t *input /*align 16*/, int qmul);
void ff_h264_chroma_dc_dequant_idct_8_websimd128(int16_t *block, int qmul);
void ff_h264_chroma422_dc_dequant_idct_8_websimd128(int16_t *block, int qmul);

void ff_h264_weight_pixels16_8_websimd128(uint8_t *block, ptrdiff_t stride, int height, int log2_denom, int weight, int offset);
void ff_h264_biweight_pixels16_8_websimd128(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int height, int log2_denom, int weightd, int weights, int offset);
void ff_h264_weight_pixels8_8_websimd128(uint8_t *block, ptrdiff_t stride, int height, int log2_denom, int weight, int offset);
void ff_h264_biweight_pixels8_8_websimd128(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int height, int log2_denom, int weightd, int weights, int offset);
void ff_h264_weight_pixels4_8_websimd128(uint8_t *_block, ptrdiff_t stride, int height, int log2_denom, int weight, int offset);
void ff_h264_biweight_pixels4_8_websimd128(uint8_t *dst, uint8_t *src, ptrdiff_t stride, int height, int log2_denom, int weightd, int weights, int offset);

/* bypass-transform */
void ff_h264_add_pixels8_clear_8_websimd128(uint8_t *dst, int16_t *block, int stride);
void ff_h264_add_pixels4_clear_8_websimd128(uint8_t *dst, int16_t *block, int stride);

/**
 * Search buf from the start for up to size bytes. Return the index
 * of a zero byte, or >= size if not found. Ideally, use lookahead
 * to filter out any zero bytes that are known to not be followed by
 * one or more further zero bytes and a one byte. Better still, filter
 * out any bytes that form the trailing_zero_8bits syntax element too.
 */
int ff_startcode_find_candidate_8_websimd128(const uint8_t *buf, int size);

#endif  // #ifndef AVCODEC_WEBASSEMBLY_H264DSP_H
