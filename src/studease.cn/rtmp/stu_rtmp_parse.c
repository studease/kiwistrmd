/*
 * stu_rtmp_parse.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

static stu_rtmp_chunk_t *stu_rtmp_get_chunk(stu_rtmp_request_t *r, stu_uint8_t fmt, stu_uint32_t csid);


stu_int32_t
stu_rtmp_parse_handshake(stu_rtmp_handshake_t *h, stu_buf_t *b) {
	u_char *p, ch;
	enum {
		sw_c0_version = 0,
		sw_c1_time,
		sw_c1_zero,
		sw_c1_random,
		sw_c2_sent,
		sw_c2_read,
		sw_c2_random,
		sw_done
	} state;

	state = h->state;

	for (p = b->pos; p < b->last; p++) {
		ch = *p;

		switch (h->state) {
		case sw_c0_version:
			if (ch != STU_RTMP_VERSION_3) {
				stu_log_error(0, "Unexpected rtmp version: %d.", ch);
				break;
			}

			h->rtmp_version = ch;
			state = sw_c1_time;
			break;

		case sw_c1_time:
			if (b->last - p < 4) {
				goto again;
			}

			h->start = p;
			h->time = stu_endian_32(*(stu_uint32_t *) p);
			p += 3;

			state = sw_c1_zero;
			break;

		case sw_c1_zero:
			if (b->last - p < 4) {
				goto again;
			}

			h->zero = stu_endian_32(*(stu_uint32_t *) p);
			p += 3;

			state = sw_c1_random;
			break;

		case sw_c1_random:
			if (b->last - p < STU_RTMP_HANDSHAKE_RANDOM_SIZE) {
				goto again;
			}

			h->random = p;

			goto done;

		case sw_c2_sent:
			if (b->last - p < 4) {
				goto again;
			}

			h->start = p;
			h->time = stu_endian_32(*(stu_uint32_t *) p);
			p += 3;

			state = sw_c2_read;
			break;

		case sw_c2_read:
			if (b->last - p < 4) {
				goto again;
			}

			h->zero = stu_endian_32(*(stu_uint32_t *) p);
			p += 3;

			state = sw_c2_random;
			break;

		case sw_c2_random:
			if (b->last - p < STU_RTMP_HANDSHAKE_RANDOM_SIZE) {
				goto again;
			}

			h->random = p;
			/* no break */

		case sw_done:
			goto hs_done;

		default:
			stu_log_error(0, "Unknown rtmp handshake state: %d.", h->state);
			break;
		}
	}

again:

	b->pos = p;
	h->state = state;

	return STU_AGAIN;

done:

	b->pos = p + 1;
	h->state = sw_c2_sent;

	return STU_OK;

hs_done:

	b->pos = p + 1;
	h->state = sw_done;

	return STU_DONE;
}


stu_int32_t
stu_rtmp_parse_chunk(stu_rtmp_request_t *r, stu_buf_t *b) {
	stu_rtmp_chunk_t *ck;
	u_char           *p, ch;
	stu_uint8_t       fmt;
	stu_uint32_t      csid, len, n;
	enum {
		sw_fmt = 0,
		sw_csid_0,
		sw_csid_1,
		sw_time_0,
		sw_time_1,
		sw_time_2,
		sw_mlen_0,
		sw_mlen_1,
		sw_mlen_2,
		sw_mtid,
		sw_msid_0,
		sw_msid_1,
		sw_msid_2,
		sw_msid_3,
		sw_extm_0,
		sw_extm_1,
		sw_extm_2,
		sw_extm_3,
		sw_data
	} state;

	state = r->state;

	ck = stu_rtmp_get_chunk(r, 0, 0);

	for (p = b->pos; p < b->last; p++) {
		ch = *p;

		switch (state) {
		case sw_fmt:
			fmt = ch >> 6;
			csid = ch & 0x3F;

			switch (csid) {
			case 0:
				csid = 0;
				state = sw_csid_1;
				break;
			case 1:
				state = sw_csid_0;
				break;
			default:
				ck = stu_rtmp_get_chunk(r, fmt, csid);
				if (ck == NULL) {
					return STU_ERROR;
				}

				if (ck->fmt == 3) {
					state = ck->extended ? sw_extm_0 : sw_data;
				} else {
					state = sw_time_0;
				}
				break;
			}

			state = sw_csid_0;
			break;

		case sw_csid_0:
			csid = ch;
			state = sw_csid_1;
			break;

		case sw_csid_1:
			csid |= ch << 8;
			csid += 64;

			ck = stu_rtmp_get_chunk(r, fmt, csid);
			if (ck == NULL) {
				return STU_ERROR;
			}

			if (ck->fmt == 3) {
				state = ck->extended ? sw_extm_0 : sw_data;
			} else {
				state = sw_time_0;
			}
			break;

		case sw_time_0:
			ck->timestamp = ch << 16;
			state = sw_time_1;
			break;

		case sw_time_1:
			ck->timestamp |= ch << 8;
			state = sw_time_2;
			break;

		case sw_time_2:
			ck->timestamp |= ch;
			if (ck->timestamp == 0xFFFFFF) {
				ck->extended = TRUE;
			}

			if (fmt == 2) {
				state = ck->extended ? sw_extm_0 : sw_data;
			} else {
				state = sw_mlen_0;
			}
			break;

		case sw_mlen_0:
			ck->payload.size = ch << 16;
			state = sw_mlen_1;
			break;

		case sw_mlen_1:
			ck->payload.size |= ch << 8;
			state = sw_mlen_2;
			break;

		case sw_mlen_2:
			ck->payload.size |= ch;
			state = sw_mtid;
			break;

		case sw_mtid:
			ck->type = ch;

			if (ck->fmt == 1) {
				state = ck->extended ? sw_extm_0 : sw_data;
			} else {
				state = sw_msid_0;
			}
			break;

		case sw_msid_0:
			ck->msid = ch;
			state = sw_msid_1;
			break;

		case sw_msid_1:
			ck->msid |= ch << 8;
			state = sw_msid_2;
			break;

		case sw_msid_2:
			ck->msid |= ch << 16;
			state = sw_msid_3;
			break;

		case sw_msid_3:
			ck->msid |= ch << 24;
			state = ck->extended ? sw_extm_0 : sw_data;
			break;

		case sw_extm_0:
			ck->timestamp = ch << 24;
			state = sw_extm_1;
			break;

		case sw_extm_1:
			ck->timestamp |= ch << 16;
			state = sw_extm_2;
			break;

		case sw_extm_2:
			ck->timestamp |= ch << 8;
			state = sw_extm_3;
			break;

		case sw_extm_3:
			ck->timestamp |= ch;
			state = sw_data;
			break;

		case sw_data:
			if (ck->payload.start == NULL) {
				ck->payload.start = stu_calloc(ck->payload.size + 1);
				if (ck->payload.start == NULL) {
					return STU_ERROR;
				}

				ck->payload.pos = ck->payload.last = ck->payload.start;
				ck->payload.end = ck->payload.start + ck->payload.size;
			}

			n = (ck->payload.last - ck->payload.start) % r->nc->far_chunk_size;
			if (n == 0) {
				n = r->nc->far_chunk_size;
			}

			len = stu_min(ck->payload.end - ck->payload.last, b->last - p);
			if (len >= n) {
				state = sw_fmt;
			}

			p += len - 1;

			ck->payload.last = stu_memcpy(ck->payload.last, p, len);
			if (ck->payload.last == ck->payload.end) {
				state = sw_fmt;
				goto done;
			}
			break;

		default:
			stu_log_error(0, "Unknown rtmp chunk state: %d.", state);
			return STU_ERROR;
		}
	}

	b->pos = p;
	r->state = ck->state = state;

	return STU_AGAIN;

done:

	b->pos = p + 1;
	r->state = ck->state = state;

	return STU_OK;
}

static stu_rtmp_chunk_t *
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
