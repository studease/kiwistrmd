/*
 * stu_aac.c
 *
 *  Created on: 2018年5月29日
 *      Author: Tony Lau
 */

#include "stu_codec.h"

static stu_int32_t  stu_aac_parse_config_ALS(stu_aac_context_t *ctx, stu_bitstream_t *gb);

stu_int32_t  STU_AAC_SAMPLING_FREQUENCYS[16] = {
	96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000, 7350
};

stu_uint8_t  STU_AAC_CHANNELS[8] = {
	0, 1, 2, 3, 4, 5, 6, 8
};


stu_int32_t
stu_aac_parse_packet(stu_aac_context_t *ctx, stu_uint32_t timestamp, u_char *data, size_t len) {
	u_char      *pos;
	stu_int32_t  rc;

	pos = data;
	rc = STU_OK;

	if (len < 5) {
		stu_log_error(0, "Data not enough while parsing AAC packet.");
		return STU_ERROR;
	}

	ctx->avctx.format = *pos >> 4;
	ctx->avctx.sample_rate = *pos >> 2;
	ctx->avctx.sample_size = *pos >> 1;
	ctx->avctx.sample_type = *pos++;
	ctx->avctx.data_type = *pos++;

	switch (ctx->avctx.data_type) {
	case STU_AAC_TYPE_SPECIFIC_CONFIG:
		rc = stu_aac_parse_specific_config(ctx, pos, len - (pos - data));
		break;

	case STU_AAC_TYPE_RAW_FRAME_DATA:
		rc = stu_aac_parse_data(ctx, timestamp, pos, len - (pos - data));
		break;

	default:
		stu_log_error(0, "Unknown AAC packet type: %d.", ctx->avctx.data_type);
		return STU_ERROR;
	}

	return rc;
}

stu_int32_t
stu_aac_parse_specific_config(stu_aac_context_t *ctx, u_char *data, size_t len) {
	stu_bitstream_t *gb;

	gb = &ctx->gb;

	if (len < 2) {
		stu_log_error(0, "Data not enough while parsing AAC specific config.");
		return STU_ERROR;
	}

	if (stu_bitstream_init(gb, data, len) == STU_ERROR) {
		stu_log_error(0, "Failed to init bitstream.");
		return STU_ERROR;
	}

	ctx->audio_object_type = stu_bitstream_get_bits(gb, 5);
	if (ctx->audio_object_type == STU_AOT_ESCAPE) {
		ctx->audio_object_type = 32 + stu_bitstream_get_bits(gb, 6);
	}

	ctx->sampling_frequency_index = stu_bitstream_get_bits(gb, 4);
	ctx->sampling_frequency = ctx->sampling_frequency_index == 0xF ? stu_bitstream_get_bits(gb, 24) :
			STU_AAC_SAMPLING_FREQUENCYS[ctx->sampling_frequency_index];

	ctx->channel_configuration = stu_bitstream_get_bits(gb, 4);
	if (ctx->channel_configuration < STU_AV_ARRAY_LEN(STU_AAC_CHANNELS)) {
		ctx->channels = STU_AAC_CHANNELS[ctx->channel_configuration];
	}

	if (ctx->audio_object_type == STU_AOT_SBR || (ctx->audio_object_type == STU_AOT_PS &&
			// check for W6132 Annex YYYY draft MP3onMP4
			!((stu_bitstream_show_bits(gb, 3) & 0x03) && !(stu_bitstream_show_bits(gb, 9) & 0x3F)))) {
		ctx->ext_sampling_frequency_index = stu_bitstream_get_bits(gb, 4);
		ctx->ext_sampling_frequency = ctx->ext_sampling_frequency_index == 0xF ? stu_bitstream_get_bits(gb, 24) :
				STU_AAC_SAMPLING_FREQUENCYS[ctx->ext_sampling_frequency_index];

		ctx->ext_audio_object_type = stu_bitstream_get_bits(gb, 5);

		switch (ctx->ext_audio_object_type) {
		case STU_AOT_ESCAPE:
			ctx->ext_audio_object_type = 32 + stu_bitstream_get_bits(gb, 6);
			break;
		case STU_AOT_ER_BSAC:
			ctx->ext_channel_configuration = stu_bitstream_get_bits(gb, 4);
			break;
		}
	} else {
		ctx->ext_audio_object_type = STU_AOT_NULL;
		ctx->ext_sampling_frequency = 0;
	}

	if (ctx->audio_object_type == STU_AOT_ALS) {
		stu_bitstream_skip_bits(gb, 5);
		if (stu_bitstream_show_bits_long(gb, 24) != stu_endian_32(*(stu_uint32_t *) "\0ALS")) {
			stu_bitstream_skip_bits(gb, 24);
		}

		if (stu_aac_parse_config_ALS(ctx, gb) == STU_ERROR) {
			stu_log_error(0, "Failed to parse config ALS.");
			return STU_ERROR;
		}
	}

	ctx->avctx.ref_sample_duration = ctx->avctx.timescale * 1024 / ctx->sampling_frequency;

	// force to STU_AOT_SBR
	ctx->audio_object_type = STU_AOT_SBR;
	ctx->ext_sampling_frequency_index = ctx->sampling_frequency_index;
	if (ctx->sampling_frequency_index >= 6) {
		ctx->ext_sampling_frequency_index -= 3;
	} else if (ctx->channel_configuration == 1) { // Mono channel
		ctx->audio_object_type = STU_AOT_AAC_LC;
	}

	ctx->conf[0] = ctx->audio_object_type << 3 | ctx->sampling_frequency_index >> 1;
	ctx->conf[1] = ctx->sampling_frequency_index << 7 | ctx->channel_configuration << 3;
	ctx->conf_size = 2;

	if (ctx->audio_object_type == STU_AOT_SBR) {
		ctx->conf[1] |= ctx->ext_sampling_frequency_index >> 1;
		ctx->conf[2]  = ctx->ext_sampling_frequency_index << 7;
		ctx->conf[2] |= 2 << 2;
		ctx->conf[3]  = 0;
		ctx->conf_size = 4;
	}

	return STU_OK;
}

static stu_int32_t
stu_aac_parse_config_ALS(stu_aac_context_t *ctx, stu_bitstream_t *gb) {
	if (stu_bitstream_left(gb) < 112) {
		return STU_ERROR;
	}

	if (stu_bitstream_get_bits_long(gb, 32) != stu_endian_32(*(stu_uint32_t *) "ALS\0")) {
		return STU_ERROR;
	}

	// override AudioSpecificConfig channel configuration and sample rate
	// which are buggy in old ALS conformance files
	ctx->sampling_frequency = stu_bitstream_get_bits_long(gb, 32);

	// skip number of samples
	stu_bitstream_skip_bits(gb, 32);

	// read number of channels
	ctx->channel_configuration = 0;
	ctx->channels = stu_bitstream_get_bits(gb, 16) + 1;

	return STU_OK;
}

stu_int32_t
stu_aac_parse_data(stu_aac_context_t *ctx, stu_uint32_t timestamp, u_char *data, size_t len) {
	ctx->avctx.dts = ctx->avctx.timebase + timestamp;
	ctx->avctx.pts = ctx->avctx.dts;

	ctx->avctx.data = data;
	ctx->avctx.data_size = len;

	return STU_OK;
}
