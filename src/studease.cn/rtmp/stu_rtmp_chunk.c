/*
 * stu_rtmp_chunk.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"


stu_rtmp_chunk_t *
stu_rtmp_get_chunk(stu_rtmp_request_t *r, stu_uint8_t fmt, stu_uint32_t csid) {
	stu_rtmp_chunk_t *ck;
	stu_queue_t      *q;

	ck = NULL;

	if (fmt == 0) {
		ck = stu_calloc(sizeof(stu_rtmp_chunk_t));
		if (ck == NULL) {
			stu_log_error(0, "Failed to calloc rtmp chunk.");
			return NULL;
		}

		ck->fmt = 0;
		ck->csid = csid;
	} else {
		for (q = stu_queue_tail(&r->chunks); q != stu_queue_sentinel(&r->chunks); q = stu_queue_prev(q)) {
			ck = stu_queue_data(q, stu_rtmp_chunk_t, queue);
			if (ck->state || ck->csid == csid) {
				ck->fmt = fmt;
				ck->csid = csid;
				break;
			}
		}
	}

	return ck;
}

void
stu_rtmp_free_chunk(stu_rtmp_request_t *r, stu_uint32_t csid) {
	stu_rtmp_chunk_t *ck;
	stu_queue_t      *q;

	for (q = stu_queue_tail(&r->chunks); q != stu_queue_sentinel(&r->chunks); q = stu_queue_prev(q)) {
		ck = stu_queue_data(q, stu_rtmp_chunk_t, queue);
		if (ck->csid == csid) {
			stu_queue_remove(q);
			break;
		}
	}
}
