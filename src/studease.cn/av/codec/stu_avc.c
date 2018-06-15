/*
 * stu_avc.c
 *
 *  Created on: 2018年5月29日
 *      Author: Tony Lau
 */

#include "stu_codec.h"

#define STU_AVC_MAX_LOG2_MAX_FRAME_NUM    (12 + 4)
#define STU_AVC_MIN_LOG2_MAX_FRAME_NUM    4

static const stu_uint8_t  stu_avc_default_scaling4[2][16] = {
	{  6, 13, 20, 28, 13, 20, 28, 32,
	  20, 28, 32, 37, 28, 32, 37, 42 },
	{ 10, 14, 20, 24, 14, 20, 24, 27,
	  20, 24, 27, 30, 24, 27, 30, 34 }
};

static const stu_uint8_t  stu_avc_default_scaling8[2][64] = {
	{  6, 10, 13, 16, 18, 23, 25, 27,
	  10, 11, 16, 18, 23, 25, 27, 29,
	  13, 16, 18, 23, 25, 27, 29, 31,
	  16, 18, 23, 25, 27, 29, 31, 33,
	  18, 23, 25, 27, 29, 31, 33, 36,
	  23, 25, 27, 29, 31, 33, 36, 38,
	  25, 27, 29, 31, 33, 36, 38, 40,
	  27, 29, 31, 33, 36, 38, 40, 42 },
	{  9, 13, 15, 17, 19, 21, 22, 24,
	  13, 13, 17, 19, 21, 22, 24, 25,
	  15, 17, 19, 21, 22, 24, 25, 27,
	  17, 19, 21, 22, 24, 25, 27, 28,
	  19, 21, 22, 24, 25, 27, 28, 30,
	  21, 22, 24, 25, 27, 28, 30, 32,
	  22, 24, 25, 27, 28, 30, 32, 33,
	  24, 25, 27, 28, 30, 32, 33, 35 }
};

static const stu_rational_t  stu_avc_pixel_aspect[17] = {
	{   0,  1 },
	{   1,  1 },
	{  12, 11 },
	{  10, 11 },
	{  16, 11 },
	{  40, 33 },
	{  24, 11 },
	{  20, 11 },
	{  32, 11 },
	{  80, 33 },
	{  18, 11 },
	{  15, 11 },
	{  64, 33 },
	{ 160, 99 },
	{   4,  3 },
	{   3,  2 },
	{   2,  1 },
};

static const stu_uint8_t  stu_avc_dequant4_coeff_init[6][3] = {
	{ 10, 13, 16 },
	{ 11, 14, 18 },
	{ 13, 16, 20 },
	{ 14, 18, 23 },
	{ 16, 20, 25 },
	{ 18, 23, 29 },
};

static const stu_uint8_t stu_avc_dequant8_coeff_init_scan[16] = {
	0, 3, 4, 3, 3, 1, 5, 1, 4, 5, 2, 5, 3, 1, 5, 1
};

static const stu_uint8_t stu_avc_dequant8_coeff_init[6][6] = {
	{ 20, 18, 32, 19, 25, 24 },
	{ 22, 19, 35, 21, 28, 26 },
	{ 26, 23, 42, 24, 33, 31 },
	{ 28, 25, 45, 26, 35, 33 },
	{ 32, 28, 51, 30, 40, 38 },
	{ 36, 32, 58, 34, 46, 43 },
};

static const stu_uint8_t stu_avc_quant_rem6[STU_AVC_MAX_QP_NUM + 1] = {
	0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2,
	3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5,
	0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2,
	3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5, 0, 1, 2, 3, 4, 5,
	0, 1, 2, 3,
};

static const stu_uint8_t stu_avc_quant_div6[STU_AVC_MAX_QP_NUM + 1] = {
	0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3,  3,  3,
	3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6,  6,  6,
	7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 10, 10, 10,
	10,10,10,11,11,11,11,11,11,12,12,12,12,12,12,13,13,13, 13, 13, 13,
	14,14,14,14,
};

#define QP(qP, depth) ((qP) + 6 * ((depth) - 8))

#define CHROMA_QP_TABLE_END(d)                                          \
	QP(0,  d), QP(1,  d), QP(2,  d), QP(3,  d), QP(4,  d), QP(5,  d),   \
	QP(6,  d), QP(7,  d), QP(8,  d), QP(9,  d), QP(10, d), QP(11, d),   \
	QP(12, d), QP(13, d), QP(14, d), QP(15, d), QP(16, d), QP(17, d),   \
	QP(18, d), QP(19, d), QP(20, d), QP(21, d), QP(22, d), QP(23, d),   \
	QP(24, d), QP(25, d), QP(26, d), QP(27, d), QP(28, d), QP(29, d),   \
	QP(29, d), QP(30, d), QP(31, d), QP(32, d), QP(32, d), QP(33, d),   \
	QP(34, d), QP(34, d), QP(35, d), QP(35, d), QP(36, d), QP(36, d),   \
	QP(37, d), QP(37, d), QP(37, d), QP(38, d), QP(38, d), QP(38, d),   \
	QP(39, d), QP(39, d), QP(39, d), QP(39, d)

static const stu_uint8_t  stu_avc_chroma_qp[7][STU_AVC_MAX_QP_NUM + 1] = {
	{ CHROMA_QP_TABLE_END(8) },
	{ 0, 1, 2, 3, 4, 5,
	  CHROMA_QP_TABLE_END(9) },
	{ 0, 1, 2, 3,  4,  5,
	  6, 7, 8, 9, 10, 11,
	  CHROMA_QP_TABLE_END(10) },
	{ 0,  1, 2, 3,  4,  5,
	  6,  7, 8, 9, 10, 11,
	  12,13,14,15, 16, 17,
	  CHROMA_QP_TABLE_END(11) },
	{ 0,  1, 2, 3,  4,  5,
	  6,  7, 8, 9, 10, 11,
	  12,13,14,15, 16, 17,
	  18,19,20,21, 22, 23,
	  CHROMA_QP_TABLE_END(12) },
	{ 0,  1, 2, 3,  4,  5,
	  6,  7, 8, 9, 10, 11,
	  12,13,14,15, 16, 17,
	  18,19,20,21, 22, 23,
	  24,25,26,27, 28, 29,
	  CHROMA_QP_TABLE_END(13) },
	{ 0,  1, 2, 3,  4,  5,
	  6,  7, 8, 9, 10, 11,
	  12,13,14,15, 16, 17,
	  18,19,20,21, 22, 23,
	  24,25,26,27, 28, 29,
	  30,31,32,33, 34, 35,
	  CHROMA_QP_TABLE_END(14) },
};

static stu_int32_t  stu_avc_extract_rbsp(stu_bitstream_t *gb, u_char *src, size_t len);
static void         stu_avc_decode_scaling_matrices(stu_bitstream_t *gb, stu_avc_sps_t *sps, stu_avc_pps_t *pps, stu_bool_t is_sps, stu_uint8_t(*scaling_matrix4)[16], stu_uint8_t(*scaling_matrix8)[64]);
static void         stu_avc_decode_scaling_list(stu_bitstream_t *gb, stu_uint8_t *factors, stu_int32_t size, const stu_uint8_t *jvt_list, const stu_uint8_t *fallback_list);
static stu_int32_t  stu_avc_image_check_size(stu_uint32_t w, stu_uint32_t h);
static stu_int32_t  stu_avc_decode_vui_parameters(stu_bitstream_t *gb, stu_avc_sps_t *sps);
static stu_int32_t  stu_avc_decode_hrd_parameters(stu_bitstream_t *gb, stu_avc_hrd_t *hrd);

static stu_int32_t  stu_avc_more_rbsp_data_in_pps(const stu_avc_sps_t *sps);
static void         stu_avc_build_qp_table(stu_avc_pps_t *pps, stu_int32_t t, stu_int32_t index, const stu_int32_t depth);
static void         stu_avc_init_dequant_tables(stu_avc_pps_t *pps, const stu_avc_sps_t *sps);
static void         stu_avc_init_dequant4_coeff_table(stu_avc_pps_t *pps, const stu_avc_sps_t *sps);
static void         stu_avc_init_dequant8_coeff_table(stu_avc_pps_t *pps, const stu_avc_sps_t *sps);


stu_int32_t
stu_avc_parse_packet(stu_avc_context_t *ctx, stu_uint32_t timestamp, u_char *data, size_t len) {
	u_char      *pos;
	stu_int32_t  rc;

	pos = data;
	rc = STU_OK;

	if (len < 5) {
		stu_log_error(0, "Data not enough while parsing AVC packet.");
		return STU_ERROR;
	}

	ctx->avctx.frame_type = *pos >> 4;
	ctx->avctx.codec = *pos++;
	ctx->avctx.data_type = *pos++;
	ctx->avctx.cts  = *pos++ << 16;
	ctx->avctx.cts |= *pos++ << 8;
	ctx->avctx.cts |= *pos++;

	switch (ctx->avctx.data_type) {
	case STU_AVC_TYPE_SEQUENCE_HEADER:
		rc = stu_avc_parse_decoder_configuration_record(ctx, pos, len - (pos - data));
		break;

	case STU_AVC_TYPE_NALU:
		rc = stu_avc_parse_nal_units(ctx, timestamp, pos, len - (pos - data));
		break;

	case STU_AVC_TYPE_END_OF_SEQUENCE:
		stu_log_debug(0x10, "AVC sequence end.");
		break;

	default:
		stu_log_error(0, "Unknown AVC packet type: %d.", ctx->avctx.data_type);
		return STU_ERROR;
	}

	return rc;
}

stu_int32_t
stu_avc_parse_decoder_configuration_record(stu_avc_context_t *ctx, u_char *data, size_t len) {
	u_char       *pos;
	stu_uint8_t   i, num_of_sps, num_of_pps;
	stu_uint16_t  sps_len, pps_len;

	pos = data;

	if (len < 7) {
		stu_log_error(0, "Data not enough while parsing AVC decoder configuration record.");
		return STU_ERROR;
	}

	ctx->avcc = data;
	ctx->avcc_size = len;

	ctx->configuration_version = *pos++;
	ctx->profile_indication = *pos++;
	ctx->profile_compatibility = *pos++;
	ctx->level_indication = *pos++;

	if (ctx->configuration_version != 1) {
		stu_log_error(0, "Invalid AVC configuration version: %d.", ctx->configuration_version);
		return STU_ERROR;
	}

	ctx->nal_length_size = (*pos++ & 0x03) + 1;
	if (ctx->nal_length_size != 3 && ctx->nal_length_size != 4) {
		stu_log_error(0, "Invalid nal_length_size: %d.", ctx->nal_length_size);
		return STU_ERROR;
	}

	num_of_sps = *pos++ & 0x1F;
	for (i = 0; i < num_of_sps; i++) {
		sps_len = stu_endian_16(*(stu_uint16_t *) pos);
		pos += 2;

		if (sps_len == 0) {
			continue;
		}

		if (stu_avc_parse_sps(ctx, pos, sps_len) == STU_ERROR) {
			stu_log_error(0, "Failed to parse SPS.");
			return STU_ERROR;
		}

		pos += sps_len;
	}

	num_of_pps = *pos++;
	for (i = 0; i < num_of_pps; i++) {
		pps_len = stu_endian_16(*(stu_uint16_t *) pos);
		pos += 2;

		if (pps_len == 0) {
			continue;
		}

		if (stu_avc_parse_pps(ctx, pos, pps_len) == STU_ERROR) {
			stu_log_error(0, "Failed to parse PPS.");
			return STU_ERROR;
		}

		pos += pps_len;
	}

	return STU_OK;
}

stu_int32_t
stu_avc_parse_nal_units(stu_avc_context_t *ctx, stu_uint32_t timestamp, u_char *data, size_t len) {
	stu_uint32_t  nalu_size;

	if (len <= 4) {
		stu_log_error(0, "Data not enough while parsing NALUs.");
		return STU_ERROR;
	}

	ctx->avctx.keyframe = ctx->avctx.frame_type == STU_AVC_KEYFRAME ? TRUE : FALSE;
	ctx->avctx.dts = ctx->avctx.timebase + timestamp;
	ctx->avctx.pts = ctx->avctx.cts + ctx->avctx.dts;

	nalu_size = stu_endian_32(*(stu_uint32_t *) data);
	if (ctx->nal_length_size == 3) {
		nalu_size >>= 8;
	}

	if (nalu_size > len - ctx->nal_length_size) {
		stu_log_error(0, "Malformed Nalus near timestamp %d.", ctx->avctx.dts);
		return STU_ERROR;
	}

	ctx->nal_unit_type = *(data + ctx->nal_length_size) & 0x1F;
	if (ctx->nal_unit_type == STU_AVC_INFO_OR_COMMAND_FRAME) {
		ctx->avctx.keyframe = TRUE;
	}

	ctx->avctx.data = data;
	ctx->avctx.data_size = len;

	return STU_OK;
}

stu_int32_t
stu_avc_parse_sps(stu_avc_context_t *ctx, u_char *data, size_t len) {
	stu_avc_sps_t   *sps;
	stu_bitstream_t *gb;
	stu_int32_t      i, rc;

	sps = &ctx->sps;
	gb = &ctx->gb;
	rc = STU_ERROR;

	if (stu_avc_extract_rbsp(gb, data, len) == STU_ERROR) {
		stu_log_error(0, "Failed to extract rbsp.");
		goto failed;
	}

	stu_bitstream_get_bits(gb, 8);

	sps->profile_idc = stu_bitstream_get_bits(gb, 8);
	sps->constraint_set_flags  = stu_bitstream_get_bits(gb, 1) << 0; // constraint_set0_flag
	sps->constraint_set_flags |= stu_bitstream_get_bits(gb, 1) << 1; // constraint_set1_flag
	sps->constraint_set_flags |= stu_bitstream_get_bits(gb, 1) << 2; // constraint_set2_flag
	sps->constraint_set_flags |= stu_bitstream_get_bits(gb, 1) << 3; // constraint_set3_flag
	sps->constraint_set_flags |= stu_bitstream_get_bits(gb, 1) << 4; // constraint_set4_flag
	sps->constraint_set_flags |= stu_bitstream_get_bits(gb, 1) << 5; // constraint_set5_flag
	sps->reserved_zero_2bits = stu_bitstream_get_bits(gb, 2);
	sps->level_idc = stu_bitstream_get_bits(gb, 8);
	sps->id = stu_golomb_get_ue(gb);

	if (sps->id >= STU_AVC_MAX_SPS_COUNT) {
		stu_log_error(0, "sps id %u out of range.", sps->id);
		goto failed;
	}

	sps->seq_scaling_matrix_present_flag = 0;
	memset(sps->scaling_matrix4, 16, sizeof(sps->scaling_matrix4));
	memset(sps->scaling_matrix8, 16, sizeof(sps->scaling_matrix8));
	sps->vui.video_full_range_flag = 1;
	sps->vui.matrix_coefficients = STU_AV_COL_SPC_UNSPECIFIED;

	if (sps->profile_idc == 100 ||  // High profile
		sps->profile_idc == 110 ||  // High10 profile
		sps->profile_idc == 122 ||  // High422 profile
		sps->profile_idc == 244 ||  // High444 Predictive profile
		sps->profile_idc ==  44 ||  // Cavlc444 profile
		sps->profile_idc ==  83 ||  // Scalable Constrained High profile (SVC)
		sps->profile_idc ==  86 ||  // Scalable High Intra profile (SVC)
		sps->profile_idc == 118 ||  // Stereo High profile (MVC)
		sps->profile_idc == 128 ||  // Multiview High profile (MVC)
		sps->profile_idc == 138 ||  // Multiview Depth High profile (MVCD)
		sps->profile_idc == 144) {  // old High444 profile
		sps->chroma_format_idc = stu_golomb_get_ue(gb);
		if (sps->chroma_format_idc > 3U) {
			stu_log_error(0, "Bad sps->chroma_format_idc %u.", sps->chroma_format_idc);
			goto failed;
		} else if (sps->chroma_format_idc == 3) {
			sps->separate_colour_plane_flag = stu_bitstream_get_bits(gb, 1);
			if (sps->separate_colour_plane_flag) {
				stu_log_error(0, "separate color planes are not supported.");
				goto failed;
			}
		}

		sps->bit_depth_luma   = stu_golomb_get_ue(gb) + 8;
		sps->bit_depth_chroma = stu_golomb_get_ue(gb) + 8;
		if (sps->bit_depth_chroma != sps->bit_depth_luma) {
			stu_log_error(0, "Different chroma and luma bit depth.");
			goto failed;
		}

		if (sps->bit_depth_luma   < 8 || sps->bit_depth_luma   > 14 ||
			sps->bit_depth_chroma < 8 || sps->bit_depth_chroma > 14) {
			stu_log_error(0, "Illegal bit depth value (%d, %d)\n", sps->bit_depth_luma, sps->bit_depth_chroma);
			goto failed;
		}

		sps->transform_bypass = stu_bitstream_get_bits(gb, 1);
		stu_avc_decode_scaling_matrices(gb, sps, NULL, 1, sps->scaling_matrix4, sps->scaling_matrix8);
	} else {
		sps->chroma_format_idc = 1;
		sps->bit_depth_luma    = 8;
		sps->bit_depth_chroma  = 8;
	}

	sps->log2_max_frame_num = stu_golomb_get_ue(gb) + 4;
	if (sps->log2_max_frame_num < STU_AVC_MIN_LOG2_MAX_FRAME_NUM || sps->log2_max_frame_num > STU_AVC_MAX_LOG2_MAX_FRAME_NUM) {
		stu_log_error(0, "log2_max_frame_num_minus4 out of range (0-12): %d.", sps->log2_max_frame_num - 4);
		goto failed;
	}

	sps->poc_type = stu_golomb_get_ue(gb);
	if (sps->poc_type == 0) {
		sps->log2_max_poc_lsb = stu_golomb_get_ue(gb) + 4;
		if (sps->log2_max_poc_lsb > 16) {
			stu_log_error(0, "log2_max_poc_lsb (%d) is out of range.", sps->log2_max_poc_lsb);
			goto failed;
		}
	} else if (sps->poc_type == 1) {
		sps->delta_pic_order_always_zero_flag = stu_bitstream_get_bits(gb, 1);
		sps->offset_for_non_ref_pic = stu_golomb_get_ue(gb);
		sps->offset_for_top_to_bottom_field = stu_golomb_get_ue(gb);
		sps->num_ref_frames_in_poc_cycle = stu_golomb_get_ue(gb);

		if (sps->num_ref_frames_in_poc_cycle >= STU_AV_ARRAY_LEN(sps->offset_for_ref_frame)) {
			stu_log_error(0, "poc_cycle_length overflow %d.", sps->num_ref_frames_in_poc_cycle);
			goto failed;
		}

		for (i = 0; i < sps->num_ref_frames_in_poc_cycle; i++) {
			sps->offset_for_ref_frame[i] = stu_golomb_get_ue(gb);
		}
	} else if (sps->poc_type != 2) {
		stu_log_error(0, "Illegal POC type %d.", sps->poc_type);
		goto failed;
	}

	sps->max_num_ref_frames = stu_golomb_get_ue(gb);
	if (sps->max_num_ref_frames > STU_AVC_MAX_PICTURE_COUNT - 2 || sps->max_num_ref_frames > 16U) {
		stu_log_error(0, "too many reference frames %d.", sps->max_num_ref_frames);
		goto failed;
	}

	sps->gaps_in_frame_num_value_allowed_flag = stu_bitstream_get_bits(gb, 1);
	sps->pic_width = stu_golomb_get_ue(gb) + 1;
	sps->pic_height = stu_golomb_get_ue(gb) + 1;
	if (sps->pic_width  >= INT_MAX / 16 || sps->pic_height >= INT_MAX / 16 ||
			stu_avc_image_check_size(16 * sps->pic_width, 16 * sps->pic_height)) {
		stu_log_error(0, "pic_width or pic_height overflow.");
		goto failed;
	}

	sps->frame_mbs_only_flag = stu_bitstream_get_bits(gb, 1);
	if (!sps->frame_mbs_only_flag) {
		sps->mb_adaptive_frame_field_flag = stu_bitstream_get_bits(gb, 1);
	} else {
		sps->mb_adaptive_frame_field_flag = 0;
	}

	ctx->avctx.coded_width  = 16 * sps->pic_width;
	ctx->avctx.coded_height = 16 * sps->pic_height * (2 - sps->frame_mbs_only_flag);

	sps->direct_8x8_inference_flag = stu_bitstream_get_bits(gb, 1);
	sps->frame_cropping_flag = stu_bitstream_get_bits(gb, 1);
	if (sps->frame_cropping_flag) {
		stu_uint32_t  crop_left   = stu_golomb_get_ue(gb);
		stu_uint32_t  crop_right  = stu_golomb_get_ue(gb);
		stu_uint32_t  crop_top    = stu_golomb_get_ue(gb);
		stu_uint32_t  crop_bottom = stu_golomb_get_ue(gb);

		stu_int32_t   vsub   = (sps->chroma_format_idc == 1) ? 1 : 0;
		stu_int32_t   hsub   = (sps->chroma_format_idc == 1 || sps->chroma_format_idc == 2) ? 1 : 0;
		stu_int32_t   step_x = 1 << hsub;
		stu_int32_t   step_y = (2 - sps->frame_mbs_only_flag) << vsub;

		if (crop_left  > (stu_uint32_t) INT_MAX / 4 / step_x ||
			crop_right  > (stu_uint32_t) INT_MAX / 4 / step_x ||
			crop_top    > (stu_uint32_t) INT_MAX / 4 / step_y ||
			crop_bottom > (stu_uint32_t) INT_MAX / 4 / step_y ||
			(crop_left + crop_right ) * step_x >= ctx->avctx.coded_width ||
			(crop_top  + crop_bottom) * step_y >= ctx->avctx.coded_height
		) {
			stu_log_error(0, "Invalid crop values %d %d %d %d / %d %d.",
					crop_left, crop_right, crop_top, crop_bottom, ctx->avctx.coded_width, ctx->avctx.coded_height);
			goto failed;
		}

		sps->frame_crop_left_offset   = crop_left   * step_x;
		sps->frame_crop_right_offset  = crop_right  * step_x;
		sps->frame_crop_top_offset    = crop_top    * step_y;
		sps->frame_crop_bottom_offset = crop_bottom * step_y;
	} else {
		sps->frame_crop_left_offset   =
		sps->frame_crop_right_offset  =
		sps->frame_crop_top_offset    =
		sps->frame_crop_bottom_offset = 0;
	}

	sps->vui_parameters_present_flag = stu_bitstream_get_bits(gb, 1);
	if (sps->vui_parameters_present_flag) {
		if (stu_avc_decode_vui_parameters(gb, sps) == STU_ERROR) {
			goto failed;
		}

		if (sps->vui.timing_info_present_flag) {
			ctx->avctx.framerate.num = sps->vui.time_scale;
			ctx->avctx.framerate.den = sps->vui.num_units_in_tick * 2;
		}

		ctx->avctx.ref_sample_duration = ctx->avctx.timescale * ctx->avctx.framerate.den / ctx->avctx.framerate.num;
	}

	ctx->avctx.coded_width -= sps->frame_crop_left_offset + sps->frame_crop_right_offset;
	ctx->avctx.coded_height -= sps->frame_crop_top_offset + sps->frame_crop_bottom_offset;

	ctx->avctx.width = ctx->avctx.coded_width * (sps->vui.sar.den ? sps->vui.sar.num / sps->vui.sar.den : 1);
	ctx->avctx.height = ctx->avctx.coded_height;

	rc = STU_OK;

failed:

	stu_bitstream_destroy(gb);

	return rc;
}

static void
stu_avc_decode_scaling_matrices(stu_bitstream_t *gb, stu_avc_sps_t *sps, stu_avc_pps_t *pps, stu_bool_t is_sps, stu_uint8_t(*scaling_matrix4)[16], stu_uint8_t(*scaling_matrix8)[64]) {
	stu_bool_t         fallback_sps = !is_sps && sps->seq_scaling_matrix_present_flag;
	const stu_uint8_t *fallback[4] = {
		fallback_sps ? sps->scaling_matrix4[0] : stu_avc_default_scaling4[0],
		fallback_sps ? sps->scaling_matrix4[3] : stu_avc_default_scaling4[1],
		fallback_sps ? sps->scaling_matrix8[0] : stu_avc_default_scaling8[0],
		fallback_sps ? sps->scaling_matrix8[3] : stu_avc_default_scaling8[1]
	};

	if (stu_bitstream_get_bits(gb, 1)) { // seq_scaling_list_present_flag
		sps->seq_scaling_matrix_present_flag |= is_sps;

		stu_avc_decode_scaling_list(gb, scaling_matrix4[0], 16, stu_avc_default_scaling4[0], fallback[0]);        // Intra, Y
		stu_avc_decode_scaling_list(gb, scaling_matrix4[1], 16, stu_avc_default_scaling4[0], scaling_matrix4[0]); // Intra, Cr
		stu_avc_decode_scaling_list(gb, scaling_matrix4[2], 16, stu_avc_default_scaling4[0], scaling_matrix4[1]); // Intra, Cb
		stu_avc_decode_scaling_list(gb, scaling_matrix4[3], 16, stu_avc_default_scaling4[1], fallback[1]);        // Inter, Y
		stu_avc_decode_scaling_list(gb, scaling_matrix4[4], 16, stu_avc_default_scaling4[1], scaling_matrix4[3]); // Inter, Cr
		stu_avc_decode_scaling_list(gb, scaling_matrix4[5], 16, stu_avc_default_scaling4[1], scaling_matrix4[4]); // Inter, Cb

		if (is_sps || pps->transform_8x8_mode_flag) {
			stu_avc_decode_scaling_list(gb, scaling_matrix8[0], 64, stu_avc_default_scaling8[0], fallback[2]); // Intra, Y
			stu_avc_decode_scaling_list(gb, scaling_matrix8[3], 64, stu_avc_default_scaling8[1], fallback[3]); // Inter, Y
			if (sps->chroma_format_idc == 3) {
				stu_avc_decode_scaling_list(gb, scaling_matrix8[1], 64, stu_avc_default_scaling8[0], scaling_matrix8[0]); // Intra, Cr
				stu_avc_decode_scaling_list(gb, scaling_matrix8[4], 64, stu_avc_default_scaling8[1], scaling_matrix8[3]); // Inter, Cr
				stu_avc_decode_scaling_list(gb, scaling_matrix8[2], 64, stu_avc_default_scaling8[0], scaling_matrix8[1]); // Intra, Cb
				stu_avc_decode_scaling_list(gb, scaling_matrix8[5], 64, stu_avc_default_scaling8[1], scaling_matrix8[4]); // Inter, Cb
			}
		}
	}
}

static void
stu_avc_decode_scaling_list(stu_bitstream_t *gb, stu_uint8_t *factors, stu_int32_t size, const stu_uint8_t *jvt_list, const stu_uint8_t *fallback_list) {
	const stu_uint8_t *scan;
	stu_int32_t        i, last, next;

	last = 8;
	next = 8;
	scan = size == 16 ? stu_av_zigzag_scan : stu_av_zigzag_direct;

	if (!stu_bitstream_get_bits(gb, 1)) { /* matrix not written, we use the predicted one */
		memcpy(factors, fallback_list, size * sizeof(stu_uint8_t));
	} else {
		for (i = 0; i < size; i++) {
			if (next) {
				next = (last + stu_golomb_get_se(gb)) & 0xff;
			}

			if (!i && !next) { /* matrix not written, we use the preset one */
				memcpy(factors, jvt_list, size * sizeof(stu_uint8_t));
				break;
			}

			last = factors[scan[i]] = next ? next : last;
		}
	}
}

static stu_int32_t
stu_avc_image_check_size(stu_uint32_t w, stu_uint32_t h) {
	if (w && h && (w + 128) * (h + 128) < INT_MAX / 8) {
		return STU_OK;
	}

	stu_log_error(0, "Invalid picture size %ux%u.", w, h);

	return STU_ERROR;
}

static stu_int32_t
stu_avc_decode_vui_parameters(stu_bitstream_t *gb, stu_avc_sps_t *sps) {
	stu_avc_vui_t *vui;

	vui = &sps->vui;

	vui->aspect_ratio_info_present_flag = stu_bitstream_get_bits(gb, 1);

	if (vui->aspect_ratio_info_present_flag) {
		vui->aspect_ratio_idc = stu_bitstream_get_bits(gb, 8);
		if (vui->aspect_ratio_idc == STU_AVC_EXTENDED_SAR) {
			vui->sar.num = stu_bitstream_get_bits(gb, 16);
			vui->sar.den = stu_bitstream_get_bits(gb, 16);
		} else if (vui->aspect_ratio_idc < STU_AV_ARRAY_LEN(stu_avc_pixel_aspect)) {
			vui->sar = stu_avc_pixel_aspect[vui->aspect_ratio_idc];
		} else {
			stu_log_error(0, "Illegal aspect ratio.");
			return STU_ERROR;
		}
	} else {
		vui->sar.num = vui->sar.den = 0;
	}

	vui->overscan_info_present_flag = stu_bitstream_get_bits(gb, 1);
	if (vui->overscan_info_present_flag) {
		vui->overscan_appropriate_flag = stu_bitstream_get_bits(gb, 1);
	}

	vui->video_signal_type_present_flag = stu_bitstream_get_bits(gb, 1);
	if (vui->video_signal_type_present_flag) {
		vui->video_format = stu_bitstream_get_bits(gb, 3);
		vui->video_full_range_flag = stu_bitstream_get_bits(gb, 1);
		vui->colour_description_present_flag = stu_bitstream_get_bits(gb, 1);

		if (vui->colour_description_present_flag) {
			vui->colour_primaries = stu_bitstream_get_bits(gb, 8);
			vui->transfer_characteristics = stu_bitstream_get_bits(gb, 8);
			vui->matrix_coefficients = stu_bitstream_get_bits(gb, 8);

			if (vui->colour_primaries >= STU_AV_COL_PRI_NB) {
				vui->colour_primaries = STU_AV_COL_PRI_UNSPECIFIED;
			}
			if (vui->transfer_characteristics >= STU_AV_COL_TRC_NB) {
				vui->transfer_characteristics = STU_AV_COL_TRC_UNSPECIFIED;
			}
			if (vui->matrix_coefficients >= STU_AV_COL_SPC_NB) {
				vui->matrix_coefficients = STU_AV_COL_SPC_UNSPECIFIED;
			}
		}
	}

	vui->chroma_loc_info_present_flag = stu_bitstream_get_bits(gb, 1);
	if (vui->chroma_loc_info_present_flag) {
		vui->chroma_sample_loc_type_top_field = stu_golomb_get_ue(gb);
		vui->chroma_sample_loc_type_bottom_field = stu_golomb_get_ue(gb);
	}

	if (stu_bitstream_get_bits(gb, 1) && stu_bitstream_left(gb) < 10) {
		stu_log_debug(0x10, "Truncated VUI.");
		return STU_OK;
	}

	vui->timing_info_present_flag = stu_bitstream_get_bits(gb, 1);
	if (vui->timing_info_present_flag) {
		vui->num_units_in_tick = stu_bitstream_get_bits_long(gb, 32);
		vui->time_scale        = stu_bitstream_get_bits_long(gb, 32);

		if (!vui->num_units_in_tick || !vui->time_scale) {
			stu_log_error(0, "time_scale/num_units_in_tick invalid or unsupported (%u/%u).",
					vui->time_scale, vui->num_units_in_tick);
			vui->timing_info_present_flag = 0;
		}

		vui->fixed_frame_rate_flag = stu_bitstream_get_bits(gb, 1);
	}

	vui->nal_hrd_parameters_present_flag = stu_bitstream_get_bits(gb, 1);
	if (vui->nal_hrd_parameters_present_flag) {
		if (stu_avc_decode_hrd_parameters(gb, &vui->nal_hrd) == STU_ERROR) {
			return STU_ERROR;
		}
	}

	vui->vcl_hrd_parameters_present_flag = stu_bitstream_get_bits(gb, 1);
	if (vui->vcl_hrd_parameters_present_flag) {
		if (stu_avc_decode_hrd_parameters(gb, &vui->vcl_hrd) == STU_ERROR) {
			return STU_ERROR;
		}
	}

	if (vui->nal_hrd_parameters_present_flag || vui->vcl_hrd_parameters_present_flag) {
		vui->low_delay_hrd_flag = stu_bitstream_get_bits(gb, 1);
	}

	vui->pic_struct_present_flag = stu_bitstream_get_bits(gb, 1);
	if (!stu_bitstream_left(gb)) {
		return STU_OK;
	}

	vui->bitstream_restriction_flag = stu_bitstream_get_bits(gb, 1);
	if (vui->bitstream_restriction_flag) {
		vui->motion_vectors_over_pic_boundaries_flag = stu_bitstream_get_bits(gb, 1);
		vui->max_bytes_per_pic_denom = stu_golomb_get_ue(gb);
		vui->max_bits_per_mb_denom = stu_golomb_get_ue(gb);
		vui->log2_max_mv_length_horizontal = stu_golomb_get_ue(gb);
		vui->log2_max_mv_length_vertical = stu_golomb_get_ue(gb);
		vui->max_num_reorder_frames = stu_golomb_get_ue(gb);
		vui->max_dec_frame_buffering = stu_golomb_get_ue(gb);

		if (stu_bitstream_left(gb) < 0) {
			vui->max_num_reorder_frames = 0;
			vui->bitstream_restriction_flag = 0;
		}

		if (vui->max_num_reorder_frames > 16U
			/* max_dec_frame_buffering || max_dec_frame_buffering > 16 */) {
			stu_log_error(0, "Clipping illegal max_num_reorder_frames %d.", vui->max_num_reorder_frames);
			vui->max_num_reorder_frames = 16;
			return STU_ERROR;
		}
	}

	return 0;
}

static stu_int32_t
stu_avc_decode_hrd_parameters(stu_bitstream_t *gb, stu_avc_hrd_t *hrd) {
	stu_int32_t  i;

	hrd->cpb_cnt = stu_golomb_get_ue(gb) + 1;
	if (hrd->cpb_cnt > 32U) {
		stu_log_error(0, "Invalid hrd->cpb_cnt %d.", hrd->cpb_cnt);
		return STU_ERROR;
	}

	hrd->bit_rate_scale = stu_bitstream_get_bits(gb, 4);
	hrd->cpb_size_scale = stu_bitstream_get_bits(gb, 4);

	for (i = 0; i < hrd->cpb_cnt; i++) {
		hrd->bit_rate_value[i] = stu_golomb_get_ue(gb);
		hrd->cpb_size_value[i] = stu_golomb_get_ue(gb);
		hrd->cbr_flag |= stu_bitstream_get_bits(gb, 1) << i;
	}

	hrd->initial_cpb_removal_delay_length = stu_bitstream_get_bits(gb, 5) + 1;
	hrd->cpb_removal_delay_length         = stu_bitstream_get_bits(gb, 5) + 1;
	hrd->dpb_output_delay_length          = stu_bitstream_get_bits(gb, 5) + 1;
	hrd->time_offset_length               = stu_bitstream_get_bits(gb, 5);

	return STU_OK;
}


stu_int32_t
stu_avc_parse_pps(stu_avc_context_t *ctx, u_char *data, size_t len) {
	stu_avc_sps_t   *sps;
	stu_avc_pps_t   *pps;
	stu_bitstream_t *gb;
	stu_int32_t      qp_bd_offset, rc;

	sps = &ctx->sps;
	pps = &ctx->pps;
	gb = &ctx->gb;
	rc = STU_ERROR;

	if (stu_avc_extract_rbsp(gb, data, len) == STU_ERROR) {
		stu_log_error(0, "Failed to extract rbsp.");
		goto failed;
	}

	pps->id = stu_golomb_get_ue(gb);
	if (pps->id >= STU_AVC_MAX_PPS_COUNT) {
		stu_log_error(0, "pps id %u out of range.", pps->id);
		goto failed;
	}

	pps->sps_id = stu_golomb_get_ue(gb);
	if (pps->sps_id >= STU_AVC_MAX_SPS_COUNT) {
		stu_log_error(0, "sps id %u out of range.", pps->sps_id);
		goto failed;
	}

	if (sps->bit_depth_luma > 14) {
		stu_log_error(0, "Invalid bit_depth_luma %d.", sps->bit_depth_luma);
		goto failed;
	} else if (sps->bit_depth_luma == 11 || sps->bit_depth_luma == 13) {
		stu_log_error(0, "Unimplemented bit_depth_luma %d.", sps->bit_depth_luma);
		goto failed;
	}

	pps->entropy_coding_mode_flag = stu_bitstream_get_bits(gb, 1);
	pps->pic_order_present_flag = stu_bitstream_get_bits(gb, 1);
	pps->num_slice_groups = stu_golomb_get_ue(gb) + 1;
	if (pps->num_slice_groups > 1) {
		pps->slice_group_map_type = stu_golomb_get_ue(gb);
		stu_log_error(0, "FMO not supported.");

		switch (pps->slice_group_map_type) {
		case 0:
#if 0
    |       for (i = 0; i <= num_slice_groups_minus1; i++)  |   |      |
    |           run_length[i]                               |1  |ue(v) |
#endif
			break;
		case 2:
#if 0
    |       for (i = 0; i < num_slice_groups_minus1; i++) { |   |      |
    |           top_left_mb[i]                              |1  |ue(v) |
    |           bottom_right_mb[i]                          |1  |ue(v) |
    |       }                                               |   |      |
#endif
			break;
		case 3:
		case 4:
		case 5:
#if 0
    |       slice_group_change_direction_flag               |1  |u(1)  |
    |       slice_group_change_rate_minus1                  |1  |ue(v) |
#endif
			break;
		case 6:
#if 0
    |       slice_group_id_cnt_minus1                       |1  |ue(v) |
    |       for (i = 0; i <= slice_group_id_cnt_minus1; i++)|   |      |
    |           slice_group_id[i]                           |1  |u(v)  |
#endif
			break;
		}
	}

	pps->num_ref_idx[0] = stu_golomb_get_ue(gb) + 1;
	pps->num_ref_idx[1] = stu_golomb_get_ue(gb) + 1;
	if (pps->num_ref_idx[0] - 1 > 32 - 1 || pps->num_ref_idx[1] - 1 > 32 - 1) {
		stu_log_error(0, "reference overflow (pps).");
		goto failed;
	}

	qp_bd_offset = 6 * (sps->bit_depth_luma - 8);

	pps->weighted_pred_flag                     = stu_bitstream_get_bits(gb, 1);
	pps->weighted_bipred_idc                    = stu_bitstream_get_bits(gb, 2);
	pps->pic_init_qp                            = stu_golomb_get_se(gb) + 26 + qp_bd_offset;
	pps->pic_init_qs                            = stu_golomb_get_se(gb) + 26 + qp_bd_offset;
	pps->chroma_qp_index_offset[0]              = stu_golomb_get_se(gb);
	pps->deblocking_filter_control_present_flag = stu_bitstream_get_bits(gb, 1);
	pps->constrained_intra_pred_flag            = stu_bitstream_get_bits(gb, 1);
	pps->redundant_pic_cnt_present_flag         = stu_bitstream_get_bits(gb, 1);

	pps->transform_8x8_mode_flag = 0;
	memcpy(pps->scaling_matrix4, sps->scaling_matrix4, sizeof(pps->scaling_matrix4));
	memcpy(pps->scaling_matrix8, sps->scaling_matrix8, sizeof(pps->scaling_matrix8));

	if (stu_bitstream_left(gb) > 0 && stu_avc_more_rbsp_data_in_pps(sps)) {
		pps->transform_8x8_mode_flag = stu_bitstream_get_bits(gb, 1);

		//stu_avc_decode_scaling_matrices(gb, sps, pps, 0, pps->scaling_matrix4, pps->scaling_matrix8);
		pps->pic_scaling_matrix_present_flag = stu_bitstream_get_bits(gb, 1);
		if (pps->pic_scaling_matrix_present_flag) {
			stu_bitstream_skip_bits(gb, 6 + ( (sps->chroma_format_idc != 3 ) ? 2 : 6 ) * pps->transform_8x8_mode_flag);
		}

		pps->chroma_qp_index_offset[1] = stu_golomb_get_se(gb); // second_chroma_qp_index_offset
	} else {
		pps->chroma_qp_index_offset[1] = pps->chroma_qp_index_offset[0];
	}

	stu_avc_build_qp_table(pps, 0, pps->chroma_qp_index_offset[0], sps->bit_depth_luma);
	stu_avc_build_qp_table(pps, 1, pps->chroma_qp_index_offset[1], sps->bit_depth_luma);

	stu_avc_init_dequant_tables(pps, sps);

	if (pps->chroma_qp_index_offset[0] != pps->chroma_qp_index_offset[1]) {
		pps->chroma_qp_diff = 1;
	}

	rc = STU_OK;

failed:

	stu_bitstream_destroy(gb);

	return rc;
}

static stu_int32_t
stu_avc_more_rbsp_data_in_pps(const stu_avc_sps_t *sps) {
	if ((sps->profile_idc == 66 || sps->profile_idc == 77 || sps->profile_idc == 88) && (sps->constraint_set_flags & 7)) {
		stu_log_error(0, "Current profile doesn't provide more RBSP data in PPS, skipping.");
		return 0;
	}

	return 1;
}

static void
stu_avc_build_qp_table(stu_avc_pps_t *pps, stu_int32_t t, stu_int32_t index, const stu_int32_t depth) {
	stu_int32_t        i;
	const stu_int32_t  max_qp = 51 + 6 * (depth - 8);

	for (i = 0; i < max_qp + 1; i++) {
		pps->chroma_qp_table[t][i] = stu_avc_chroma_qp[depth - 8][stu_av_clip(i + index, 0, max_qp)];
	}
}

static void
stu_avc_init_dequant_tables(stu_avc_pps_t *pps, const stu_avc_sps_t *sps) {
	stu_int32_t  i, x;

	stu_avc_init_dequant4_coeff_table(pps, sps);
	memset(pps->dequant8_coeff, 0, sizeof(pps->dequant8_coeff));

	if (pps->transform_8x8_mode_flag) {
		stu_avc_init_dequant8_coeff_table(pps, sps);
	}

	if (sps->transform_bypass) {
		for (i = 0; i < 6; i++) {
			for (x = 0; x < 16; x++) {
				pps->dequant4_coeff[i][0][x] = 1 << 6;
			}
		}

		if (pps->transform_8x8_mode_flag) {
			for (i = 0; i < 6; i++) {
				for (x = 0; x < 64; x++) {
					pps->dequant8_coeff[i][0][x] = 1 << 6;
				}
			}
		}
	}
}

static void
stu_avc_init_dequant4_coeff_table(stu_avc_pps_t *pps, const stu_avc_sps_t *sps) {
	stu_int32_t        i, j, q, x;
	const stu_int32_t  max_qp = 51 + 6 * (sps->bit_depth_luma - 8);

	for (i = 0; i < 6; i++) {
		pps->dequant4_coeff[i] = pps->dequant4_buffer[i];

		for (j = 0; j < i; j++) {
			if (!memcmp(pps->scaling_matrix4[j], pps->scaling_matrix4[i], 16 * sizeof(uint8_t))) {
				pps->dequant4_coeff[i] = pps->dequant4_buffer[j];
				break;
			}
		}

		if (j < i) {
			continue;
		}

		for (q = 0; q < max_qp + 1; q++) {
			stu_int32_t  shift = stu_avc_quant_div6[q] + 2;
			stu_int32_t  idx   = stu_avc_quant_rem6[q];

			for (x = 0; x < 16; x++) {
				pps->dequant4_coeff[i][q][(x >> 2) | ((x << 2) & 0xF)] =
						((uint32_t)stu_avc_dequant4_coeff_init[idx][(x & 1) + ((x >> 2) & 1)] *
						pps->scaling_matrix4[i][x]) << shift;
			}
		}
	}
}

static void
stu_avc_init_dequant8_coeff_table(stu_avc_pps_t *pps, const stu_avc_sps_t *sps) {
	stu_int32_t        i, j, q, x;
	const stu_int32_t  max_qp = 51 + 6 * (sps->bit_depth_luma - 8);

	for (i = 0; i < 6; i++) {
		pps->dequant8_coeff[i] = pps->dequant8_buffer[i];

		for (j = 0; j < i; j++) {
			if (!memcmp(pps->scaling_matrix8[j], pps->scaling_matrix8[i], 64 * sizeof(uint8_t))) {
				pps->dequant8_coeff[i] = pps->dequant8_buffer[j];
				break;
			}
		}

		if (j < i) {
			continue;
		}

		for (q = 0; q < max_qp + 1; q++) {
			stu_int32_t  shift = stu_avc_quant_div6[q];
			stu_int32_t  idx   = stu_avc_quant_rem6[q];

			for (x = 0; x < 64; x++) {
				pps->dequant8_coeff[i][q][(x >> 3) | ((x & 7) << 3)] =
						((uint32_t)stu_avc_dequant8_coeff_init[idx][stu_avc_dequant8_coeff_init_scan[((x >> 1) & 12) | (x & 3)]] *
						pps->scaling_matrix8[i][x]) << shift;
			}
		}
	}
}


static stu_int32_t
stu_avc_extract_rbsp(stu_bitstream_t *gb, u_char *src, size_t len) {
	u_char      *dst;
	stu_int32_t  i, j;

	if (gb->start) {
		stu_free((u_char *) gb->start);
	}

	dst = stu_calloc(len);
	if (dst == NULL) {
		stu_log_error(stu_errno, "Failed to calloc rbsp.");
		return STU_ERROR;
	}

	for (i = 0, j = 0; i < len; i++) {
		if (i >= 2 && src[i - 2] == 0 && src[i - 1] == 0 && src[i] == 3) {
			continue;
		}

		dst[j++] = src[i];
	}

	return stu_bitstream_init(gb, dst, j);
}
