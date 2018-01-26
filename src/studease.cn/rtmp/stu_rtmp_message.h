/*
 * stu_rtmp_message.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_MESSAGE_H_
#define STUDEASE_CN_RTMP_STU_RTMP_MESSAGE_H_

#include "stu_rtmp.h"

#define STU_RTMP_MSG_TYPE_SET_CHUNK_SIZE      0x01
#define STU_RTMP_MSG_TYPE_ABORT               0x02
#define STU_RTMP_MSG_TYPE_ACK                 0x03
#define STU_RTMP_MSG_TYPE_USER_CONTROL        0x04
#define STU_RTMP_MSG_TYPE_ACK_WINDOW_SIZE     0x05
#define STU_RTMP_MSG_TYPE_BANDWIDTH           0x06
#define STU_RTMP_MSG_TYPE_EDGE                0x07
#define STU_RTMP_MSG_TYPE_AUDIO               0x08
#define STU_RTMP_MSG_TYPE_VIDEO               0x09
#define STU_RTMP_MSG_TYPE_AMF3_DATA           0x0F
#define STU_RTMP_MSG_TYPE_AMF3_SHARED_OBJECT  0x10
#define STU_RTMP_MSG_TYPE_AMF3_COMMAND        0x11
#define STU_RTMP_MSG_TYPE_DATA                0x12
#define STU_RTMP_MSG_TYPE_SHARED_OBJECT       0x13
#define STU_RTMP_MSG_TYPE_COMMAND             0x14
#define STU_RTMP_MSG_TYPE_AGGREGATE           0x16

#define STU_RTMP_EVENT_TYPE_STREAM_BEGIN        0X0000
#define STU_RTMP_EVENT_TYPE_STREAM_EOF          0X0001
#define STU_RTMP_EVENT_TYPE_STREAM_DRY          0X0002
#define STU_RTMP_EVENT_TYPE_SET_BUFFER_LENGTH   0X0003
#define STU_RTMP_EVENT_TYPE_STREAM_IS_RECORDED  0X0004
#define STU_RTMP_EVENT_TYPE_PING_REQUEST        0X0006
#define STU_RTMP_EVENT_TYPE_PING_RESPONSE       0X0007

#define STU_RTMP_AUDIO_FORMAT_LINEAR_PCM_PLATFORM_ENDIAN    0x00
#define STU_RTMP_AUDIO_FORMAT_ADPCM                         0x01
#define STU_RTMP_AUDIO_FORMAT_MP3                           0x02
#define STU_RTMP_AUDIO_FORMAT_LINEAR_PCM_LITTLE_ENDIAN      0x03
#define STU_RTMP_AUDIO_FORMAT_NELLYMOSER_16_kHz_MONO        0x04
#define STU_RTMP_AUDIO_FORMAT_NELLYMOSER_8_kHz_MONO         0x05
#define STU_RTMP_AUDIO_FORMAT_NELLYMOSER                    0x06
#define STU_RTMP_AUDIO_FORMAT_G_711_A_LAW_LOGARITHMIC_PCM   0x07
#define STU_RTMP_AUDIO_FORMAT_G_711_MU_LAW_LOGARITHMIC_PCM  0x08
#define STU_RTMP_AUDIO_FORMAT_RESERVED                      0x09
#define STU_RTMP_AUDIO_FORMAT_AAC                           0x0A
#define STU_RTMP_AUDIO_FORMAT_SPEEX                         0x0B
#define STU_RTMP_AUDIO_FORMAT_MP3_8_kHz                     0x0E
#define STU_RTMP_AUDIO_FORMAT_DEVICE_SPECIFIC_SOUND         0x0F

#define STU_RTMP_VIDEO_CODEC_JPEG            0x01
#define STU_RTMP_VIDEO_CODEC_H263            0x02
#define STU_RTMP_VIDEO_CODEC_SCREEN_VIDEO    0x03
#define STU_RTMP_VIDEO_CODEC_VP6             0x04
#define STU_RTMP_VIDEO_CODEC_VP6_ALPHA       0x05
#define STU_RTMP_VIDEO_CODEC_SCREEN_VIDEO_2  0x06
#define STU_RTMP_VIDEO_CODEC_AVC             0x07

typedef struct {
	stu_uint8_t                type;        // 1 byte
	stu_uint32_t               payload_len; // 3 bytes
	stu_uint32_t               timestamp;   // 4 bytes
	stu_uint32_t               stream_id;   // 3 bytes
} stu_rtmp_message_header_t;

typedef struct {
	stu_rtmp_message_header_t  header;
	stu_buf_t                  payload;

	void                      *data;

	// used for parsing rtmp message.
	stu_uint8_t                state;
} stu_rtmp_message_t;

typedef struct {
	stu_rtmp_message_header_t  header;
	unsigned                   format:4;      // 1111 0000
	unsigned                   sample_rate:2; // 0000 1100
	unsigned                   sample_size:1; // 0000 0010
	unsigned                   channels:1;    // 0000 0001
	stu_uint8_t                data_type;
	stu_buf_t                  payload;
} stu_rtmp_audio_message_t;

typedef struct {
	stu_rtmp_message_header_t  header;
	unsigned                   frame_type:4; // 0xF0
	unsigned                   codec:4;      // 0x0F
	stu_uint8_t                data_type;
	stu_buf_t                  payload;
} stu_rtmp_video_message_t;

typedef struct {
	stu_rtmp_message_header_t  header;
	stu_str_t                  handler;
	stu_str_t                  key;
	stu_rtmp_amf_t            *value;
	stu_buf_t                  payload;
} stu_rtmp_data_message_t;

typedef struct {
	stu_rtmp_message_header_t  header;
	stu_str_t                  name;
	stu_uint64_t               transaction_id;
	stu_rtmp_amf_t            *command_obj;
	stu_rtmp_amf_t            *arguments;
	stu_rtmp_amf_t            *response;
	stu_uint64_t               stream_id;
	stu_str_t                  stream_name;
	stu_double_t               start;
	stu_double_t               duration;
	stu_bool_t                 reset;
	stu_bool_t                 flag;
	stu_str_t                  publishing_name;
	stu_str_t                  publishing_type;
	stu_double_t               milliseconds;
	stu_bool_t                 pause;
} stu_rtmp_command_message_t;

#endif /* STUDEASE_CN_RTMP_STU_RTMP_MESSAGE_H_ */
