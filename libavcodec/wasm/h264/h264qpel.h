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

#ifndef AVCODEC_WEBASSEMBLY_H264QPEL_H
#define AVCODEC_WEBASSEMBLY_H264QPEL_H

#include "libavcodec/h264qpel.h"

void put_h264_qpel4_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel4_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel8_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void put_h264_qpel16_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel4_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel8_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc00_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc10_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc20_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc30_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc01_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc02_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc03_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc11_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc31_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc13_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc33_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc22_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc21_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc23_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc12_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);
void avg_h264_qpel16_mc32_8_websimd128(uint8_t *dst, const uint8_t *src, ptrdiff_t stride);

#endif  /* AVCODEC_WEBASSEMBLY_H264QPEL_H */
