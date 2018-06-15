/*
 * stu_codec.h
 *
 *  Created on: 2018��4��9��
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_CODEC_STU_CODEC_H_
#define STUDEASE_CN_CODEC_STU_CODEC_H_

#include "../stu_av.h"

typedef stu_int32_t (*stu_av_parse_pt)(void *ctx, stu_uint32_t timestamp, u_char *data, size_t len);

typedef struct {
	stu_av_type_e      type;
	stu_av_codec_id_e  codec_id;
	stu_av_parse_pt    parse_pt;

	union {
		struct {
			unsigned     frame_type:4;    // 0xF0
			unsigned     codec:4;         // 0x0F
		};
		struct {
			unsigned     format:4;        // 1111 0000
			unsigned     sample_rate:2;   // 0000 1100
			unsigned     sample_size:1;   // 0000 0010
			unsigned     sample_type:1;   // 0000 0001
		};
	};
	stu_uint8_t        data_type;
	stu_int32_t        cts;             // 3 bytes, big endian, composition time

	stu_int32_t        bit_rate;
	stu_int32_t        timebase;
	stu_rational_t     framerate;
	stu_uint32_t       timescale;
	stu_uint32_t       ref_sample_duration;
	stu_bool_t         keyframe;
	stu_int32_t        width, height;
	stu_int32_t        coded_width, coded_height;
	stu_int32_t        sampling_frequency;
	struct {
		unsigned        is_leading:2;
		unsigned        sample_depends_on:2;
		unsigned        sample_is_depended_on:2;
		unsigned        sample_has_redundancy:2;
		unsigned        is_non_sync:4;
	} flags;

	stu_uint32_t       dts;
	stu_uint32_t       pts;
	stu_uint32_t       expected_dts;
	stu_uint32_t       duration;

	u_char            *data;
	size_t             data_size;
} stu_av_codec_context_t;

typedef struct {
	stu_av_type_e      type;
	stu_av_codec_id_e  id;
	void              *ctx;
} stu_av_codec_t;

#include "stu_aac.h"
#include "stu_avc.h"

#endif /* STUDEASE_CN_CODEC_STU_CODEC_H_ */
