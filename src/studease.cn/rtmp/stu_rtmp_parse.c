/*
 * stu_rtmp_parse.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"


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

		switch (state) {
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
			stu_log_error(0, "Unknown rtmp handshake state: %d.", state);
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
	stu_uint32_t      n;
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

	ck = stu_rtmp_get_chunk(r, 0);
	if (ck == NULL) {
		return STU_ERROR;
	}

	for (p = b->pos; p < b->last; p++) {
		ch = *p;

		switch (state) {
		case sw_fmt:
			ck->basic.fmt = ch >> 6;
			ck->basic.csid = ch & 0x3F;

			switch (ck->basic.csid) {
			case 0:
				state = sw_csid_1;
				break;
			case 1:
				state = sw_csid_0;
				break;
			default:
				if (ck->basic.fmt == 3) {
					state = ck->extended ? sw_extm_0 : sw_data;
				} else {
					state = sw_time_0;
				}
				break;
			}
			break;

		case sw_csid_0:
			ck->basic.csid = ch;
			state = sw_csid_1;
			break;

		case sw_csid_1:
			if (ck->basic.csid == 0) {
				ck->basic.csid = ch;
			} else {
				ck->basic.csid |= ch << 8;
			}
			ck->basic.csid += 64;

			if (ck->basic.fmt == 3) {
				state = ck->extended ? sw_extm_0 : sw_data;
			} else {
				state = sw_time_0;
			}
			break;

		case sw_time_0:
			ck->header.timestamp = ch << 16;
			state = sw_time_1;
			break;

		case sw_time_1:
			ck->header.timestamp |= ch << 8;
			state = sw_time_2;
			break;

		case sw_time_2:
			ck->header.timestamp |= ch;
			if (ck->header.timestamp == 0xFFFFFF) {
				ck->extended = TRUE;
			}

			if (ck->basic.fmt == 2) {
				state = ck->extended ? sw_extm_0 : sw_data;
			} else {
				state = sw_mlen_0;
			}
			break;

		case sw_mlen_0:
			ck->header.message_len = ch << 16;
			state = sw_mlen_1;
			break;

		case sw_mlen_1:
			ck->header.message_len |= ch << 8;
			state = sw_mlen_2;
			break;

		case sw_mlen_2:
			ck->header.message_len |= ch;
			state = sw_mtid;
			break;

		case sw_mtid:
			ck->header.type = ch;

			if (ck->basic.fmt == 1) {
				state = ck->extended ? sw_extm_0 : sw_data;
			} else {
				state = sw_msid_0;
			}
			break;

		case sw_msid_0:
			ck->header.stream_id = ch;
			state = sw_msid_1;
			break;

		case sw_msid_1:
			ck->header.stream_id |= ch << 8;
			state = sw_msid_2;
			break;

		case sw_msid_2:
			ck->header.stream_id |= ch << 16;
			state = sw_msid_3;
			break;

		case sw_msid_3:
			ck->header.stream_id |= ch << 24;
			state = ck->extended ? sw_extm_0 : sw_data;
			break;

		case sw_extm_0:
			ck->header.timestamp = ch << 24;
			state = sw_extm_1;
			break;

		case sw_extm_1:
			ck->header.timestamp |= ch << 16;
			state = sw_extm_2;
			break;

		case sw_extm_2:
			ck->header.timestamp |= ch << 8;
			state = sw_extm_3;
			break;

		case sw_extm_3:
			ck->header.timestamp |= ch;
			state = sw_data;
			break;

		case sw_data:
			if (ck->payload.start == NULL) {
				ck->payload.start = stu_calloc(ck->header.message_len + 1);
				if (ck->payload.start == NULL) {
					return STU_ERROR;
				}

				ck->payload.pos = ck->payload.last = ck->payload.start;
				ck->payload.end = ck->payload.start + ck->header.message_len;
				ck->payload.size = ck->header.message_len;
			}

			n = stu_min(ck->payload.end - ck->payload.last, b->last - p);
			n = stu_min(n, r->nc.far_chunk_size - (ck->payload.last - ck->payload.start) % r->nc.far_chunk_size);

			ck->payload.last = stu_memcpy(ck->payload.last, p, n);
			p += n - 1;

			if ((ck->payload.last - ck->payload.start) % r->nc.far_chunk_size == 0) {
				state = sw_fmt;
			}

			if (ck->payload.last == ck->payload.end) {
				r->chunk_in = ck;
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

stu_int32_t
stu_rtmp_parse_url(stu_str_t *url, stu_str_t *dst, stu_uint8_t flag) {
	u_char *p, *last, c, ch;
	enum {
		sw_protocol,
		sw_schema,
		sw_schema_slash,
		sw_schema_slash_slash,
		sw_host_start,
		sw_host,
		sw_port_start,
		sw_port,
		sw_app_start,
		sw_app,
		sw_inst_start,
		sw_inst
	} state;

	state = sw_protocol;
	last = url->data + url->len;
	dst->data = url->data;

	for (p = url->data; p < last; p++) {
		ch = *p;

		switch (state) {
		case sw_protocol:
			c = (u_char) (ch | 0x20);
			if (c >= 'a' && c <= 'z') {
				break;
			}

			if (ch != ':') {
				return STU_ERROR;
			}

			if (flag & STU_RTMP_URL_FLAG_PROTOCOL) {
				dst->len = p - dst->data;
				goto done;
			}

			state = sw_schema;
			/* no break */

		case sw_schema:
			switch (ch) {
			case ':':
				state = sw_schema_slash;
				break;
			default:
				return STU_ERROR;
			}
			break;

		case sw_schema_slash:
			switch (ch) {
			case '/':
				state = sw_schema_slash_slash;
				break;
			default:
				return STU_ERROR;
			}
			break;

		case sw_schema_slash_slash:
			switch (ch) {
			case '/':
				state = sw_host_start;
				break;
			default:
				return STU_ERROR;
			}
			break;

		case sw_host_start:
			dst->data = p;
			state = sw_host;
			/* no break */

		case sw_host:
			c = (u_char) (ch | 0x20);
			if (c >= 'a' && c <= 'z') {
				break;
			}

			if ((ch >= '0' && ch <= '9') || ch == '.' || ch == '-') {
				break;
			}

			switch (ch) {
			case ':':
				state = sw_port_start;
				break;
			case '/':
				state = sw_app_start;
				break;
			default:
				return STU_ERROR;
			}

			if (flag & STU_RTMP_URL_FLAG_HOST) {
				dst->len = p - dst->data;
				goto done;
			}
			break;

		case sw_port_start:
			dst->data = p;
			state = sw_port;
			/* no break */

		case sw_port:
			if (ch >= '0' && ch <= '9') {
				break;
			}

			switch (ch) {
			case '/':
				state = sw_app_start;
				break;
			default:
				return STU_ERROR;
			}

			if (flag & STU_RTMP_URL_FLAG_PORT) {
				dst->len = p - dst->data;
				goto done;
			}
			break;

		case sw_app_start:
			dst->data = p;
			state = sw_app;
			/* no break */

		case sw_app:
			c = (u_char) (ch | 0x20);
			if (c >= 'a' && c <= 'z') {
				break;
			}

			if ((ch >= '0' && ch <= '9') || ch == '_') {
				break;
			}

			switch (ch) {
			case '/':
				state = sw_inst_start;
				break;
			default:
				return STU_ERROR;
			}

			if (flag & STU_RTMP_URL_FLAG_APP) {
				dst->len = p - dst->data;
				goto done;
			}
			break;

		case sw_inst_start:
			dst->data = p;
			state = sw_inst;
			/* no break */

		case sw_inst:
			dst->len = last - dst->data;
			goto done;
		}
	}

	return STU_AGAIN;

done:

	return STU_OK;
}
