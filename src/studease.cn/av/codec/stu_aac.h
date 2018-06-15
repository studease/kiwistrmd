/*
 * stu_aac.h
 *
 *  Created on: 2018��4��9��
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_CODEC_STU_AAC_H_
#define STUDEASE_CN_CODEC_STU_AAC_H_

#include "stu_codec.h"

#define STU_AAC_TYPE_SPECIFIC_CONFIG  0x00
#define STU_AAC_TYPE_RAW_FRAME_DATA   0x01

typedef enum {
	STU_AOT_NULL,                  // Support?                Name
	STU_AOT_AAC_MAIN,              // Y                       Main
	STU_AOT_AAC_LC,                // Y                       Low Complexity
	STU_AOT_AAC_SSR,               // N (code in SoC repo)    Scalable Sample Rate
	STU_AOT_AAC_LTP,               // Y                       Long Term Prediction
	STU_AOT_SBR,                   // Y                       Spectral Band Replication
	STU_AOT_AAC_SCALABLE,          // N                       Scalable
	STU_AOT_TWINVQ,                // N                       Twin Vector Quantizer
	STU_AOT_CELP,                  // N                       Code Excited Linear Prediction
	STU_AOT_HVXC,                  // N                       Harmonic Vector eXcitation Coding
	STU_AOT_TTSI             = 12, // N                       Text-To-Speech Interface
	STU_AOT_MAINSYNTH,             // N                       Main Synthesis
	STU_AOT_WAVESYNTH,             // N                       Wavetable Synthesis
	STU_AOT_MIDI,                  // N                       General MIDI
	STU_AOT_SAFX,                  // N                       Algorithmic Synthesis and Audio Effects
	STU_AOT_ER_AAC_LC,             // N                       Error Resilient Low Complexity
	STU_AOT_ER_AAC_LTP       = 19, // N                       Error Resilient Long Term Prediction
	STU_AOT_ER_AAC_SCALABLE,       // N                       Error Resilient Scalable
	STU_AOT_ER_TWINVQ,             // N                       Error Resilient Twin Vector Quantizer
	STU_AOT_ER_BSAC,               // N                       Error Resilient Bit-Sliced Arithmetic Coding
	STU_AOT_ER_AAC_LD,             // N                       Error Resilient Low Delay
	STU_AOT_ER_CELP,               // N                       Error Resilient Code Excited Linear Prediction
	STU_AOT_ER_HVXC,               // N                       Error Resilient Harmonic Vector eXcitation Coding
	STU_AOT_ER_HILN,               // N                       Error Resilient Harmonic and Individual Lines plus Noise
	STU_AOT_ER_PARAM,              // N                       Error Resilient Parametric
	STU_AOT_SSC,                   // N                       SinuSoidal Coding
	STU_AOT_PS,                    // N                       Parametric Stereo
	STU_AOT_SURROUND,              // N                       MPEG Surround
	STU_AOT_ESCAPE,                // Y                       Escape Value
	STU_AOT_L1,                    // Y                       Layer 1
	STU_AOT_L2,                    // Y                       Layer 2
	STU_AOT_L3,                    // Y                       Layer 3
	STU_AOT_DST,                   // N                       Direct Stream Transfer
	STU_AOT_ALS,                   // Y                       Audio LosslesS
	STU_AOT_SLS,                   // N                       Scalable LosslesS
	STU_AOT_SLS_NON_CORE,          // N                       Scalable LosslesS (non core)
	STU_AOT_ER_AAC_ELD,            // N                       Error Resilient Enhanced Low Delay
	STU_AOT_SMR_SIMPLE,            // N                       Symbolic Music Representation Simple
	STU_AOT_SMR_MAIN,              // N                       Symbolic Music Representation Main
	STU_AOT_USAC_NOSBR,            // N                       Unified Speech and Audio Coding (no SBR)
	STU_AOT_SAOC,                  // N                       Spatial Audio Object Coding
	STU_AOT_LD_SURROUND,           // N                       Low Delay MPEG Surround
	STU_AOT_USAC,                  // N                       Unified Speech and Audio Coding
} stu_aot_e;

typedef struct {
	stu_av_codec_context_t  avctx;
	stu_bitstream_t         gb;

	// Specific Config
	unsigned                audio_object_type:5;
	unsigned                sampling_frequency_index:4;
	stu_uint32_t            sampling_frequency;
	unsigned                channel_configuration:4;
	stu_uint16_t            channels;
	unsigned                ext_audio_object_type:5;
	unsigned                ext_sampling_frequency_index:4;
	unsigned                ext_sampling_frequency:24;
	unsigned                ext_channel_configuration:4;
	u_char                  conf[4];
	size_t                  conf_size;
} stu_aac_context_t;

stu_int32_t  stu_aac_parse_packet(stu_aac_context_t *ctx, stu_uint32_t timestamp, u_char *data, size_t len);
stu_int32_t  stu_aac_parse_specific_config(stu_aac_context_t *ctx, u_char *data, size_t len);
stu_int32_t  stu_aac_parse_data(stu_aac_context_t *ctx, stu_uint32_t timestamp, u_char *data, size_t len);

#endif /* STUDEASE_CN_CODEC_STU_AAC_H_ */
