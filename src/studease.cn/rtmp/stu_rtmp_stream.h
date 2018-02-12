/*
 * stu_rtmp_stream.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_STREAM_H_
#define STUDEASE_CN_RTMP_STU_RTMP_STREAM_H_

#include "stu_rtmp.h"

#define STU_RTMP_STREAM_TYPE_IDLE         0x00
#define STU_RTMP_STREAM_TYPE_PUBLISHING   0x01
#define STU_RTMP_STREAM_TYPE_PLAYING_LIVE 0x02
#define STU_RTMP_STREAM_TYPE_PLAYING_VOD  0x04

typedef struct {
	stu_uint32_t              id;
	stu_str_t                 name;
	stu_uint8_t               type;
	stu_hash_t                data_frames;     // *DataMessage.DataMessage
	stu_rtmp_audio_message_t *init_audio;
	stu_rtmp_video_message_t *init_video;
	stu_double_t              time;            // at when to init
	stu_double_t              buffer_time;
	stu_double_t              current_time;
	stu_double_t              duration;
	stu_double_t              max_queue_delay; // ms
	stu_int32_t               max_queue_size;
	stu_bool_t                pause;
	stu_str_t                 publish_query_string;
	stu_bool_t                receive_audio;
	stu_bool_t                receive_video;
} stu_rtmp_stream_t;

void             stu_rtmp_stream_init(stu_rtmp_stream_t *stream);

stu_inline void  stu_rtmp_stream_sink(stu_rtmp_stream_t *stream, stu_rtmp_stream_t *dst);
void             stu_rtmp_stream_source(stu_rtmp_stream_t *stream, stu_rtmp_stream_t *src);

void             stu_rtmp_stream_close(stu_rtmp_stream_t *stream);
void             stu_rtmp_stream_clear(stu_rtmp_stream_t *stream);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_STREAM_H_ */
