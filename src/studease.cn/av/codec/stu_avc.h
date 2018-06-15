/*
 * stu_avc.h
 *
 *  Created on: 2018��4��9��
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_CODEC_STU_AVC_H_
#define STUDEASE_CN_CODEC_STU_AVC_H_

#include "stu_codec.h"

#define STU_AVC_MAX_PICTURE_COUNT      36
#define STU_AVC_MAX_SPS_COUNT          32
#define STU_AVC_MAX_PPS_COUNT          256
#define STU_AVC_MAX_SLICES             32
#define STU_AVC_MAX_QP_NUM            (51 + 6*6)

#define STU_AVC_EXTENDED_SAR           255

#define STU_AVC_TYPE_SEQUENCE_HEADER   0x00
#define STU_AVC_TYPE_NALU              0x01
#define STU_AVC_TYPE_END_OF_SEQUENCE   0x02

#define STU_AVC_KEYFRAME               0x1
#define STU_AVC_INTER_FRAME            0x2
#define STU_AVC_DISPOSABLE_INTER_FRAME 0x3
#define STU_AVC_GENERATED_KEYFRAME     0x4
#define STU_AVC_INFO_OR_COMMAND_FRAME  0x5

/* NAL unit types */
enum {
	NAL_SLICE           = 1,
	NAL_DPA             = 2,
	NAL_DPB             = 3,
	NAL_DPC             = 4,
	NAL_IDR_SLICE       = 5,
	NAL_SEI             = 6,
	NAL_SPS             = 7,
	NAL_PPS             = 8,
	NAL_AUD             = 9,
	NAL_END_SEQUENCE    = 10,
	NAL_END_STREAM      = 11,
	NAL_FILLER_DATA     = 12,
	NAL_SPS_EXT         = 13,
	NAL_AUXILIARY_SLICE = 19,
	NAL_FF_IGNORE       = 0xFF0F001,
};

typedef struct {
	stu_uint32_t            cpb_cnt;                          // cpb_cnt_minus1 + 1, see H.264 E.1.2
	unsigned                bit_rate_scale:4;
	unsigned                cpb_size_scale:4;
	stu_uint32_t            bit_rate_value[32];               // bit_rate_value_minus1 + 1
	stu_uint32_t            cpb_size_value[32];               // cpb_size_value_minus1 + 1
	stu_uint32_t            cbr_flag;
	stu_uint32_t            initial_cpb_removal_delay_length; // initial_cpb_removal_delay_length_minus1 + 1
	stu_uint32_t            cpb_removal_delay_length;         // cpb_removal_delay_length_minus1 + 1
	stu_uint32_t            dpb_output_delay_length;          // dpb_output_delay_length_minus1 + 1
	stu_uint32_t            time_offset_length;
} stu_avc_hrd_t;

typedef struct {
	unsigned                aspect_ratio_info_present_flag:1;
	stu_uint8_t             aspect_ratio_idc;
	stu_rational_t          sar;
	unsigned                overscan_info_present_flag:1;
	unsigned                overscan_appropriate_flag:1;
	unsigned                video_signal_type_present_flag:1;
	unsigned                video_format:3;
	unsigned                video_full_range_flag:1;
	unsigned                colour_description_present_flag:1;
	stu_uint8_t             colour_primaries;
	stu_uint8_t             transfer_characteristics;
	stu_uint8_t             matrix_coefficients;
	unsigned                chroma_loc_info_present_flag:1;
	stu_uint32_t            chroma_sample_loc_type_top_field;
	stu_uint32_t            chroma_sample_loc_type_bottom_field;
	unsigned                timing_info_present_flag:1;
	stu_uint32_t            num_units_in_tick;
	stu_uint32_t            time_scale;
	unsigned                fixed_frame_rate_flag:1;
	unsigned                nal_hrd_parameters_present_flag:1;
	stu_avc_hrd_t           nal_hrd;
	unsigned                vcl_hrd_parameters_present_flag:1;
	stu_avc_hrd_t           vcl_hrd;
	unsigned                low_delay_hrd_flag:1;
	unsigned                pic_struct_present_flag:1;
	unsigned                bitstream_restriction_flag:1;
	unsigned                motion_vectors_over_pic_boundaries_flag:1;
	stu_uint32_t            max_bytes_per_pic_denom;
	stu_uint32_t            max_bits_per_mb_denom;
	stu_uint32_t            log2_max_mv_length_horizontal;
	stu_uint32_t            log2_max_mv_length_vertical;
	stu_uint32_t            max_num_reorder_frames;
	stu_uint32_t            max_dec_frame_buffering;
} stu_avc_vui_t;

/*
 * Sequence parameter set
 */
typedef struct {
	stu_uint8_t             profile_idc;
	unsigned                constraint_set_flags:6;
	unsigned                reserved_zero_2bits:2;            /* equal to 0 */
	stu_uint8_t             level_idc;
	stu_uint32_t            id;                               // seq_parameter_set_id
	stu_uint32_t            chroma_format_idc;
	unsigned                separate_colour_plane_flag:1;
	stu_uint32_t            bit_depth_luma;                   // bit_depth_luma_minus8 + 8
	stu_uint32_t            bit_depth_chroma;                 // bit_depth_chroma_minus8 + 8
	unsigned                transform_bypass:1;               // qpprime_y_zero_transform_bypass_flag
	unsigned                seq_scaling_matrix_present_flag:1;
	stu_uint8_t             scaling_matrix4[6][16];
	stu_uint8_t             scaling_matrix8[6][64];
	stu_uint32_t            log2_max_frame_num;               // log2_max_frame_num_minus4 + 4
	stu_uint32_t            poc_type;                         // pic_order_cnt_type
	stu_uint32_t            log2_max_poc_lsb;                 // log2_max_pic_order_cnt_lsb_minus4 + 4
	unsigned                delta_pic_order_always_zero_flag:1;
	stu_uint32_t            offset_for_non_ref_pic;
	stu_uint32_t            offset_for_top_to_bottom_field;
	stu_uint32_t            num_ref_frames_in_poc_cycle;      // num_ref_frames_in_pic_order_cnt_cycle
	stu_uint16_t            offset_for_ref_frame[256];
	stu_uint32_t            max_num_ref_frames;
	unsigned                gaps_in_frame_num_value_allowed_flag:1;
	stu_uint32_t            pic_width;                        // pic_width_in_mbs_minus1 + 1
	stu_uint32_t            pic_height;                       // pic_height_in_map_units_minus1 + 1
	unsigned                frame_mbs_only_flag:1;
	unsigned                mb_adaptive_frame_field_flag:1;
	unsigned                direct_8x8_inference_flag:1;
	unsigned                frame_cropping_flag:1;
	stu_uint32_t            frame_crop_left_offset;
	stu_uint32_t            frame_crop_right_offset;
	stu_uint32_t            frame_crop_top_offset;
	stu_uint32_t            frame_crop_bottom_offset;
	unsigned                vui_parameters_present_flag:1;
	stu_avc_vui_t           vui;

	stu_buf_t               data;
} stu_avc_sps_t;

/*
 * Picture parameter set
 */
typedef struct {
	stu_uint32_t            id;                               // pic_parameter_set_id
	stu_uint32_t            sps_id;                           // seq_parameter_set_id

	unsigned                entropy_coding_mode_flag:1;
	unsigned                pic_order_present_flag:1;         // bottom_field_pic_order_in_frame_present_flag
	stu_uint32_t            num_slice_groups;                 // num_slice_groups_minus1 + 1
	stu_uint32_t            slice_group_map_type;
	stu_uint32_t            num_ref_idx[2];                   // num_ref_idx_l0/1_default_active_minus1 + 1
	unsigned                weighted_pred_flag:1;
	unsigned                weighted_bipred_idc:2;
	stu_int32_t             pic_init_qp;                      // pic_init_qp_minus26 + 26
	stu_int32_t             pic_init_qs;                      // pic_init_qs_minus26 + 26
	stu_int32_t             chroma_qp_index_offset[2];
	unsigned                deblocking_filter_control_present_flag:1;
	unsigned                constrained_intra_pred_flag:1;
	unsigned                redundant_pic_cnt_present_flag:1;
	unsigned                transform_8x8_mode_flag:1;
	unsigned                pic_scaling_matrix_present_flag:1;
	stu_uint8_t             scaling_matrix4[6][16];
	stu_uint8_t             scaling_matrix8[6][64];
	stu_uint8_t             chroma_qp_table[2][STU_AVC_MAX_QP_NUM + 1]; // pre-scaled (with chroma_qp_index_offset) version of qp_table
	stu_int32_t             chroma_qp_diff;
	stu_buf_t               data;

	stu_uint32_t            dequant4_buffer[6][STU_AVC_MAX_QP_NUM + 1][16];
	stu_uint32_t            dequant8_buffer[6][STU_AVC_MAX_QP_NUM + 1][64];
	stu_uint32_t          (*dequant4_coeff[6])[16];
	stu_uint32_t          (*dequant8_coeff[6])[64];
} stu_avc_pps_t;

typedef struct {
	stu_av_codec_context_t  avctx;
	stu_bitstream_t         gb;

	// Decoder Configuration Record
	u_char                 *avcc;
	size_t                  avcc_size;
	stu_uint8_t             configuration_version;
	stu_uint8_t             profile_indication;
	stu_uint8_t             profile_compatibility;
	stu_uint8_t             level_indication;
	stu_uint8_t             nal_length_size; // length_size_minus1 + 1
	stu_avc_sps_t           sps;
	stu_avc_pps_t           pps;

	// NAL Units
	unsigned                forbidden_zero_bit:1;
	unsigned                nal_ref_idc:2;
	unsigned                nal_unit_type:5;
} stu_avc_context_t;

stu_int32_t  stu_avc_parse_packet(stu_avc_context_t *ctx, stu_uint32_t timestamp, u_char *data, size_t len);
stu_int32_t  stu_avc_parse_decoder_configuration_record(stu_avc_context_t *ctx, u_char *data, size_t len);
stu_int32_t  stu_avc_parse_nal_units(stu_avc_context_t *ctx, stu_uint32_t timestamp, u_char *data, size_t len);

stu_int32_t  stu_avc_parse_sps(stu_avc_context_t *ctx, u_char *data, size_t len);
stu_int32_t  stu_avc_parse_pps(stu_avc_context_t *ctx, u_char *data, size_t len);

#endif /* STUDEASE_CN_CODEC_STU_AVC_H_ */
