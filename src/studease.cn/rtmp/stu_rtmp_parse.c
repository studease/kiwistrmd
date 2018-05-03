/*
 * stu_rtmp_parse.c
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

static stu_rtmp_chunk_t *stu_rtmp_get_uncomplete_chunk(stu_rtmp_request_t *r);


stu_int32_t
stu_rtmp_parse_handshaker(stu_rtmp_handshaker_t *h, stu_buf_t *src) {
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

	for (p = src->pos; p < src->last; p++) {
		ch = *p;

		switch (state) {
		case sw_c0_version:
			if (ch != STU_RTMP_VERSION_3) {
				stu_log_error(0, "Unexpected rtmp version: %d.", ch);
				break;
			}

			h->version = ch;
			state = sw_c1_time;
			break;

		case sw_c1_time:
			if (src->last - p < 4) {
				goto again;
			}

			h->start = p;
			h->time = stu_endian_32(*(stu_uint32_t *) p);
			p += 3;

			state = sw_c1_zero;
			break;

		case sw_c1_zero:
			if (src->last - p < 4) {
				goto again;
			}

			h->zero = stu_endian_32(*(stu_uint32_t *) p);
			p += 3;

			state = sw_c1_random;
			break;

		case sw_c1_random:
			if (src->last - p < STU_RTMP_HANDSHAKER_RANDOM_SIZE) {
				goto again;
			}

			h->random = p;

			if (h->type == STU_RTMP_HANDSHAKER_TYPE_CLIENT) {
				state = sw_c2_sent;
				break;
			}

			goto done;

		case sw_c2_sent:
			if (src->last - p < 4) {
				goto again;
			}

			h->start = p;
			h->time = stu_endian_32(*(stu_uint32_t *) p);
			p += 3;

			state = sw_c2_read;
			break;

		case sw_c2_read:
			if (src->last - p < 4) {
				goto again;
			}

			h->zero = stu_endian_32(*(stu_uint32_t *) p);
			p += 3;

			state = sw_c2_random;
			break;

		case sw_c2_random:
			if (src->last - p < STU_RTMP_HANDSHAKER_RANDOM_SIZE) {
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

	src->pos = p;
	h->state = state;

	return STU_AGAIN;

done:

	src->pos = p + 1;
	h->state = sw_c2_sent;

	return STU_OK;

hs_done:

	src->pos = p + 1;
	h->state = sw_done;

	return STU_DONE;
}


stu_int32_t
stu_rtmp_parse_chunk(stu_rtmp_request_t *r, stu_buf_t *src) {
	stu_rtmp_connection_t *nc;
	stu_rtmp_chunk_t      *ck;
	u_char                 ch;
	stu_uint32_t           n;
	enum {
		sw_fmt = 0,
		sw_csid_0,
		sw_csid_1,
		sw_timestamp_0,
		sw_timestamp_1,
		sw_timestamp_2,
		sw_msg_len_0,
		sw_msg_len_1,
		sw_msg_len_2,
		sw_msg_type,
		sw_stream_id_0,
		sw_stream_id_1,
		sw_stream_id_2,
		sw_stream_id_3,
		sw_ext_timestamp_0,
		sw_ext_timestamp_1,
		sw_ext_timestamp_2,
		sw_ext_timestamp_3,
		sw_data
	};

	nc = &r->connection;

	for (/* void */; src->pos < src->last; src->pos++) {
		ch = *src->pos;
		ck = r->chunk_in = stu_rtmp_get_uncomplete_chunk(r);

		switch (ck->state) {
		case sw_fmt:
			ck->fmt = ch >> 6;
			ck->csid = ch & 0x3F;

			switch (ck->csid) {
			case 0:
				ck->state = sw_csid_1;
				break;
			case 1:
				ck->state = sw_csid_0;
				break;
			default:
				if (ck->fmt == 3) {
					ck->state = ck->extended ? sw_ext_timestamp_0 : sw_data;
				} else {
					ck->state = sw_timestamp_0;
				}
				break;
			}
			break;

		case sw_csid_0:
			ck->csid = ch;
			ck->state = sw_csid_1;
			break;

		case sw_csid_1:
			if (ck->csid == 0) {
				ck->csid = ch;
			} else {
				ck->csid |= ch << 8;
			}
			ck->csid += 64;

			if (ck->fmt == 3) {
				ck->state = ck->extended ? sw_ext_timestamp_0 : sw_data;
			} else {
				ck->state = sw_timestamp_0;
			}
			break;

		case sw_timestamp_0:
			ck->timestamp = ch << 16;
			ck->state = sw_timestamp_1;
			break;

		case sw_timestamp_1:
			ck->timestamp |= ch << 8;
			ck->state = sw_timestamp_2;
			break;

		case sw_timestamp_2:
			ck->timestamp |= ch;
			if (ck->timestamp == 0xFFFFFF) {
				ck->extended = TRUE;
			}

			if (ck->fmt == 2) {
				ck->state = ck->extended ? sw_ext_timestamp_0 : sw_data;
			} else {
				ck->state = sw_msg_len_0;
			}
			break;

		case sw_msg_len_0:
			ck->payload.size = ch << 16;
			ck->state = sw_msg_len_1;
			break;

		case sw_msg_len_1:
			ck->payload.size |= ch << 8;
			ck->state = sw_msg_len_2;
			break;

		case sw_msg_len_2:
			ck->payload.size |= ch;
			ck->state = sw_msg_type;
			break;

		case sw_msg_type:
			ck->type_id = ch;

			if (ck->fmt == 1) {
				ck->state = ck->extended ? sw_ext_timestamp_0 : sw_data;
			} else {
				ck->state = sw_stream_id_0;
			}
			break;

		case sw_stream_id_0:
			ck->stream_id = ch;
			ck->state = sw_stream_id_1;
			break;

		case sw_stream_id_1:
			ck->stream_id |= ch << 8;
			ck->state = sw_stream_id_2;
			break;

		case sw_stream_id_2:
			ck->stream_id |= ch << 16;
			ck->state = sw_stream_id_3;
			break;

		case sw_stream_id_3:
			ck->stream_id |= ch << 24;
			ck->state = ck->extended ? sw_ext_timestamp_0 : sw_data;
			break;

		case sw_ext_timestamp_0:
			ck->timestamp = ch << 24;
			ck->state = sw_ext_timestamp_1;
			break;

		case sw_ext_timestamp_1:
			ck->timestamp |= ch << 16;
			ck->state = sw_ext_timestamp_2;
			break;

		case sw_ext_timestamp_2:
			ck->timestamp |= ch << 8;
			ck->state = sw_ext_timestamp_3;
			break;

		case sw_ext_timestamp_3:
			ck->timestamp |= ch;
			ck->state = sw_data;
			break;

		case sw_data:
			if (ck->payload.start == NULL) {
				ck->payload.start = (u_char *) stu_calloc(ck->payload.size);
				if (ck->payload.start == NULL) {
					return STU_ERROR;
				}

				ck->payload.pos = ck->payload.last = ck->payload.start;
				ck->payload.end = ck->payload.start + ck->payload.size;
			}

			n = stu_min(ck->payload.end - ck->payload.last, src->last - src->pos);
			n = stu_min(n, nc->far_chunk_size - (ck->payload.last - ck->payload.start) % nc->far_chunk_size);

			ck->payload.last = stu_memcpy(ck->payload.last, src->pos, n);
			src->pos += n - 1;

			if ((ck->payload.last - ck->payload.start) % nc->far_chunk_size == 0) {
				ck->state = sw_fmt;
			}

			if (ck->payload.last == ck->payload.end) {
				src->pos++;
				ck->state = sw_fmt;
				return STU_OK;
			}
			break;

		default:
			stu_log_error(0, "Unknown rtmp chunk state: %d.", ck->state);
			return STU_ERROR;
		}
	}

	return STU_AGAIN;
}

static stu_rtmp_chunk_t *
stu_rtmp_get_uncomplete_chunk(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_queue_t      *q;

	for (q = stu_queue_tail(&r->chunks); q != stu_queue_sentinel(&r->chunks); q = stu_queue_prev(q)) {
		ck = stu_queue_data(q, stu_rtmp_chunk_t, queue);

		if (ck->state || ck->payload.start == NULL || ck->payload.last != ck->payload.end) {
			goto done;
		}
	}

	ck = stu_calloc(sizeof(stu_rtmp_chunk_t));
	if (ck) {
		stu_queue_insert_tail(&r->chunks, &ck->queue);
	}

done:

	return ck;
}


stu_int32_t
stu_rtmp_parse_url(stu_rtmp_url_t *url, u_char *src, size_t len) {
	u_char *pos, *last, *port, ch, c;
	enum {
		sw_start,
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

	state = sw_start;
	pos = src;
	last = pos + len;

	url->data = src;
	url->len = len;

	for (/* void */; pos < last; pos++) {
		ch = *pos;

		switch (state) {
		case sw_start:
			url->protocol.data = pos;
			state = sw_protocol;
			/* no break */

		case sw_protocol:
			c = (u_char) (ch | 0x20);
			if (c >= 'a' && c <= 'z') {
				break;
			}

			url->protocol.len = pos - url->protocol.data;
			state = sw_schema;

			if (ch != ':') {
				return STU_ERROR;
			}
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
			url->host.data = pos;
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

			url->host.len = pos - url->host.data;

			switch (ch) {
			case ':':
				state = sw_port_start;
				break;
			case '/':
				url->port = 1935;
				state = sw_app_start;
				break;
			default:
				return STU_ERROR;
			}
			break;

		case sw_port_start:
			port = pos;
			state = sw_port;
			/* no break */

		case sw_port:
			if (ch >= '0' && ch <= '9') {
				break;
			}

			url->port = atoi((const char *) port);

			switch (ch) {
			case '/':
				state = sw_app_start;
				break;
			default:
				return STU_ERROR;
			}
			break;

		case sw_app_start:
			url->application.data = pos;
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

			url->application.len = pos - url->application.data;

			switch (ch) {
			case '/':
				state = sw_inst_start;
				break;
			default:
				return STU_ERROR;
			}
			break;

		case sw_inst_start:
			url->instance.data = pos;
			state = sw_inst;
			/* no break */

		case sw_inst:
			url->instance.len = last - url->instance.data;
			return STU_OK;

		default:
			break;
		}
	}

	switch (state) {
	case sw_protocol:
		url->protocol.len = pos - url->protocol.data;
		break;

	case sw_host:
		url->host.len = pos - url->host.data;
		break;

	case sw_port:
		url->port = atoi((const char *) port);
		break;

	case sw_app:
		url->application.len = pos - url->application.data;
		/* no break */

	case sw_inst:
		if (url->instance.data) {
			url->instance.len = last - url->instance.data;
		} else {
			url->instance = stu_rtmp_definst;
		}
		break;

	default:
		break;
	}

	return STU_OK;
}
