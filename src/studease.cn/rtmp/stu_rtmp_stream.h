/*
 * stu_rtmp_stream.h
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_STREAM_H_
#define STUDEASE_CN_RTMP_STU_RTMP_STREAM_H_

#include "stu_rtmp.h"

#define STU_RTMP_DATA_FRAMES_SIZE         8

#define STU_RTMP_STREAM_TYPE_IDLE         0x00
#define STU_RTMP_STREAM_TYPE_PUBLISHING   0x01
#define STU_RTMP_STREAM_TYPE_PLAYING_LIVE 0x02
#define STU_RTMP_STREAM_TYPE_PLAYING_VOD  0x04

typedef stu_int32_t  (*stu_rtmp_status_handler_pt)(stu_rtmp_request_t *r);

typedef struct {
	stu_uint8_t                 type;          // 1 byte
	stu_uint32_t                timestamp;     // 4 bytes
	stu_uint32_t                stream_id;     // 3 bytes

	unsigned                    format:4;      // 1111 0000
	unsigned                    sample_rate:2; // 0000 1100
	unsigned                    sample_size:1; // 0000 0010
	unsigned                    channels:1;    // 0000 0001
	stu_uint8_t                 data_type;

	stu_buf_t                   payload;
} stu_rtmp_audio_frame_t;

typedef struct {
	stu_uint8_t                 type;         // 1 byte
	stu_uint32_t                timestamp;    // 4 bytes
	stu_uint32_t                stream_id;    // 3 bytes

	unsigned                    frame_type:4; // 0xF0
	unsigned                    codec:4;      // 0x0F
	stu_uint8_t                 data_type;

	stu_buf_t                   payload;
} stu_rtmp_video_frame_t;

typedef struct {
	stu_rtmp_connection_t      *connection;

	stu_rtmp_status_handler_pt  on_status;

	stu_uint32_t                id;
	stu_str_t                   name;
	stu_uint8_t                 type;

	stu_hash_t                  data_frames;
	stu_rtmp_amf_t             *metadata;
	stu_rtmp_audio_frame_t     *audio_init;
	stu_rtmp_video_frame_t     *video_init;

	stu_double_t                buffer_time;
	stu_double_t                time;
	stu_double_t                duration;
	stu_double_t                max_queue_delay;
	stu_int32_t                 max_queue_size;
	stu_bool_t                  pause;
	stu_str_t                   publish_query_string;
	stu_bool_t                  receive_audio;
	stu_bool_t                  receive_video;
} stu_rtmp_stream_t;

void         stu_rtmp_stream_attach(stu_rtmp_stream_t *ns, stu_rtmp_connection_t *nc);

stu_int32_t  stu_rtmp_play(stu_rtmp_stream_t *ns, u_char *name, size_t len, stu_double_t start, stu_double_t duration, stu_bool_t reset);
stu_int32_t  stu_rtmp_play2(stu_rtmp_stream_t *ns);
stu_int32_t  stu_rtmp_release_stream(stu_rtmp_connection_t *nc, u_char *name, size_t len);
stu_int32_t  stu_rtmp_delete_stream(stu_rtmp_connection_t *nc, stu_uint32_t stream_id);
stu_int32_t  stu_rtmp_close_stream(stu_rtmp_stream_t *ns);
stu_int32_t  stu_rtmp_receive_audio(stu_rtmp_stream_t *ns, stu_bool_t flag);
stu_int32_t  stu_rtmp_receive_video(stu_rtmp_stream_t *ns, stu_bool_t flag);
stu_int32_t  stu_rtmp_publish(stu_rtmp_stream_t *ns, u_char *name, size_t len, stu_str_t *type);
stu_int32_t  stu_rtmp_seek(stu_rtmp_stream_t *ns);
stu_int32_t  stu_rtmp_pause(stu_rtmp_stream_t *ns, stu_bool_t flag, stu_double_t time);
stu_int32_t  stu_rtmp_send_status(stu_rtmp_stream_t *ns, stu_str_t *level, stu_str_t *code, const char *description);

stu_int32_t  stu_rtmp_set_data_frame(stu_rtmp_stream_t *ns, stu_str_t *key, stu_rtmp_amf_t *value, stu_bool_t remote);
stu_int32_t  stu_rtmp_clear_data_frame(stu_rtmp_stream_t *ns, stu_str_t *key, stu_bool_t remote);
stu_int32_t  stu_rtmp_send_video_frame(stu_rtmp_stream_t *ns, stu_uint32_t timestamp, u_char *data, size_t len);
stu_int32_t  stu_rtmp_send_audio_frame(stu_rtmp_stream_t *ns, stu_uint32_t timestamp, u_char *data, size_t len);

stu_int32_t  stu_rtmp_on_play(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_play2(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_release_stream(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_delete_stream(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_close_stream(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_receive_audio(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_receive_video(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_fcpublish(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_publish(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_seek(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_pause(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_status(stu_rtmp_request_t *r);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_STREAM_H_ */
