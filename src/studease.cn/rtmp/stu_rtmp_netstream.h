/*
 * stu_rtmp_netstream.h
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_NETSTREAM_H_
#define STUDEASE_CN_RTMP_STU_RTMP_NETSTREAM_H_

#include "stu_rtmp.h"

#define STU_RTMP_DATA_FRAMES_SIZE  8

typedef stu_int32_t (*stu_rtmp_status_handler_pt)(stu_rtmp_request_t *r);
typedef stu_int32_t (*stu_rtmp_data_handler_pt)(stu_rtmp_request_t *r);

struct stu_rtmp_netstream_s {
	stu_rtmp_netconnection_t   *connection;
	stu_rtmp_stream_t          *stream;

	stu_rtmp_status_handler_pt  on_status;
	stu_rtmp_data_handler_pt    on_data;

	stu_uint32_t                id;
	stu_str_t                   name;
	stu_str_t                   publish_query_string;

	stu_double_t                buffer_time;
	stu_double_t                time;
	stu_bool_t                  pause;
	stu_bool_t                  receive_audio;
	stu_bool_t                  receive_video;
};

stu_int32_t  stu_rtmp_attach(stu_rtmp_netstream_t *ns, stu_rtmp_netconnection_t *nc);
stu_int32_t  stu_rtmp_sink(stu_rtmp_netstream_t *ns, stu_rtmp_stream_t *s);
stu_int32_t  stu_rtmp_source(stu_rtmp_netstream_t *ns, stu_rtmp_stream_t *s);

stu_int32_t  stu_rtmp_play(stu_rtmp_netstream_t *ns, u_char *name, size_t len, stu_double_t start, stu_double_t duration, stu_bool_t reset);
stu_int32_t  stu_rtmp_play2(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_release_stream(stu_rtmp_netconnection_t *nc, u_char *name, size_t len);
stu_int32_t  stu_rtmp_delete_stream(stu_rtmp_netconnection_t *nc, stu_uint32_t stream_id);
stu_int32_t  stu_rtmp_close_stream(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_receive_audio(stu_rtmp_netstream_t *ns, stu_bool_t flag);
stu_int32_t  stu_rtmp_receive_video(stu_rtmp_netstream_t *ns, stu_bool_t flag);
stu_int32_t  stu_rtmp_publish(stu_rtmp_netstream_t *ns, u_char *name, size_t len, stu_str_t *type);
stu_int32_t  stu_rtmp_seek(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_pause(stu_rtmp_netstream_t *ns, stu_bool_t flag, stu_double_t time);
stu_int32_t  stu_rtmp_send_status(stu_rtmp_netstream_t *ns, stu_str_t *level, stu_str_t *code, const char *description);

stu_int32_t  stu_rtmp_set_data_frame(stu_rtmp_netstream_t *ns, stu_str_t *key, stu_rtmp_amf_t *value, stu_bool_t remote);
stu_int32_t  stu_rtmp_clear_data_frame(stu_rtmp_netstream_t *ns, stu_str_t *key, stu_bool_t remote);
stu_int32_t  stu_rtmp_send_video_frame(stu_rtmp_netstream_t *ns, stu_uint32_t timestamp, stu_rtmp_frame_info_t *info, u_char *data, size_t len);
stu_int32_t  stu_rtmp_send_audio_frame(stu_rtmp_netstream_t *ns, stu_uint32_t timestamp, stu_rtmp_frame_info_t *info, u_char *data, size_t len);

stu_int32_t  stu_rtmp_on_play(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_play2(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_release_stream(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_delete_stream(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_close_stream(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_receive_audio(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_receive_video(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_fcpublish(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_fcunpublish(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_publish(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_seek(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_pause(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_on_status(stu_rtmp_request_t *r);

void         stu_rtmp_close_netstream(stu_rtmp_netstream_t *ns);
void         stu_rtmp_stream_detach(stu_rtmp_netstream_t *ns);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_NETSTREAM_H_ */
