/*
 * stu_rtmp_request.c
 *
 *  Created on: 2018骞�1鏈�22鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

static void         stu_rtmp_process_chunk(stu_event_t *ev);
static ssize_t      stu_rtmp_read_request_chunk(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_message(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_filter_foreach_handler(stu_rtmp_request_t *r, stu_str_t *pattern, stu_list_t *list);
static void         stu_rtmp_run_phases(stu_rtmp_request_t *r);
static void         stu_rtmp_request_empty_handler(stu_rtmp_request_t *r);

static stu_int32_t  stu_rtmp_process_set_chunk_size(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_abort(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_ack(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_user_control(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_ack_window_size(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_bandwidth(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_edge(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_audio(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_video(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_data(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_shared_object(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_command(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_process_aggregate(stu_rtmp_request_t *r);

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

static stu_int32_t  stu_rtmp_on_set_chunk_size(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_abort(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_ack(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_user_control(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_ack_window_size(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_bandwidth(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_edge(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_audio(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_video(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_data(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_shared_object(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_command(stu_rtmp_request_t *r);

static stu_int32_t  stu_rtmp_on_set_data_frame(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_clear_data_frame(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_on_metadata(stu_rtmp_request_t *r);

extern stu_hash_t  stu_rtmp_filter_hash;
extern stu_list_t  stu_rtmp_phases;

extern stu_hash_t  stu_rtmp_message_listener_hash;
extern stu_hash_t  stu_rtmp_command_listener_hash;
extern stu_hash_t  stu_rtmp_data_listener_hash;

extern const stu_str_t  STU_RTMP_KEY_TC_URL;
extern const stu_str_t  STU_RTMP_KEY_OBJECT_ENCODING;

stu_rtmp_message_listener_t  stu_rtmp_message_listeners[] = {
	{ STU_RTMP_MESSAGE_TYPE_SET_CHUNK_SIZE,     stu_rtmp_process_set_chunk_size },
	{ STU_RTMP_MESSAGE_TYPE_ABORT,              stu_rtmp_process_abort },
	{ STU_RTMP_MESSAGE_TYPE_ACK,                stu_rtmp_process_ack },
	{ STU_RTMP_MESSAGE_TYPE_USER_CONTROL,       stu_rtmp_process_user_control },
	{ STU_RTMP_MESSAGE_TYPE_ACK_WINDOW_SIZE,    stu_rtmp_process_ack_window_size },
	{ STU_RTMP_MESSAGE_TYPE_BANDWIDTH,          stu_rtmp_process_bandwidth },
	{ STU_RTMP_MESSAGE_TYPE_EDGE,               stu_rtmp_process_edge },
	{ STU_RTMP_MESSAGE_TYPE_AUDIO,              stu_rtmp_process_audio },
	{ STU_RTMP_MESSAGE_TYPE_VIDEO,              stu_rtmp_process_video },
	{ STU_RTMP_MESSAGE_TYPE_AMF3_DATA,          stu_rtmp_process_data },
	{ STU_RTMP_MESSAGE_TYPE_AMF3_SHARED_OBJECT, stu_rtmp_process_shared_object },
	{ STU_RTMP_MESSAGE_TYPE_AMF3_COMMAND,       stu_rtmp_process_command },
	{ STU_RTMP_MESSAGE_TYPE_DATA,               stu_rtmp_process_data },
	{ STU_RTMP_MESSAGE_TYPE_SHARED_OBJECT,      stu_rtmp_process_shared_object },
	{ STU_RTMP_MESSAGE_TYPE_COMMAND,            stu_rtmp_process_command },
	{ STU_RTMP_MESSAGE_TYPE_AGGREGATE,          stu_rtmp_process_aggregate },
	{ 0, NULL }
};

stu_rtmp_command_listener_t  stu_rtmp_command_listeners[] = {
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

stu_rtmp_data_listener_t  stu_rtmp_data_listeners[] = {
	{ stu_string("@setDataFrame"),   stu_rtmp_on_set_data_frame },
	{ stu_string("@clearDataFrame"), stu_rtmp_on_clear_data_frame },
	{ stu_string("onMetaData"),      stu_rtmp_on_metadata },
	{ stu_null_string, NULL }
};


void
stu_rtmp_request_read_handler(stu_event_t *ev) {
	stu_connection_t   *c;
	stu_rtmp_request_t *r;
	stu_int32_t         n;

	c = (stu_connection_t *) ev->data;

	//stu_mutex_lock(&c->lock);

	if (c->buffer.start == NULL) {
		c->buffer.start = (u_char *) stu_pcalloc(c->pool, STU_RTMP_REQUEST_DEFAULT_SIZE);
		c->buffer.pos = c->buffer.last = c->buffer.start;
		c->buffer.end = c->buffer.start + STU_RTMP_REQUEST_DEFAULT_SIZE;
		c->buffer.size = STU_RTMP_REQUEST_DEFAULT_SIZE;
	}

	if (c->buffer.end == c->buffer.last) {
		c->buffer.pos = c->buffer.last = c->buffer.start;
		stu_memzero(c->buffer.start, c->buffer.size);
	}

	n = c->recv(c, c->buffer.last, c->buffer.end - c->buffer.last);
	if (n == STU_AGAIN) {
		goto done;
	}

	if (n == STU_ERROR) {
		c->error = TRUE;
		goto failed;
	}

	if (n == 0) {
		stu_log_error(0, "rtmp remote peer prematurely closed connection.");
		c->close = TRUE;
		goto failed;
	}

	c->buffer.last += n;
	stu_log_debug(4, "recv: fd=%d, bytes=%d.", c->fd, n);

	c->request = (void *) stu_rtmp_create_request(c);
	if (c->request == NULL) {
		stu_log_error(0, "Failed to create rtmp request.");
		goto failed;
	}

	//ev->handler = stu_rtmp_process_chunk;
	stu_rtmp_process_chunk(ev);

	goto done;

failed:

	r = c->request;
	if (r) {
		stu_rtmp_close_connection(&r->connection);
	} else {
		stu_connection_close(c);
	}

done:

	stu_log_debug(4, "rtmp request done.");

	//stu_mutex_unlock(&c->lock);
}

// TODO: memory leak while request is available
stu_rtmp_request_t *
stu_rtmp_create_request(stu_connection_t *c) {
	stu_rtmp_request_t *r;

	if (c->request == NULL) {
		r = stu_pcalloc(c->pool, sizeof(stu_rtmp_request_t));
		if (r == NULL) {
			stu_log_error(0, "Failed to create rtmp request: fd=%d.", c->fd);
			return NULL;
		}

		stu_rtmp_connection_init(&r->connection, c);
		stu_queue_init(&r->chunks);
	} else {
		r = c->request;
	}

	return r;
}

static void
stu_rtmp_process_chunk(stu_event_t *ev) {
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
			stu_log_debug(4, "rtmp message parsed.");

			rc = stu_rtmp_process_message(r);
			if (rc != STU_OK) {
				stu_log_error(0, "rtmp failed to process request message.");
				stu_rtmp_finalize_request(r, STU_ERROR);
				return;
			}

			stu_rtmp_process_request(r);

			if (c->buffer.pos == c->buffer.last) {
				c->buffer.pos = c->buffer.last = c->buffer.start;
				rc = STU_AGAIN;
			}

			continue;
		}

		if (rc == STU_AGAIN) {
			stu_log_debug(4, "rtmp message parsing is still not complete.");
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

	c = r->connection.conn;

	if (c->buffer.end == c->buffer.last) {
		c->buffer.pos = c->buffer.last = c->buffer.start;
		stu_memzero(c->buffer.start, c->buffer.size);
	}

	n = c->recv(c, c->buffer.last, c->buffer.end - c->buffer.last);
	if (n == STU_AGAIN) {
		return STU_AGAIN;
	}

	if (n == STU_ERROR) {
		c->error = TRUE;
		return STU_ERROR;
	}

	if (n == 0) {
		stu_log_error(0, "rtmp remote peer prematurely closed connection.");
		c->close = TRUE;
		return STU_ERROR;
	}

	c->buffer.last += n;
	stu_log_debug(4, "recv: fd=%d, bytes=%d.", c->fd, n);

	return n;
}

static stu_int32_t
stu_rtmp_process_message(stu_rtmp_request_t *r) {
	stu_rtmp_message_listener_t *l;
	stu_rtmp_chunk_t            *ck;
	u_char                       tmp[3];
	stu_str_t                    key;
	stu_uint32_t                 hk;

	ck = r->chunk_in;
	stu_memzero(tmp, 3);

	stu_log_debug(4, "rtmp message: type=0x%02X.", ck->type_id);

	key.data = tmp;
	key.len = stu_sprintf(tmp, "%u", ck->type_id) - tmp;

	hk = stu_hash_key(key.data, key.len, stu_rtmp_message_listener_hash.flags);

	l = stu_hash_find_locked(&stu_rtmp_message_listener_hash, hk, key.data, key.len);
	if (l && l->handler) {
		l->handler(r);
	}

	return STU_OK;
}


static stu_int32_t
stu_rtmp_process_set_chunk_size(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;

	ck = r->chunk_in;
	buf = &ck->payload;

	if (buf->last - buf->pos < 4) {
		stu_log_error(0, "Failed to process rtmp message of set chunk size: Data not enough.");
		return STU_ERROR;
	}

	r->chunk_size = stu_endian_32(*(stu_uint32_t *) buf->pos) & 0x7FFFFFFF;

	return stu_rtmp_on_set_chunk_size(r);
}

static stu_int32_t
stu_rtmp_process_abort(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;

	ck = r->chunk_in;
	buf = &ck->payload;

	if (buf->last - buf->pos < 4) {
		stu_log_error(0, "Failed to process rtmp message of abort: Data not enough.");
		return STU_ERROR;
	}

	r->chunk_stream_id = stu_endian_32(*(stu_uint32_t *) buf->pos);

	return stu_rtmp_on_abort(r);
}

static stu_int32_t
stu_rtmp_process_ack(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;

	ck = r->chunk_in;
	buf = &ck->payload;

	if (buf->last - buf->pos < 4) {
		stu_log_error(0, "Failed to process rtmp message of ack: Data not enough.");
		return STU_ERROR;
	}

	r->ack = stu_endian_32(*(stu_uint32_t *) buf->pos);

	return stu_rtmp_on_ack(r);
}

static stu_int32_t
stu_rtmp_process_user_control(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;

	ck = r->chunk_in;
	buf = &ck->payload;

	if (buf->last - buf->pos < 6) {
		stu_log_error(0, "Failed to parse rtmp message of user control: Data not enough.");
		return STU_ERROR;
	}

	r->event_type = stu_endian_16(*(stu_uint16_t *) buf->pos);
	buf->pos += 2;

	switch (r->event_type) {
	case STU_RTMP_EVENT_TYPE_STREAM_BEGIN:
	case STU_RTMP_EVENT_TYPE_STREAM_EOF:
	case STU_RTMP_EVENT_TYPE_STREAM_DRY:
	case STU_RTMP_EVENT_TYPE_STREAM_IS_RECORDED:
		r->stream_id = stu_endian_32(*(stu_uint32_t *) buf->pos);
		buf->pos += 4;
		break;

	case STU_RTMP_EVENT_TYPE_SET_BUFFER_LENGTH:
		if (buf->last - buf->pos < 8) {
			stu_log_error(0, "Failed to parse rtmp message of user control: Data not enough.");
			return STU_ERROR;
		}

		r->stream_id = stu_endian_32(*(stu_uint32_t *) buf->pos);
		buf->pos += 4;

		r->buffer_length = stu_endian_32(*(stu_uint32_t *) buf->pos);
		buf->pos += 4;
		break;

	case STU_RTMP_EVENT_TYPE_PING_REQUEST:
	case STU_RTMP_EVENT_TYPE_PING_RESPONSE:
		r->timestamp = stu_endian_32(*(stu_uint32_t *) buf->pos);
		buf->pos += 4;
		break;

	default:
		stu_log_error(0, "Failed to parse rtmp message of user control: Unknown event type %d.", r->event_type);
		return STU_ERROR;
	}

	return stu_rtmp_on_user_control(r);
}

static stu_int32_t
stu_rtmp_process_ack_window_size(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;

	ck = r->chunk_in;
	buf = &ck->payload;

	if (buf->last - buf->pos < 4) {
		stu_log_error(0, "Failed to process rtmp message of ack window size: Data not enough.");
		return STU_ERROR;
	}

	r->ack_window_size = stu_endian_32(*(stu_uint32_t *) buf->pos);

	return stu_rtmp_on_ack_window_size(r);
}

static stu_int32_t
stu_rtmp_process_bandwidth(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;

	ck = r->chunk_in;
	buf = &ck->payload;

	if (buf->last - buf->pos < 5) {
		stu_log_error(0, "Failed to parse rtmp message of bandwidth: Data not enough.");
		return STU_ERROR;
	}

	r->bandwidth = stu_endian_32(*(stu_uint32_t *) buf->pos);
	buf->pos += 4;

	r->limit_type = *buf->pos++;

	return stu_rtmp_on_bandwidth(r);
}

static stu_int32_t
stu_rtmp_process_edge(stu_rtmp_request_t *r) {
	return stu_rtmp_on_edge(r);
}

static stu_int32_t
stu_rtmp_process_audio(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	u_char           *pos;

	ck = r->chunk_in;
	buf = &ck->payload;
	pos = buf->pos;

	if (buf->last - buf->pos < 2) {
		stu_log_error(0, "Failed to parse rtmp message of audio: Data not enough.");
		return STU_ERROR;
	}

	r->format = (*pos >> 4) & 0x0F;
	r->sample_rate = (*pos >> 2) & 0x03;
	r->sample_size = (*pos >> 1) & 0x01;
	r->channels = *pos & 0x01;
	pos++;

	r->data_type = *pos++;

	return stu_rtmp_on_audio(r);
}

static stu_int32_t
stu_rtmp_process_video(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	u_char           *pos;

	ck = r->chunk_in;
	buf = &ck->payload;
	pos = buf->pos;

	if (buf->last - buf->pos < 2) {
		stu_log_error(0, "Failed to parse rtmp message of video: Data not enough.");
		return STU_ERROR;
	}

	r->frame_type = (*pos >> 4) & 0x0F;
	r->codec = *pos & 0x03;
	pos++;

	r->data_type = *pos++;

	return stu_rtmp_on_video(r);
}

static stu_int32_t
stu_rtmp_process_data(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v, *ai_hdlr, *ai_key;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// handler
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of data: Bad AMF format[1].");
		return STU_ERROR;
	}

	r->data_handler = (stu_str_t *) v->value;
	buf->pos += v->cost;

	ai_hdlr = v;

	if (stu_strncmp(STU_RTMP_SET_DATA_FRAME.data, r->data_handler->data, r->data_handler->len) == 0 ||
			stu_strncmp(STU_RTMP_CLEAR_DATA_FRAME.data, r->data_handler->data, r->data_handler->len) == 0) {
		r->data_key = NULL;
		ai_key = NULL;
		goto value;
	}

	// key
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of data: Bad AMF format[2].");
		goto done;
	}

	r->data_key = (stu_str_t *) v->value;
	buf->pos += v->cost;

	ai_key = v;

value:

	// value
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	buf->pos += v ? v->cost : 0;

	r->data_value = v;

	// handler
	rc = stu_rtmp_on_data(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp data: handler=%s, key=%s.",
				r->data_handler->data, r->data_key ? r->data_key->data : NULL);
	}

done:

	stu_rtmp_amf_delete(ai_hdlr);
	stu_rtmp_amf_delete(ai_key);
	stu_rtmp_amf_delete(r->data_value);

	return rc;
}

static stu_int32_t
stu_rtmp_process_shared_object(stu_rtmp_request_t *r) {
	return stu_rtmp_on_shared_object(r);
}

static stu_int32_t
stu_rtmp_process_command(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v, *ai_cmd;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	rc = STU_ERROR;

	if (buf->last - buf->pos < 7) {
		stu_log_error(0, "Failed to parse rtmp message of command: Data not enough.");
		return STU_ERROR;
	}

	// command name
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[1].");
		return STU_ERROR;
	}

	r->command = (stu_str_t *) v->value;
	buf->pos += v->cost;

	ai_cmd = v;

	// transaction id
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[2].");
		goto done;
	}

	r->transaction_id = *(stu_double_t *) v->value;
	buf->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	rc = stu_rtmp_on_command(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

done:

	stu_rtmp_amf_delete(ai_cmd);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_connect(stu_rtmp_request_t *r) {
	stu_rtmp_connection_t *nc;
	stu_rtmp_chunk_t      *ck;
	stu_buf_t             *buf;
	stu_rtmp_amf_t        *v, *item;
	stu_str_t             *str;
	stu_int32_t            rc;

	nc = &r->connection;
	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp message of command: Bad AMF format[3].");
		return STU_ERROR;
	}

	item = stu_rtmp_amf_get_object_item_by(v, (stu_str_t *) &STU_RTMP_KEY_TC_URL);
	if (item && item->type == STU_RTMP_AMF_STRING) {
		str = (stu_str_t *) item->value;

		nc->url.len = str->len;
		nc->url.data = stu_pcalloc(nc->conn->pool, str->len + 1);
		stu_strncpy(nc->url.data, str->data, str->len);

		rc = stu_rtmp_parse_url(&nc->url, nc->url.data, nc->url.len);
		if (rc == STU_ERROR) {
			stu_log_error(0, "Failed to parse rtmp url.");
			goto failed;
		}
	}

	item = stu_rtmp_amf_get_object_item_by(v, (stu_str_t *) &STU_RTMP_KEY_OBJECT_ENCODING);
	if (item && item->type == STU_RTMP_AMF_DOUBLE) {
		nc->object_encoding = *(stu_double_t *) item->value;
	}

	r->command_obj = v;
	buf->pos += v->cost;

	// arguments
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);

	r->arguments = v;
	buf->pos += v ? v->cost : 0;

	// handler
	rc = stu_rtmp_on_connect(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

failed:

	stu_rtmp_amf_delete(r->command_obj);
	stu_rtmp_amf_delete(r->arguments);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_close(stu_rtmp_request_t *r) {
	stu_int32_t  rc;

	// handler
	rc = stu_rtmp_on_close(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_create_stream(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v;
	buf->pos += v->cost;

	// handler
	rc = stu_rtmp_on_create_stream(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

	stu_rtmp_amf_delete(r->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_result(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v;
	buf->pos += v->cost;

	// response
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->arguments = v;
	buf->pos += v->cost;

	// handler
	rc = stu_rtmp_on_result(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

failed:

	stu_rtmp_amf_delete(r->command_obj);
	stu_rtmp_amf_delete(r->arguments);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_error(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v;
	buf->pos += v->cost;

	// response
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->arguments = v;
	buf->pos += v->cost;

	// handler
	rc = stu_rtmp_on_error(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

failed:

	stu_rtmp_amf_delete(r->command_obj);
	stu_rtmp_amf_delete(r->arguments);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_play(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v, *ai_name;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v; // null
	buf->pos += v->cost;

	// stream name
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->stream_name = (stu_str_t *) v->value;
	buf->pos += v->cost;

	ai_name = v;

	// start
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);

	r->start = v ? *(stu_double_t *) v->value : -2;
	buf->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// duration
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);

	r->duration = v ? *(stu_double_t *) v->value : -1;
	buf->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// reset
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);

	r->reset = v ? (stu_bool_t) v->value : TRUE;
	buf->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	rc = stu_rtmp_on_play(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

	stu_rtmp_amf_delete(ai_name);

failed:

	stu_rtmp_amf_delete(r->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_play2(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v; // null
	buf->pos += v->cost;

	// parameters
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->arguments = v;
	buf->pos += v->cost;

	// handler
	rc = stu_rtmp_on_play2(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

failed:

	stu_rtmp_amf_delete(r->command_obj);
	stu_rtmp_amf_delete(r->arguments);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_release_stream(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v, *ai_name;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v; // null
	buf->pos += v->cost;

	// stream name
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->stream_name = (stu_str_t *) v->value;
	buf->pos += v->cost;

	ai_name = v;

	// handler
	rc = stu_rtmp_on_release_stream(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

	stu_rtmp_amf_delete(ai_name);

failed:

	stu_rtmp_amf_delete(r->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_delete_stream(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v; // null
	buf->pos += v->cost;

	// stream id
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->stream_id = *(stu_double_t *) v->value;
	buf->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	rc = stu_rtmp_on_delete_stream(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

failed:

	stu_rtmp_amf_delete(r->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_close_stream(stu_rtmp_request_t *r) {
	stu_int32_t  rc;

	// handler
	rc = stu_rtmp_on_close_stream(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_receive_audio(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v; // null
	buf->pos += v->cost;

	// flag
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->flag = (stu_bool_t) v->value;
	buf->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	rc = stu_rtmp_on_receive_audio(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

failed:

	stu_rtmp_amf_delete(r->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_receive_video(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v; // null
	buf->pos += v->cost;

	// flag
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->flag = (stu_bool_t) v->value;
	buf->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	rc = stu_rtmp_on_receive_video(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

failed:

	stu_rtmp_amf_delete(r->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_fcpublish(stu_rtmp_request_t *r) {
	stu_int32_t  rc;

	// handler
	rc = stu_rtmp_on_fcpublish(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_publish(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v, *ai_name, *ai_type;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v; // null
	buf->pos += v->cost;

	// publishing name
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->publishing_name = (stu_str_t *) v->value;
	buf->pos += v->cost;

	ai_name = v;

	// publishing type
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->publishing_type = (stu_str_t *) v->value;
	buf->pos += v->cost;

	ai_type = v;

	// handler
	rc = stu_rtmp_on_publish(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

	stu_rtmp_amf_delete(ai_name);
	stu_rtmp_amf_delete(ai_type);

failed:

	stu_rtmp_amf_delete(r->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_seek(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v; // null
	buf->pos += v->cost;

	// milliseconds
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->milliseconds = *(stu_double_t *) v->value;
	buf->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	rc = stu_rtmp_on_seek(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

failed:

	stu_rtmp_amf_delete(r->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_pause(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v; // null
	buf->pos += v->cost;

	// pause
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->flag = (stu_bool_t) v->value;
	buf->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// milliseconds
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->milliseconds = *(stu_double_t *) v->value;
	buf->pos += v->cost;

	stu_rtmp_amf_delete(v);

	// handler
	rc = stu_rtmp_on_pause(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

failed:

	stu_rtmp_amf_delete(r->command_obj);

	return rc;
}

static stu_int32_t
stu_rtmp_process_command_on_status(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_buf_t        *buf;
	stu_rtmp_amf_t   *v;
	stu_int32_t       rc;

	ck = r->chunk_in;
	buf = &ck->payload;

	// command object
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		return STU_ERROR;
	}

	r->command_obj = v; // null
	buf->pos += v->cost;

	// information
	v = stu_rtmp_amf_parse(buf->pos, buf->last - buf->pos);
	if (v == NULL) {
		stu_log_error(0, "Failed to parse rtmp command: %s.", r->command->data);
		goto failed;
	}

	r->arguments = v;
	buf->pos += v->cost;

	// handler
	rc = stu_rtmp_on_status(r);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle rtmp command: %s.", r->command->data);
	}

failed:

	stu_rtmp_amf_delete(r->command_obj);
	stu_rtmp_amf_delete(r->arguments);

	return rc;
}

static stu_int32_t
stu_rtmp_process_aggregate(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck, sub;
	stu_buf_t        *buf;

	ck = r->chunk_in;
	buf = &ck->payload;

	if (buf->last - buf->pos < 11) {
		stu_log_error(0, "Failed to parse rtmp message of aggregate: Data not enough.");
		return STU_ERROR;
	}

	r->chunk_in = &sub;

	stu_queue_init(&sub.queue);
	sub.fmt = ck->fmt;
	sub.csid = ck->csid;

	for (/* void */; buf->pos < buf->last; /* void */) {
		sub.type_id = *buf->pos++;

		sub.payload.size  = *buf->pos++ << 16;
		sub.payload.size |= *buf->pos++ << 8;
		sub.payload.size |= *buf->pos++;

		sub.timestamp  = *buf->pos++ << 16;
		sub.timestamp |= *buf->pos++ << 8;
		sub.timestamp |= *buf->pos++;
		sub.timestamp |= *buf->pos++ << 24;

		sub.stream_id  = *buf->pos++ << 16;
		sub.stream_id |= *buf->pos++ << 8;
		sub.stream_id |= *buf->pos++;

		sub.payload.start = sub.payload.pos = buf->pos;
		sub.payload.end = sub.payload.last = buf->pos + sub.payload.size;

		if (sub.payload.last > buf->last) {
			stu_log_error(0, "Failed to parse rtmp message of aggregate: Bad payload length.");
			return STU_ERROR;
		}

		if (stu_rtmp_process_message(r) == STU_ERROR) {
			stu_log_error(0, "Failed to parse rtmp message of aggregate.");
			return STU_ERROR;
		}

		buf->pos += sub.payload.size + 4;
	}

	r->chunk_in = ck;

	return STU_OK;
}


static stu_int32_t
stu_rtmp_on_set_chunk_size(stu_rtmp_request_t *r) {
	r->connection.far_chunk_size = r->chunk_size;
	stu_log_debug(4, "rtmp set far_chunk_size: %d.", r->connection.far_chunk_size);
	return STU_OK;
}

static stu_int32_t
stu_rtmp_on_abort(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t *ck;
	stu_queue_t      *q;

	ck = NULL;

	for (q = stu_queue_tail(&r->chunks); q != stu_queue_sentinel(&r->chunks); q = stu_queue_prev(q)) {
		ck = stu_queue_data(q, stu_rtmp_chunk_t, queue);

		if (ck->state || ck->csid == r->chunk_stream_id) {
			stu_queue_remove(q);
			break;
		}
	}

	return STU_OK;
}

static stu_int32_t
stu_rtmp_on_ack(stu_rtmp_request_t *r) {
	stu_log_debug(4, "rtmp ack sequence number: %d, %2f\%.", r->ack, r->ack / r->connection.stat.bytes_out * 100);
	return STU_OK;
}

static stu_int32_t
stu_rtmp_on_user_control(stu_rtmp_request_t *r) {
	stu_rtmp_stream_t *ns;
	u_char             tmp[10];
	stu_str_t          key;
	stu_uint32_t       hk;

	stu_memzero(tmp, 10);

	stu_log_debug(4, "rtmp user control: type=%d, stream=%d, timestamp=%d, buffer=%d.",
			r->event_type, r->stream_id, r->timestamp, r->buffer_length);

	switch (r->event_type) {
	case STU_RTMP_EVENT_TYPE_SET_BUFFER_LENGTH:
		key.data = tmp;
		key.len = stu_sprintf(tmp, "%u", r->stream_id) - tmp;

		hk = stu_hash_key(key.data, key.len, r->connection.streams.flags);

		ns = stu_hash_find_locked(&r->connection.streams, hk, key.data, key.len);
		if (ns == NULL) {
			return STU_ERROR;
		}

		ns->buffer_time = r->buffer_length;
		break;
	}

	return STU_OK;
}

static stu_int32_t
stu_rtmp_on_ack_window_size(stu_rtmp_request_t *r) {
	r->connection.far_ack_window_size = r->ack_window_size;
	stu_log_debug(4, "rtmp set far_ack_window_size: %d.", r->connection.far_ack_window_size);
	return STU_OK;
}

static stu_int32_t
stu_rtmp_on_bandwidth(stu_rtmp_request_t *r) {
	r->connection.near_bandwidth = r->bandwidth;
	r->connection.near_limit_type = r->limit_type;
	stu_log_debug(4, "rtmp set near_bandwidth: %d, limit=%d.", r->bandwidth, r->limit_type);
	return STU_OK;
}

static stu_int32_t
stu_rtmp_on_edge(stu_rtmp_request_t *r) {
	return STU_OK;
}

static stu_int32_t
stu_rtmp_on_audio(stu_rtmp_request_t *r) {
	stu_rtmp_finalize_request(r, STU_DECLINED);
	return STU_OK;
}

static stu_int32_t
stu_rtmp_on_video(stu_rtmp_request_t *r) {
	stu_rtmp_finalize_request(r, STU_DECLINED);
	return STU_OK;
}

static stu_int32_t
stu_rtmp_on_data(stu_rtmp_request_t *r) {
	stu_rtmp_data_listener_t *l;
	stu_uint32_t              hk;

	hk = stu_hash_key(r->data_handler->data, r->data_handler->len, stu_rtmp_data_listener_hash.flags);

	l = stu_hash_find_locked(&stu_rtmp_data_listener_hash, hk, r->data_handler->data, r->data_handler->len);
	if (l && l->handler) {
		l->handler(r);
	}

	return STU_OK;
}

static stu_int32_t
stu_rtmp_on_shared_object(stu_rtmp_request_t *r) {
	return STU_OK;
}

static stu_int32_t
stu_rtmp_on_command(stu_rtmp_request_t *r) {
	stu_rtmp_command_listener_t *l;
	stu_uint32_t                 hk;
	stu_int32_t                  rc;

	hk = stu_hash_key(r->command->data, r->command->len, stu_rtmp_command_listener_hash.flags);

	l = stu_hash_find_locked(&stu_rtmp_command_listener_hash, hk, r->command->data, r->command->len);
	if (l && l->handler) {
		rc = l->handler(r);
	} else {
		stu_log_debug(4, "Unrecognized rtmp command: %s.", r->command->data);
		rc = STU_OK; // Just ignore this command.
	}

	return rc;
}


static stu_int32_t
stu_rtmp_on_set_data_frame(stu_rtmp_request_t *r) {
	stu_rtmp_connection_t *nc;
	stu_rtmp_stream_t     *ns;
	stu_rtmp_chunk_t      *ck;
	u_char                 tmp[10];
	stu_str_t              key;
	stu_uint32_t           hk;

	nc = &r->connection;
	ck = r->chunk_in;
	stu_memzero(tmp, 10);

	key.data = tmp;
	key.len = stu_sprintf(tmp, "%u", ck->stream_id) - tmp;

	hk = stu_hash_key(key.data, key.len, nc->streams.flags);

	ns = stu_hash_find_locked(&nc->streams, hk, key.data, key.len);
	if (ns) {
		stu_rtmp_finalize_request(r, STU_DECLINED);
		return stu_rtmp_set_data_frame(ns, r->data_key, r->data_value);
	}

	stu_log_error(0, "Failed to set rtmp data frame: %s.", r->data_key->data);

	return STU_ERROR;
}

static stu_int32_t
stu_rtmp_on_clear_data_frame(stu_rtmp_request_t *r) {
	stu_rtmp_connection_t *nc;
	stu_rtmp_stream_t     *ns;
	stu_rtmp_chunk_t      *ck;
	u_char                 tmp[10];
	stu_str_t              key;
	stu_uint32_t           hk;

	nc = &r->connection;
	ck = r->chunk_in;
	stu_memzero(tmp, 10);

	key.data = tmp;
	key.len = stu_sprintf(tmp, "%u", ck->stream_id) - tmp;

	hk = stu_hash_key(key.data, key.len, nc->streams.flags);

	ns = stu_hash_find_locked(&nc->streams, hk, key.data, key.len);
	if (ns) {
		stu_rtmp_finalize_request(r, STU_DECLINED);
		return stu_rtmp_clear_data_frame(ns, r->data_key);
	}

	stu_log_error(0, "Failed to clear rtmp data frame: %s.", r->data_key->data);

	return STU_ERROR;
}

static stu_int32_t
stu_rtmp_on_metadata(stu_rtmp_request_t *r) {
	r->data_key = &STU_RTMP_ON_META_DATA;
	return stu_rtmp_on_set_data_frame(r);
}


void
stu_rtmp_process_request(stu_rtmp_request_t *r) {
	stu_connection_t *c;
	stu_list_elt_t   *elts;
	stu_hash_elt_t   *e;
	stu_queue_t      *q;

	c = r->connection.conn;

	if (c->read->timer_set) {
		stu_timer_del(c->read);
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

}

void
stu_rtmp_finalize_request(stu_rtmp_request_t *r, stu_int32_t rc) {
	stu_log_debug(4, "rtmp finalize request: %d.", rc);

	if (rc == STU_OK) {
		return;
	}

	if (rc == STU_DECLINED) {
		//r->write_event_handler = stu_rtmp_run_phases;
		stu_rtmp_run_phases(r);
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
	r->connection.conn->request = NULL;
}

void
stu_rtmp_close_request(stu_rtmp_request_t *r) {
	stu_rtmp_connection_t *nc;

	nc = &r->connection;

	stu_rtmp_free_request(r);
	stu_rtmp_close_connection(nc);
}
