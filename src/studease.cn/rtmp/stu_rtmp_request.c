/*
 * stu_rtmp_request.c
 *
 *  Created on: 2018年1月22日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

static void         stu_rtmp_process_request_chunk(stu_event_t *ev);
static ssize_t      stu_rtmp_read_request_chunk(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_request_message(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_filter_foreach_handler(stu_rtmp_request_t *r, stu_str_t *pattern, stu_list_t *list);
static void         stu_rtmp_run_phases(stu_rtmp_request_t *r);
static void         stu_rtmp_request_empty_handler(stu_rtmp_request_t *r);

static stu_int32_t  stu_rtmp_process_user_control(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_bandwidth(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_audio(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_video(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_data(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_aggregate(stu_rtmp_request_t *r);
static stu_rtmp_aggregate_body_t *
                    stu_rtmp_process_aggregate_body(stu_buf_t *b);

static stu_int32_t  stu_rtmp_process_command_connect(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_close(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_create_stream(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_result(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_error(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_play(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_play2(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_release_stream(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_delete_stream(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_close_stream(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_receive_audio(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_receive_video(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_fcpublish(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_publish(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_seek(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_pause(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command_on_status(stu_rtmp_request_t *r);

extern stu_hash_t  stu_rtmp_command_hash;
extern stu_hash_t  stu_rtmp_filter_hash;
extern stu_list_t  stu_rtmp_phases;

extern stu_str_t   stu_rtmp_definst;

stu_rtmp_command_t  rtmp_command[] = {
	{ stu_string("connect"),       stu_rtmp_process_command_connect },
	{ stu_string("close"),         stu_rtmp_process_command_close },
	{ stu_string("createStream"),  stu_rtmp_process_command_create_stream },
	{ stu_string("_result"),       stu_rtmp_process_command_result },
	{ stu_string("_error"),        stu_rtmp_process_command_error },

	{ stu_string("play"),          stu_rtmp_process_command_play },
	{ stu_string("play2"),         stu_rtmp_process_command_play2 },
	{ stu_string("releaseStream"), stu_rtmp_process_command_release_stream },
	{ stu_string("deleteStream"),  stu_rtmp_process_command_delete_stream },
	{ stu_string("closeStream"),   stu_rtmp_process_command_close_stream },
	{ stu_string("receiveAudio"),  stu_rtmp_process_command_receive_audio },
	{ stu_string("receiveVideo"),  stu_rtmp_process_command_receive_video },
	{ stu_string("FCPublish"),     stu_rtmp_process_command_fcpublish },
	{ stu_string("publish"),       stu_rtmp_process_command_publish },
	{ stu_string("seek"),          stu_rtmp_process_command_seek },
	{ stu_string("pause"),         stu_rtmp_process_command_pause },
	{ stu_string("onStatus"),      stu_rtmp_process_command_on_status },
	{ stu_null_string, NULL }
};


void
stu_rtmp_request_read_handler(stu_event_t *ev) {
	stu_connection_t *c;
	stu_int32_t       n, err;

	c = (stu_connection_t *) ev->data;

	//stu_mutex_lock(&c->lock);

	if (c->buffer.start == NULL) {
		c->buffer.start = (u_char *) stu_pcalloc(c->pool, STU_RTMP_REQUEST_DEFAULT_SIZE);
		c->buffer.pos = c->buffer.last = c->buffer.start;
		c->buffer.end = c->buffer.start + STU_RTMP_REQUEST_DEFAULT_SIZE;
		c->buffer.size = STU_RTMP_REQUEST_DEFAULT_SIZE;
	}
	c->buffer.pos = c->buffer.last = c->buffer.start;
	stu_memzero(c->buffer.start, c->buffer.size);

again:

	n = recv(c->fd, c->buffer.last, c->buffer.size, 0);
	if (n == -1) {
		err = stu_errno;
		if (err == EINTR) {
			stu_log_debug(3, "recv trying again: fd=%d, errno=%d.", c->fd, err);
			goto again;
		}

		if (err == EAGAIN) {
			stu_log_debug(3, "no data received: fd=%d, errno=%d.", c->fd, err);
			goto done;
		}

		stu_log_error(err, "Failed to recv data: fd=%d.", c->fd);
		goto failed;
	}

	if (n == 0) {
		stu_log_debug(4, "rtmp client has closed connection: fd=%d.", c->fd);
		goto failed;
	}

	c->buffer.last += n;
	stu_log_debug(4, "recv: fd=%d, bytes=%d.", c->fd, n);

	c->request = (void *) stu_rtmp_create_request(c);
	if (c->request == NULL) {
		stu_log_error(0, "Failed to create rtmp request.");
		goto failed;
	}

	//ev->handler = stu_rtmp_process_request_frames;
	stu_rtmp_process_request_chunk(ev);

	goto done;

failed:

	stu_rtmp_close_connection(c);

done:

	stu_log_debug(4, "rtmp request done.");

	//stu_mutex_unlock(&c->lock);
}

// TODO: memory leak while request is available
stu_rtmp_request_t *
stu_rtmp_create_request(stu_connection_t *c) {
	stu_rtmp_request_t *r;
	stu_str_t           s;

	if (c->request == NULL) {
		r = stu_pcalloc(c->pool, sizeof(stu_rtmp_request_t));
		if (r == NULL) {
			stu_log_error(0, "Failed to create rtmp request.");
			return NULL;
		}

		stu_queue_init(&r->chunks);

		r->nc.far_chunk_size = 128;
		r->nc.near_chunk_size = 128;
		r->nc.transaction_id = 0;
		r->nc.object_encoding = STU_RTMP_AMF0;

		s.data = stu_pcalloc(c->pool, 2);
		s.len = 1;
		*s.data = '/';

		r->nc.read_access = s;
		r->nc.write_access = s;
		r->nc.audio_sample_access = s;
		r->nc.video_sample_access = s;

		r->ns.nc = &r->nc;

		r->write_event_handler = stu_rtmp_request_write_handler;
	} else {
		r = c->request;
	}

	r->nc.connection = c;

	return r;
}

static void
stu_rtmp_process_request_chunk(stu_event_t *ev) {
	stu_connection_t   *c;
	stu_rtmp_request_t *r;
	ssize_t             n;
	stu_int32_t         rc;

	c = ev->data;
	r = c->request;

	stu_log_debug(4, "rtmp process request chunk.");

	if (ev->timedout) {
		stu_log_error(STU_ETIMEDOUT, "Failed to process rtmp request chunk.");

		c->timedout = TRUE;
		stu_rtmp_finalize_request(r, STU_ERROR);

		return;
	}

	rc = STU_DONE;

	for ( ;; ) {
		if (rc == STU_AGAIN) {
			n = stu_rtmp_read_request_chunk(r);
			if (n == STU_AGAIN) {
				return;
			}

			if (n == STU_ERROR) {
				stu_log_error(0, "rtmp failed to read request chunk.");
				stu_rtmp_finalize_request(r, STU_ERROR);
				return;
			}
		}

		rc = stu_rtmp_parse_chunk(r, &c->buffer);
		if (rc == STU_OK) {
			/* a complete message has been parsed successfully */
			stu_log_debug(4, "rtmp message parsed.");

			rc = stu_rtmp_process_request_message(r);
			if (rc != STU_OK) {
				stu_log_error(0, "rtmp failed to process request message.");
				stu_rtmp_finalize_request(r, STU_ERROR);
				return;
			}

			stu_rtmp_process_request(r);

			if (c->buffer.pos < c->buffer.last) {
				continue;
			}

			rc = STU_AGAIN;

			continue;
		}

		if (rc == STU_AGAIN) {
			/* a header line parsing is still not complete */
			continue;
		}

		stu_log_error(0, "Failed to process rtmp request chunk: %d.", rc);
		stu_rtmp_finalize_request(r, STU_ERROR);

		return;
	}
}

static ssize_t
stu_rtmp_read_request_chunk(stu_rtmp_request_t *r) {
	stu_connection_t *c;
	ssize_t           n;
	stu_int32_t       err;

	c = r->nc.connection;

	n = c->buffer.last - c->buffer.pos;
	if (n > 0) {
		/* buffer remains */
		return n;
	}

	c->buffer.pos = c->buffer.last = c->buffer.start;
	stu_memzero(c->buffer.start, c->buffer.size);

again:

	n = recv(c->fd, c->buffer.last, c->buffer.size, 0);
	if (n == -1) {
		err = stu_errno;
		if (err == EINTR) {
			stu_log_debug(4, "recv trying again: fd=%d, errno=%d.", c->fd, err);
			goto again;
		}

		if (err == EAGAIN) {
			stu_log_debug(4, "no data received: fd=%d, errno=%d.", c->fd, err);
			return STU_AGAIN;
		}
	}

	if (n == 0) {
		c->close = TRUE;
		stu_log_error(0, "rtmp client prematurely closed connection.");
	}

	if (n == 0 || n == STU_ERROR) {
		c->error = TRUE;
		return STU_ERROR;
	}

	c->buffer.last += n;

	return n;
}

static stu_int32_t
stu_rtmp_process_request_message(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *b;
	stu_uint32_t      n;
	stu_int32_t       rc;

	ck = r->chunk_in;
	b = &ck->payload;
	rc = STU_ERROR;

	stu_log_debug(4, "rtmp message: type=0x%02x.", ck->header.type);

	switch (ck->header.type) {
	case STU_RTMP_MSG_TYPE_SET_CHUNK_SIZE:
		if (b->size < 4) {
			stu_log_error(0, "Failed to process rtmp message: Data not enough.");
			goto failed;
		}

		r->nc.far_chunk_size = stu_endian_32(*(stu_uint32_t *) b->pos) & 0x7FFFFFFF;
		stu_log_debug(4, "rtmp set far_chunk_size: %d.", r->nc.far_chunk_size);
		break;

	case STU_RTMP_MSG_TYPE_ABORT:
		if (b->size < 4) {
			stu_log_error(0, "Failed to process rtmp message: Data not enough.");
			goto failed;
		}

		n = stu_endian_32(*(stu_uint32_t *) b->pos);
		stu_rtmp_free_chunk(r, n);
		stu_log_debug(4, "rtmp abort chunk stream: id=%d.", n);
		break;

	case STU_RTMP_MSG_TYPE_ACK:
		if (b->size < 4) {
			stu_log_error(0, "Failed to process rtmp message: Data not enough.");
			goto failed;
		}

		n = stu_endian_32(*(stu_uint32_t *) b->pos);
		stu_log_debug(4, "rtmp sequence number: %d, %2f\%.", n, n / r->nc.stat.bytes_out * 100);
		break;

	case STU_RTMP_MSG_TYPE_USER_CONTROL:
		if (stu_rtmp_process_user_control(r) == STU_ERROR) {
			stu_log_error(0, "Failed to process rtmp message: 0x%02X.", ck->header.type);
			goto failed;
		}
		break;

	case STU_RTMP_MSG_TYPE_ACK_WINDOW_SIZE:
		if (b->size < 4) {
			stu_log_error(0, "Failed to process rtmp message: Data not enough.");
			goto failed;
		}

		r->nc.far_ack_window_size = stu_endian_32(*(stu_uint32_t *) b->pos);
		stu_log_debug(4, "rtmp set far_ack_window_size: %d.", r->nc.far_ack_window_size);
		break;

	case STU_RTMP_MSG_TYPE_BANDWIDTH:
		if (stu_rtmp_process_bandwidth(r) == STU_ERROR) {
			stu_log_error(0, "Failed to process rtmp message: 0x%02X.", ck->header.type);
			goto failed;
		}
		break;

	case STU_RTMP_MSG_TYPE_EDGE:
		// TODO: message type edge
		break;

	case STU_RTMP_MSG_TYPE_AUDIO:
		if (stu_rtmp_process_audio(r) == STU_ERROR) {
			stu_log_error(0, "Failed to process rtmp message: 0x%02X.", ck->header.type);
			goto failed;
		}
		break;

	case STU_RTMP_MSG_TYPE_VIDEO:
		if (stu_rtmp_process_video(r) == STU_ERROR) {
			stu_log_error(0, "Failed to process rtmp message: 0x%02X.", ck->header.type);
			goto failed;
		}
		break;

	case STU_RTMP_MSG_TYPE_AMF3_DATA:
	case STU_RTMP_MSG_TYPE_DATA:
		if (stu_rtmp_process_data(r) == STU_ERROR) {
			stu_log_error(0, "Failed to process rtmp message: 0x%02X.", ck->header.type);
			goto failed;
		}
		break;

	case STU_RTMP_MSG_TYPE_AMF3_SHARED_OBJECT:
	case STU_RTMP_MSG_TYPE_SHARED_OBJECT:
		// TODO: message type shared message
		break;

	case STU_RTMP_MSG_TYPE_AMF3_COMMAND:
		b->pos++;
		/* no break */
	case STU_RTMP_MSG_TYPE_COMMAND:
		if (stu_rtmp_process_command(r) == STU_ERROR) {
			stu_log_error(0, "Failed to process rtmp message: 0x%02X.", ck->header.type);
			goto failed;
		}
		break;

	case STU_RTMP_MSG_TYPE_AGGREGATE:
		if (stu_rtmp_process_aggregate(r) == STU_ERROR) {
			stu_log_error(0, "Failed to process rtmp message: 0x%02X.", ck->header.type);
			goto failed;
		}
		break;

	default:
		break;
	}

	rc = STU_OK;

failed:

	return rc;
}

static stu_int32_t
stu_rtmp_process_user_control(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t                *ck;
	stu_buf_t                       *b;
	stu_rtmp_user_control_message_t  m;

	ck = r->chunk_in;
	b = &ck->payload;
	r->message = &m;

	m.payload.start = m.payload.pos = b->start;
	m.payload.end = m.payload.last = b->end;
	m.payload.size = b->size;

	stu_memzero(&m, sizeof(stu_rtmp_user_control_message_t));

	if (b->last - b->pos < 6) {
		stu_log_error(0, "Failed to parse rtmp message of user control: Data not enough.");
		return STU_ERROR;
	}

	// parse event
	m.type = stu_endian_16(*(stu_uint16_t *) b->pos);
	b->pos += 2;

	switch (m.type) {
	case STU_RTMP_EVENT_TYPE_SET_BUFFER_LENGTH:
		if (b->last - b->pos < 8) {
			stu_log_error(0, "Failed to parse rtmp message of user control: Data not enough.");
			return STU_ERROR;
		}

		b->pos += 4;
		m.buffer_len = stu_endian_32(*(stu_uint32_t *) b->pos);
		/* no break */

	case STU_RTMP_EVENT_TYPE_STREAM_BEGIN:
	case STU_RTMP_EVENT_TYPE_STREAM_EOF:
	case STU_RTMP_EVENT_TYPE_STREAM_DRY:
	case STU_RTMP_EVENT_TYPE_STREAM_IS_RECORDED:
		m.stream_id = stu_endian_32(*(stu_uint32_t *) b->pos - 4);
		b->pos += 4;
		break;

	case STU_RTMP_EVENT_TYPE_PING_REQUEST:
	case STU_RTMP_EVENT_TYPE_PING_RESPONSE:
		m.timestamp = stu_endian_32(*(stu_uint32_t *) b->pos);
		b->pos += 4;
		break;

	default:
		stu_log_error(0, "Failed to parse rtmp message of user control: Unknown event type %d.", m.type);
		return STU_ERROR;
	}

	// handle message
	switch (m.type) {
	case STU_RTMP_EVENT_TYPE_STREAM_BEGIN:
		stu_log_debug(4, "rtmp user control: stream begin, id=%d.", m.stream_id);
		break;

	case STU_RTMP_EVENT_TYPE_STREAM_EOF:
		stu_log_debug(4, "rtmp user control: stream eof, id=%d.", m.stream_id);
		break;

	case STU_RTMP_EVENT_TYPE_STREAM_DRY:
		stu_log_debug(4, "rtmp user control: stream dry, id=%d.", m.stream_id);
		break;

	case STU_RTMP_EVENT_TYPE_SET_BUFFER_LENGTH:
		stu_rtmp_on_set_buffer_length(&r->ns);
		stu_log_debug(4, "rtmp user control: set buffer length, id=%d, len=%d.", m.stream_id, m.buffer_len);
		break;

	case STU_RTMP_EVENT_TYPE_STREAM_IS_RECORDED:
		// TODO: set record flag
		stu_log_debug(4, "rtmp user control: stream is recorded, id=%d.", m.stream_id);
		break;

	case STU_RTMP_EVENT_TYPE_PING_REQUEST:
		stu_log_debug(4, "rtmp user control: ping request, timestamp=%d.", m.timestamp);
		break;

	case STU_RTMP_EVENT_TYPE_PING_RESPONSE:
		stu_log_debug(4, "rtmp user control: ping response, timestamp=%d.", m.timestamp);
		break;
	}

	return STU_OK;
}

static stu_int32_t
stu_rtmp_process_bandwidth(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t             *ck;
	stu_buf_t                    *b;
	stu_rtmp_bandwidth_message_t  m;

	ck = r->chunk_in;
	b = &ck->payload;
	r->message = &m;

	m.payload.start = m.payload.pos = b->start;
	m.payload.end = m.payload.last = b->end;
	m.payload.size = b->size;

	stu_memzero(&m, sizeof(stu_rtmp_bandwidth_message_t));

	if (b->last - b->pos < 5) {
		stu_log_error(0, "Failed to parse rtmp message of bandwidth: Data not enough.");
		return STU_ERROR;
	}

	m.bandwidth = stu_endian_32(*(stu_uint32_t *) b->pos);
	b->pos += 4;

	m.limit_type = *b->pos++;

	// handle message
	r->nc.near_bandwidth = m.bandwidth;
	r->nc.near_limit_type = m.limit_type;

	return STU_OK;
}

static stu_int32_t
stu_rtmp_process_audio(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t         *ck;
	stu_buf_t                *b;
	stu_rtmp_audio_message_t *m;

	ck = r->chunk_in;
	b = &ck->payload;

	m = stu_calloc(sizeof(stu_rtmp_audio_message_t));
	if (m == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of audio: Malloc failed.");
		return STU_ERROR;
	}

	r->message = m;

	m->payload.start = m->payload.pos = b->start;
	m->payload.end = m->payload.last = b->end;
	m->payload.size = b->size;

	if (b->last - b->pos < 2) {
		stu_log_error(0, "Failed to parse rtmp message of audio: Data not enough.");
		return STU_ERROR;
	}

	m->header.payload_len = ck->header.message_len - 11;

	m->format = (*b->pos >> 4) & 0x0F;
	m->sample_rate = (*b->pos >> 2) & 0x03;
	m->sample_size = (*b->pos >> 1) & 0x01;
	m->channels = *b->pos & 0x01;
	b->pos++;

	m->data_type = *b->pos++;

	// handle message
	stu_rtmp_on_audio_frame(&r->ns);

	return STU_OK;
}

static stu_int32_t
stu_rtmp_process_video(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t         *ck;
	stu_buf_t                *b;
	stu_rtmp_video_message_t *m;

	ck = r->chunk_in;
	b = &ck->payload;

	m = stu_calloc(sizeof(stu_rtmp_video_message_t));
	if (m == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of video: Malloc failed.");
		return STU_ERROR;
	}

	r->message = m;

	m->payload.start = m->payload.pos = b->start;
	m->payload.end = m->payload.last = b->end;
	m->payload.size = b->size;

	if (b->last - b->pos < 2) {
		stu_log_error(0, "Failed to parse rtmp message of video: Data not enough.");
		return STU_ERROR;
	}

	m->header.payload_len = ck->header.message_len - 11;

	m->frame_type = (*b->pos >> 4) & 0x0F;
	m->codec = *b->pos & 0x03;
	b->pos++;

	m->data_type = *b->pos++;

	// handle message
	stu_rtmp_on_video_frame(&r->ns);

	return STU_OK;
}

static stu_int32_t
stu_rtmp_process_data(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t        *ck;
	stu_buf_t               *b;
	stu_rtmp_amf_t          *v;
	stu_rtmp_data_message_t *m;
	stu_str_t                s;

	ck = r->chunk_in;
	b = &ck->payload;

	m = stu_calloc(sizeof(stu_rtmp_data_message_t));
	if (m == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of data: Malloc failed.");
		return STU_ERROR;
	}

	r->message = m;

	m->payload.start = m->payload.pos = b->start;
	m->payload.end = m->payload.last = b->end;
	m->payload.size = b->size;

	// handler
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of data: Bad AMF format[1].");
		return STU_ERROR;
	}

	s = *(stu_str_t *) v->value;
	b->pos += v->cost;

	m->handler.data = stu_calloc(s.len + 1);
	if (m->handler.data == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Malloc failed.");
		return STU_ERROR;
	}

	(void) stu_memcpy(m->handler.data, s.data, s.len);
	m->handler.len = s.len;

	stu_rtmp_amf_delete(v);

	// key
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of data: Bad AMF format[2].");
		return STU_ERROR;
	}

	s = *(stu_str_t *) v->value;
	b->pos += v->cost;

	m->key.data = stu_calloc(s.len + 1);
	if (m->key.data == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Malloc failed.");
		return STU_ERROR;
	}

	(void) stu_memcpy(m->key.data, s.data, s.len);
	m->key.len = s.len;

	stu_rtmp_amf_delete(v);

	// value
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	m->value = v;
	b->pos += v ? v->cost : 0;

	// handle message
	if (stu_strncmp(STU_RTMP_SET_DATA_FRAME.data, m->handler.data, m->handler.len) == 0) {
		stu_rtmp_on_set_data_frame(&r->ns);
		goto done;
	}

	if (stu_strncmp(STU_RTMP_CLEAR_DATA_FRAME.data, m->handler.data, m->handler.len) == 0) {
		stu_rtmp_on_clear_data_frame(&r->ns);
		goto done;
	}

done:

	// delete data frame on release

	return STU_OK;
}

static stu_int32_t
stu_rtmp_process_command(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_t         *command;
	stu_rtmp_command_message_t  m;
	stu_str_t                   dst;
	stu_uint32_t                hk;

	ck = r->chunk_in;
	b = &ck->payload;
	r->message = &m;

	m.payload.start = m.payload.pos = b->start;
	m.payload.end = m.payload.last = b->end;
	m.payload.size = b->size;

	stu_memzero(&m, sizeof(stu_rtmp_command_message_t));

	if (b->last - b->pos < 1) {
		stu_log_error(0, "Failed to parse rtmp message of command: Data not enough.");
		return STU_ERROR;
	}

	// command name
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[1].");
		return STU_ERROR;
	}

	dst = *(stu_str_t *) v->value;
	b->pos += v->cost;

	m.name.data = stu_calloc(dst.len + 1);
	if (m.name.data == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Malloc failed.");
		return STU_ERROR;
	}

	(void) stu_memcpy(m.name.data, dst.data, dst.len);
	m.name.len = dst.len;

	stu_rtmp_amf_delete(v);

	// transaction id
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[2].");
		return STU_ERROR;
	}

	m.transaction_id = *(stu_double_t *) v->value;
	b->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// process command
	hk = stu_hash_key(m.name.data, m.name.len, stu_rtmp_command_hash.flags);

	command = stu_hash_find_locked(&stu_rtmp_command_hash, hk, m.name.data, m.name.len);
	if (command == NULL) {
		stu_log_debug(4, "Unrecognized rtmp command: %s.", m.name.data);

		stu_free(m.name.data);
		stu_str_null(&m.name);

		return STU_OK; // Ignore command only.
	}

	return command->handler(r);
}

static stu_int32_t
stu_rtmp_process_command_connect(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v, *item;
	stu_str_t                  *val, key, dst;
	stu_rtmp_command_message_t *m;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[3].");
		return STU_ERROR;
	}

	stu_str_set(&key, "app");
	item = stu_rtmp_amf_get_object_item_by(v, &key);
	if (item && item->type == STU_RTMP_AMF_STRING) {
		val = (stu_str_t *) item->value;

		r->nc.app_name.len = val->len;
		r->nc.app_name.data = stu_pcalloc(r->nc.connection->pool, val->len + 1);
		stu_strncpy(r->nc.app_name.data, val->data, val->len);
	}

	stu_str_set(&key, "tcUrl");
	item = stu_rtmp_amf_get_object_item_by(v, &key);
	if (item && item->type == STU_RTMP_AMF_STRING) {
		val = (stu_str_t *) item->value;

		rc = stu_rtmp_parse_url(val, &dst, STU_RTMP_URL_FLAG_INST);
		if (rc == STU_ERROR) {
			stu_log_error(0, "Failed to parse rtmp instance name.");
			goto failed;
		}

		if (rc == STU_AGAIN) {
			dst = stu_rtmp_definst;
		}

		r->nc.inst_name.len = dst.len;
		r->nc.inst_name.data = stu_pcalloc(r->nc.connection->pool, dst.len + 1);
		stu_strncpy(r->nc.inst_name.data, dst.data, dst.len);
	}

	stu_str_set(&key, "objectEncoding");
	item = stu_rtmp_amf_get_object_item_by(v, &key);
	if (item && item->type == STU_RTMP_AMF_DOUBLE) {
		r->nc.object_encoding = *(stu_double_t *) item->value;
	}

	m->command_obj = v;
	b->pos += v->cost;

	// arguments
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	m->arguments = v;
	b->pos += v ? v->cost : 0;

	// handler
	if (stu_rtmp_on_connect(&r->nc) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);
	stu_rtmp_amf_delete(m->arguments);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_close(stu_rtmp_request_t *r) {
	stu_int32_t  rc;

	rc = STU_ERROR;

	// handler
	if (stu_rtmp_on_close(&r->nc) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_create_stream(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[4].");
		return STU_ERROR;
	}

	m->command_obj = v;
	b->pos += v->cost;

	// handler
	if (stu_rtmp_on_create_stream(&r->nc) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_result(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[5].");
		return STU_ERROR;
	}

	m->command_obj = v;
	b->pos += v->cost;

	// response
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[6].");
		goto failed;
	}

	m->response = v;
	b->pos += v->cost;

	// handler
	if (stu_rtmp_on_result(&r->nc) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);
	stu_rtmp_amf_delete(m->response);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_error(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[5].");
		return STU_ERROR;
	}

	m->command_obj = v;
	b->pos += v->cost;

	// response
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[6].");
		goto failed;
	}

	m->response = v;
	b->pos += v->cost;

	// handler
	if (stu_rtmp_on_error(&r->nc) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);
	stu_rtmp_amf_delete(m->response);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_play(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_str_t                   dst;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[7].");
		return STU_ERROR;
	}

	m->command_obj = v; // null
	b->pos += v->cost;

	// stream name
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[8].");
		goto failed;
	}

	dst = *(stu_str_t *) v->value;
	b->pos += v->cost;

	m->stream_name.data = stu_calloc(dst.len + 1);
	if (m->stream_name.data == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Malloc failed.");
		goto failed;
	}

	(void) stu_memcpy(m->stream_name.data, dst.data, dst.len);
	m->stream_name.len = dst.len;

	// start
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	m->start = v ? *(stu_double_t *) v->value : -2;
	b->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// duration
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	m->duration = v ? *(stu_double_t *) v->value : -1;
	b->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// reset
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	m->reset = v ? (stu_bool_t) v->value : TRUE;
	b->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	if (stu_rtmp_on_play(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);

	if (m->stream_name.data) {
		stu_free(m->stream_name.data);
		stu_str_null(&m->stream_name);
	}

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_play2(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[9].");
		return STU_ERROR;
	}

	m->command_obj = v; // null
	b->pos += v->cost;

	// arguments
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[10].");
		goto failed;
	}

	m->arguments = v;
	b->pos += v->cost;

	// handler
	if (stu_rtmp_on_play2(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);
	stu_rtmp_amf_delete(m->arguments);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_release_stream(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_str_t                   dst;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[11].");
		return STU_ERROR;
	}

	m->command_obj = v; // null
	b->pos += v->cost;

	// stream name
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[8].");
		goto failed;
	}

	dst = *(stu_str_t *) v->value;
	b->pos += v->cost;

	m->stream_name.data = stu_calloc(dst.len + 1);
	if (m->stream_name.data == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Malloc failed.");
		goto failed;
	}

	(void) stu_memcpy(m->stream_name.data, dst.data, dst.len);
	m->stream_name.len = dst.len;

	stu_rtmp_amf_delete(v);

	// handler
	if (stu_rtmp_on_delete_stream(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);

	if (m->stream_name.data) {
		stu_free(m->stream_name.data);
		stu_str_null(&m->stream_name);
	}

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_delete_stream(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[11].");
		return STU_ERROR;
	}

	m->command_obj = v; // null
	b->pos += v->cost;

	// stream id
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[12].");
		goto failed;
	}

	m->stream_id = *(stu_double_t *) v->value;
	b->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	if (stu_rtmp_on_delete_stream(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_close_stream(stu_rtmp_request_t *r) {
	stu_int32_t  rc;

	rc = STU_ERROR;

	// handler
	if (stu_rtmp_on_close_stream(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_receive_audio(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[13].");
		return STU_ERROR;
	}

	m->command_obj = v; // null
	b->pos += v->cost;

	// flag
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[14].");
		goto failed;
	}

	m->flag = (stu_bool_t) v->value;
	b->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	if (stu_rtmp_on_receive_audio(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_receive_video(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[13].");
		return STU_ERROR;
	}

	m->command_obj = v; // null
	b->pos += v->cost;

	// flag
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[14].");
		goto failed;
	}

	m->flag = (stu_bool_t) v->value;
	b->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	if (stu_rtmp_on_receive_video(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_fcpublish(stu_rtmp_request_t *r) {
	stu_int32_t  rc;

	rc = STU_ERROR;

	// handler
	if (stu_rtmp_on_fcpublish(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_publish(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_str_t                   dst;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[15].");
		return STU_ERROR;
	}

	m->command_obj = v; // null
	b->pos += v->cost;

	// publishing name
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[16].");
		goto failed;
	}

	dst = *(stu_str_t *) v->value;
	b->pos += v->cost;

	m->publishing_name.data = stu_calloc(dst.len + 1);
	if (m->publishing_name.data == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Malloc failed.");
		goto failed;
	}

	(void) stu_memcpy(m->publishing_name.data, dst.data, dst.len);
	m->publishing_name.len = dst.len;

	stu_rtmp_amf_delete(v);

	// publishing type
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[17].");
		goto failed;
	}

	dst = *(stu_str_t *) v->value;
	b->pos += v->cost;

	m->publishing_type.data = stu_calloc(dst.len + 1);
	if (m->publishing_type.data == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Malloc failed.");
		goto failed;
	}

	(void) stu_memcpy(m->publishing_type.data, dst.data, dst.len);
	m->publishing_type.len = dst.len;

	stu_rtmp_amf_delete(v);

	// handler
	if (stu_rtmp_on_publish(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);

	if (m->publishing_name.data) {
		stu_free(m->publishing_name.data);
		stu_str_null(&m->publishing_name);
	}

	if (m->publishing_type.data) {
		stu_free(m->publishing_type.data);
		stu_str_null(&m->publishing_type);
	}

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_seek(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[18].");
		return STU_ERROR;
	}

	m->command_obj = v; // null
	b->pos += v->cost;

	// milliseconds
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[19].");
		goto failed;
	}

	m->milliseconds = *(stu_double_t *) v->value;
	b->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	if (stu_rtmp_on_seek(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_pause(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[20].");
		return STU_ERROR;
	}

	m->command_obj = v; // null
	b->pos += v->cost;

	// pause
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[21].");
		goto failed;
	}

	m->pause = (stu_bool_t) v->value;
	b->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// milliseconds
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[22].");
		goto failed;
	}

	m->milliseconds = *(stu_double_t *) v->value;
	b->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	if (stu_rtmp_on_pause(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_on_status(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t           *ck;
	stu_buf_t                  *b;
	stu_rtmp_amf_t             *v;
	stu_rtmp_command_message_t *m;
	stu_int32_t                 rc;

	ck = r->chunk_in;
	b = &ck->payload;
	m = r->message;

	rc = STU_ERROR;

	// command object
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[23].");
		return STU_ERROR;
	}

	m->command_obj = v; // null
	b->pos += v->cost;

	// response
	v = stu_rtmp_amf_parse(b->pos, b->last - b->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[24].");
		goto failed;
	}

	m->response = v;
	b->pos += v->cost;

	// handler
	if (stu_rtmp_on_status(&r->ns) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_rtmp_amf_delete(m->command_obj);
	stu_rtmp_amf_delete(m->response);

	return rc;
}

static stu_int32_t
stu_rtmp_process_aggregate(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t             *ck;
	stu_buf_t                    *b;
	stu_queue_t                  *q;
	stu_rtmp_aggregate_body_t    *body;
	stu_rtmp_aggregate_message_t  m;

	ck = r->chunk_in;
	b = &ck->payload;
	r->message = &m;

	m.payload.start = m.payload.pos = b->start;
	m.payload.end = m.payload.last = b->end;
	m.payload.size = b->size;

	stu_memzero(&m, sizeof(stu_rtmp_aggregate_message_t));

	if (b->last - b->pos < 11) {
		stu_log_error(0, "Failed to parse rtmp message of aggregate: Data not enough.");
		return STU_ERROR;
	}

	m.header.type = *b->pos++;

	m.header.payload_len = *b->pos << 16 | *(b->pos + 1) << 8 | *(b->pos + 2);
	b->pos += 3;

	m.header.timestamp = stu_endian_32(*(stu_uint32_t *) b->pos);
	b->pos += 4;

	m.header.stream_id = *b->pos << 16 | *(b->pos + 1) << 8 | *(b->pos + 2);
	b->pos += 3;

	if (b->last - b->pos < m.header.payload_len) {
		stu_log_error(0, "Failed to parse rtmp message payload: Data not enough.");
		return STU_ERROR;
	}

	for ( ; b->pos < b->last; ) {
		body = stu_rtmp_process_aggregate_body(b);
		if (body == NULL) {
			stu_log_error(stu_errno, "Failed to parse rtmp message of aggregate body.");
			return STU_ERROR;
		}

		stu_queue_insert_tail(&m.body, &body->queue);
	}

	// TODO: handle message

	for (q = stu_queue_head(&m.body); q != stu_queue_sentinel(&m.body); q = stu_queue_next(q)) {
		body = stu_queue_data(q, stu_rtmp_aggregate_body_t, queue);
		stu_free(body);
	}

	return STU_OK;
}

static stu_rtmp_aggregate_body_t *
stu_rtmp_process_aggregate_body(stu_buf_t *b) {
	stu_rtmp_aggregate_body_t *body;

	if (b->last - b->pos < 11) {
		stu_log_error(0, "Failed to parse rtmp message of aggregate body: Data not enough.");
		return NULL;
	}

	body = stu_calloc(sizeof(stu_rtmp_aggregate_body_t));
	if (body == NULL) {
		stu_log_error(stu_errno, "Failed to calloc aggregate body.");
		return NULL;
	}

	body->payload.start = body->payload.pos = b->pos;

	body->header.type = *b->pos++;

	body->header.payload_len = *b->pos << 16 | *(b->pos + 1) << 8 | *(b->pos + 2);
	b->pos += 3;

	body->header.timestamp = stu_endian_32(*(stu_uint32_t *) b->pos);
	b->pos += 4;

	body->header.stream_id = *b->pos << 16 | *(b->pos + 1) << 8 | *(b->pos + 2);
	b->pos += 3;

	if (b->last - b->pos < body->header.payload_len) {
		stu_log_error(0, "Failed to parse rtmp message of aggregate body payload: Data not enough.");
		stu_free(body);
		return NULL;
	}

	body->payload.end = body->payload.last = b->pos;
	body->payload.size = body->payload.end - body->payload.start;

	body->size = body->payload.size;

	return STU_OK;
}

void
stu_rtmp_process_request(stu_rtmp_request_t *r) {
	stu_connection_t *c;
	stu_list_elt_t   *elts;
	stu_hash_elt_t   *e;
	stu_queue_t      *q;

	c = r->nc.connection;

	if (c->read.timer_set) {
		stu_timer_del(&c->read);
	}

	// TODO: use rwlock
	stu_mutex_lock(&stu_rtmp_filter_hash.lock);

	elts = &stu_rtmp_filter_hash.keys->elts;

	for (q = stu_queue_head(&elts->queue); q != stu_queue_sentinel(&elts->queue); q = stu_queue_next(q)) {
		e = stu_queue_data(q, stu_hash_elt_t, queue);

		if (stu_rtmp_filter_foreach_handler(r, &e->key, (stu_list_t *) e->value) == STU_OK) {
			goto done;
		}
	}

done:

	stu_mutex_unlock(&stu_rtmp_filter_hash.lock);

	r->write_event_handler(r);
}

static stu_int32_t
stu_rtmp_filter_foreach_handler(stu_rtmp_request_t *r, stu_str_t *pattern, stu_list_t *list) {
	stu_list_elt_t    *elts, *e;
	stu_queue_t       *q;
	stu_rtmp_filter_t *f;

	elts = &list->elts;

	for (q = stu_queue_tail(&elts->queue); q != stu_queue_sentinel(&elts->queue); q = stu_queue_prev(q)) {
		e = stu_queue_data(q, stu_list_elt_t, queue);
		f = (stu_rtmp_filter_t *) e->value;

		if (f && f->handler && f->handler(r) == STU_OK) {
			return STU_OK;
		}
	}

	return STU_DECLINED;
}

void
stu_rtmp_request_write_handler(stu_rtmp_request_t *r) {
	stu_connection_t *c;

	if (r == NULL) {
		stu_log_error(0, "something wrong here.");
		return;
	}

	c = r->nc.connection;
	c->timedout = FALSE;

	stu_log_debug(4, "rtmp run request.");

	stu_rtmp_finalize_request(r, r->status);
}

void
stu_rtmp_finalize_request(stu_rtmp_request_t *r, stu_int32_t rc) {
	stu_connection_t *c;

	c = r->nc.connection;

	stu_log_debug(4, "rtmp finalize request: %d.", rc);

	if (rc == STU_DONE) {
		stu_rtmp_close_request(r);
		return;
	}

	if (rc == STU_OK) {
		return;
	}

	if (rc == STU_DECLINED) {
		// TODO: response file system
		//r->write_event_handler = stu_rtmp_run_phases;
		stu_rtmp_run_phases(r);
		return;
	}

	if (rc == STU_ERROR || c->error) {
		stu_rtmp_close_request(r);
		return;
	}

	r->write_event_handler = stu_rtmp_request_empty_handler;

	stu_rtmp_close_request(r);
}

static void
stu_rtmp_run_phases(stu_rtmp_request_t *r) {
	stu_list_elt_t   *elts, *e;
	stu_queue_t      *q;
	stu_rtmp_phase_t *ph;

	elts = &stu_rtmp_phases.elts;

	for (q = stu_queue_head(&elts->queue); q != stu_queue_sentinel(&elts->queue); q = stu_queue_next(q)) {
		e = stu_queue_data(q, stu_list_elt_t, queue);
		ph = (stu_rtmp_phase_t *) e->value;

		if (ph && ph->handler && ph->handler(r) == STU_OK) {
			return;
		}
	}
}

static void
stu_rtmp_request_empty_handler(stu_rtmp_request_t *r) {
	stu_log_debug(4, "rtmp request empty handler.");
}


void
stu_rtmp_free_request(stu_rtmp_request_t *r) {
	r->nc.connection->request = NULL;
}

void
stu_rtmp_close_request(stu_rtmp_request_t *r) {
	stu_connection_t *c;

	c = r->nc.connection;

	stu_rtmp_free_request(r);
	stu_rtmp_close_connection(c);
}

void
stu_rtmp_close_connection(stu_connection_t *c) {
	stu_connection_close(c);
}
