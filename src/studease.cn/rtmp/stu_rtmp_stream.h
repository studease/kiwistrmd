/*
 * stu_rtmp_stream.h
 *
 *  Created on: 2018年5月9日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_STREAM_H_
#define STUDEASE_CN_RTMP_STU_RTMP_STREAM_H_

#include "stu_rtmp.h"

typedef struct {
	stu_queue_t       queue;

	stu_uint8_t       type;          // 1 byte
	stu_uint32_t      timestamp;     // 4 bytes
	stu_uint32_t      stream_id;     // 3 bytes

	union {
		struct {
			unsigned  frame_type:4;  // 0xF0
			unsigned  codec:4;       // 0x0F
			unsigned  data_type;
		} video;

		struct {
			unsigned  format:4;      // 1111 0000
			unsigned  sample_rate:2; // 0000 1100
			unsigned  sample_size:1; // 0000 0010
			unsigned  channels:1;    // 0000 0001
			unsigned  data_type;
		} audio;
	} info;

	stu_buf_t         payload;
} stu_rtmp_frame_t;

typedef struct {
	stu_str_t         name;
	stu_file_t        file;

	stu_hash_t        data_frames;

	stu_rtmp_frame_t *video_init;
	stu_rtmp_frame_t *audio_init;
	stu_list_t        frames;

	stu_double_t      time;
	stu_double_t      duration;
	stu_double_t      max_queue_delay;
	stu_int32_t       max_queue_size;
	stu_int32_t       subscribers;

	unsigned          publishing:1;
	unsigned          playing:1;
	unsigned          record:1;
	unsigned          destroyed:1;
} stu_rtmp_stream_t;

stu_rtmp_stream_t *stu_rtmp_stream_get(u_char *name, size_t len);
stu_rtmp_frame_t  *stu_rtmp_stream_append(stu_rtmp_stream_t *s, stu_uint8_t type, stu_uint32_t timestamp, u_char *data, size_t len);
void               stu_rtmp_stream_drop(stu_rtmp_stream_t *s, stu_uint32_t timestamp);
void               stu_rtmp_stream_free(stu_rtmp_stream_t *s);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_STREAM_H_ */
