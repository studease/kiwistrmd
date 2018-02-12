/*
 * stu_rtmp_netstream.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_NETSTREAM_H_
#define STUDEASE_CN_RTMP_STU_RTMP_NETSTREAM_H_

#include "stu_rtmp.h"

typedef struct {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_stream_t        *stream;
} stu_rtmp_netstream_t;

void         stu_rtmp_netstream_attach(stu_rtmp_netstream_t *ns, stu_rtmp_stream_t *stream);

stu_int32_t  stu_rtmp_on_play(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_play2(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_delete_stream(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_close_stream(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_receive_audio(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_receive_video(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_publish(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_seek(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_pause(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_status(stu_rtmp_netstream_t *ns);

stu_int32_t  stu_rtmp_on_set_buffer_length(stu_rtmp_netstream_t *ns);

stu_int32_t  stu_rtmp_on_set_data_frame(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_clear_data_frame(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_audio_frame(stu_rtmp_netstream_t *ns);
stu_int32_t  stu_rtmp_on_video_frame(stu_rtmp_netstream_t *ns);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_NETSTREAM_H_ */
