/*
 * stu_rtmp_chunk.c
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"


stu_int32_t
stu_rtmp_chunk_append(stu_rtmp_chunk_t *ck, u_char *data, size_t len) {
	if (ck->payload.end - ck->payload.last < len) {
		if (stu_rtmp_chunk_grow(ck, len) == STU_ERROR) {
			stu_log_error(0, "Failed to grow chunk: size=%d.", len);
			return STU_ERROR;
		}
	}

	ck->payload.last = stu_memcpy(ck->payload.last, data, len);

	return STU_OK;
}

stu_int32_t
stu_rtmp_chunk_grow(stu_rtmp_chunk_t *ck, size_t size) {
	stu_buf_t  buf;

	buf.size = ck->payload.size + size;

	buf.start = (u_char *) stu_calloc(buf.size);
	if (buf.start == NULL) {
		return STU_ERROR;
	}

	buf.pos = buf.start + (ck->payload.pos - ck->payload.start);
	buf.last = stu_memcpy(buf.start, ck->payload.start, ck->payload.last - ck->payload.start);
	buf.end = buf.start + buf.size;

	if (ck->payload.start) {
		stu_free(ck->payload.start);
	}

	ck->payload = buf;

	return STU_OK;
}

void
stu_rtmp_chunk_cleanup(stu_rtmp_chunk_t *ck) {
	ck->fmt = 0;
	ck->csid = 0;

	ck->timestamp = 0;
	ck->type_id = 0;
	ck->stream_id = 0;

	if (ck->payload.start) {
		stu_free(ck->payload.start);
	}

	ck->payload.start = NULL;
	ck->payload.pos = ck->payload.last = ck->payload.start;
	ck->payload.end = NULL;
	ck->payload.size = 0;

	ck->state = 0;
	ck->extended = FALSE;
}
