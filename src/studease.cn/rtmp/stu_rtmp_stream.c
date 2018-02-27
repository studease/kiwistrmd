/*
 * stu_rtmp_stream.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"


void
stu_rtmp_stream_init(stu_rtmp_stream_t *stream) {
	stream->id = 1; // channel id 0 is used as NetConnection
	stream->type = STU_RTMP_STREAM_TYPE_IDLE;
	stream->receive_audio = TRUE;
	stream->receive_video = TRUE;

	stu_hash_init(&stream->data_frames, STU_RTMP_STREAM_DATA_FRAMES_SIZE, NULL, STU_HASH_FLAGS_LOWCASE | STU_HASH_FLAGS_REPLACE);
}


stu_inline void
stu_rtmp_stream_sink(stu_rtmp_stream_t *stream, stu_rtmp_stream_t *dst) {
	stu_rtmp_stream_source(dst, stream);
}

void
stu_rtmp_stream_source(stu_rtmp_stream_t *stream, stu_rtmp_stream_t *src) {

}


void
stu_rtmp_stream_close(stu_rtmp_stream_t *stream) {

}

void
stu_rtmp_stream_clear(stu_rtmp_stream_t *stream) {

}
