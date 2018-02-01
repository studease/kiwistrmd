/*
 * stu_rtmp_chunk.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"


stu_int32_t
stu_rtmp_write_by_chunk(stu_rtmp_request_t *r, stu_rtmp_chunk_t *ck) {
	u_char       *p;
	u_char        tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_uint32_t  i, size;
	stu_int32_t   n;

	if (ck->header.message_len < 2) {
		stu_log_error(0, "Failed to write chunk: Data not enough[1].");
		return STU_ERROR;
	}

	p = tmp;
	ck->payload.pos = ck->payload.start;

	for (i = 0; i < ck->header.message_len; /* void */) {
		if (ck->basic.csid < 64) {
			*p++ = ck->basic.fmt << 6 | ck->basic.csid;
		} else if (ck->basic.csid < 320) {
			*p++ = ck->basic.fmt << 6 | 0x00;
			*p++ = ck->basic.csid - 64;
		} else if (ck->basic.csid < 65600) {
			*p++ = ck->basic.fmt << 6 | 0x01;
			*p++ = ck->basic.csid;
			*p++ = ck->basic.csid >> 8;
		} else {
			stu_log_error(0, "Failed to write chunk: Chunk size out of range.");
			return STU_ERROR;
		}

		if (ck->basic.fmt < 3) {
			if (ck->header.timestamp < 0xFFFFFF) {
				*p++ = ck->header.timestamp >> 16;
				*p++ = ck->header.timestamp >> 8;
				*p++ = ck->header.timestamp;
			} else {
				*p++ = 0xFF;
				*p++ = 0xFF;
				*p++ = 0xFF;
			}
		}

		if (ck->basic.fmt < 2) {
			*p++ = ck->header.message_len >> 16;
			*p++ = ck->header.message_len >> 8;
			*p++ = ck->header.message_len;

			*p++ = ck->header.type;
		}

		if (ck->basic.fmt == 0) {
			*p++ = ck->header.stream_id;
			*p++ = ck->header.stream_id >> 8;
			*p++ = ck->header.stream_id >> 16;
			*p++ = ck->header.stream_id >> 24;
		}

		// extended timestamp
		if (ck->header.timestamp >= 0xFFFFFF) {
			*p++ = ck->header.timestamp >> 24;
			*p++ = ck->header.timestamp >> 16;
			*p++ = ck->header.timestamp >> 8;
			*p++ = ck->header.timestamp;
		}

		// chunk data
		size = stu_min(ck->header.message_len - i, r->nc.near_chunk_size);
		p = stu_memcpy(p, ck->payload.pos, size);

		n = send(r->nc.connection->fd, tmp, p - tmp, 0);
		if (n == -1) {
			stu_log_error(stu_errno, "Failed to write chunk.");
			return STU_ERROR;
		}

		i += size;
		r->nc.stat.bytes_out += size;

		if (i < ck->header.message_len) {
			ck->basic.fmt = 3;
		}
	}

	return STU_OK;
}


stu_rtmp_chunk_t *
stu_rtmp_get_chunk(stu_rtmp_request_t *r, stu_uint32_t csid) {
	stu_rtmp_chunk_t *ck;
	stu_queue_t      *q;

	for (q = stu_queue_tail(&r->chunks); q != stu_queue_sentinel(&r->chunks); q = stu_queue_prev(q)) {
		ck = stu_queue_data(q, stu_rtmp_chunk_t, queue);
		if (ck->state || ck->basic.csid == csid) {
			goto done;
		}
	}

	ck = stu_calloc(sizeof(stu_rtmp_chunk_t));
	if (ck == NULL) {
		stu_log_error(0, "Failed to calloc rtmp chunk.");
		return NULL;
	}

	stu_queue_insert_tail(&r->chunks, &ck->queue);

done:

	ck->basic.csid = csid;

	return ck;
}

void
stu_rtmp_free_chunk(stu_rtmp_request_t *r, stu_uint32_t csid) {
	stu_rtmp_chunk_t *ck;
	stu_queue_t      *q;

	for (q = stu_queue_tail(&r->chunks); q != stu_queue_sentinel(&r->chunks); q = stu_queue_prev(q)) {
		ck = stu_queue_data(q, stu_rtmp_chunk_t, queue);
		if (ck->basic.csid == csid) {
			stu_queue_remove(q);
			break;
		}
	}
}
