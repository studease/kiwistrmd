/*
 * stu_rtmp_netstream.c
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

static stu_int32_t  stu_rtmp_send_frame(stu_rtmp_netstream_t *ns);
static void         stu_rtmp_run_phases(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_phase_foreach_handler(stu_rtmp_request_t *r, stu_rtmp_phase_t *ph);

extern stu_hash_t   stu_rtmp_phases;


stu_int32_t
stu_rtmp_attach(stu_rtmp_netstream_t *ns, stu_rtmp_netconnection_t *nc) {
	ns->connection = nc;
	ns->receive_audio = TRUE;
	ns->receive_video = TRUE;

	return STU_OK;
}

stu_int32_t
stu_rtmp_sink(stu_rtmp_netstream_t *ns, stu_rtmp_stream_t *s) {
	ns->stream = s;
	return STU_OK;
}

stu_int32_t
stu_rtmp_source(stu_rtmp_netstream_t *ns, stu_rtmp_stream_t *s) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_instance_t      *inst;

	nc = ns->connection;
	inst = nc->instance;

	stu_mutex_lock(&inst->lock);

	ns->stream = s;
	s->subscribers++;

	stu_mutex_unlock(&inst->lock);

	return STU_OK;
}


/*
 * @start:    -2
 * @duration: -1
 * @reset:    TRUE
 */
stu_int32_t
stu_rtmp_play(stu_rtmp_netstream_t *ns, u_char *name, size_t len, stu_double_t start, stu_double_t duration, stu_bool_t reset) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_amf_t           *ao_cmd, *ao_tran, *ao_prop, *ao_name, *ao_start, *ao_duration, *ao_reset;
	u_char                   *pos;
	u_char                    tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t               rc;

	nc = ns->connection;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	ns->name.data = stu_pcalloc(nc->conn->pool, len + 1);
	if (ns->name.data == NULL) {
		stu_log_error(0, "Failed to alloc rtmp stream name: %s.", name);
		return STU_ERROR;
	}

	stu_strncpy(ns->name.data, name, len);

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_PLAY.data, STU_RTMP_CMD_PLAY.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, nc->transaction_id++);
	ao_prop = stu_rtmp_amf_create_null(NULL);
	ao_name = stu_rtmp_amf_create_string(NULL, name, len);
	ao_start = stu_rtmp_amf_create_number(NULL, start);
	ao_duration = stu_rtmp_amf_create_number(NULL, duration);
	ao_reset = stu_rtmp_amf_create_bool(NULL, reset);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);
	pos = stu_rtmp_amf_stringify(pos, ao_name);
	pos = stu_rtmp_amf_stringify(pos, ao_start);
	pos = stu_rtmp_amf_stringify(pos, ao_duration);
	pos = stu_rtmp_amf_stringify(pos, ao_reset);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_AV, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, ns->id, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d, stream=%s.",
				STU_RTMP_CMD_CONNECT.data, nc->conn->fd, name);
		stu_rtmp_close_connection(nc);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_name);
	stu_rtmp_amf_delete(ao_start);
	stu_rtmp_amf_delete(ao_duration);
	stu_rtmp_amf_delete(ao_reset);

	return rc;
}

stu_int32_t
stu_rtmp_play2(stu_rtmp_netstream_t *ns) {
	// TODO
	return STU_OK;
}

stu_int32_t
stu_rtmp_release_stream(stu_rtmp_netconnection_t *nc, u_char *name, size_t len) {
	stu_rtmp_request_t   *r;
	stu_rtmp_netstream_t *ns;
	stu_rtmp_amf_t       *ao_cmd, *ao_tran, *ao_prop, *ao_name;
	u_char               *pos;
	u_char                tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_uint32_t          stream_id;
	stu_int32_t           rc;

	r = nc->conn->request;
	stream_id = 0;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	ns = stu_rtmp_find_netstream_by_name(r, name, len);
	if (ns) {
		r->streams[ns->id - 1] = NULL;
		stream_id = ns->id;
	}

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_RELEASE_STREAM.data, STU_RTMP_CMD_RELEASE_STREAM.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, nc->transaction_id++);
	ao_prop = stu_rtmp_amf_create_null(NULL);
	ao_name = stu_rtmp_amf_create_string(NULL, name, len);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);
	pos = stu_rtmp_amf_stringify(pos, ao_name);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, stream_id, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d, stream=%s.",
				STU_RTMP_CMD_RELEASE_STREAM.data, nc->conn->fd, name);
		stu_rtmp_close_connection(nc);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_name);

	return rc;
}

stu_int32_t
stu_rtmp_delete_stream(stu_rtmp_netconnection_t *nc, stu_uint32_t stream_id) {
	stu_rtmp_request_t   *r;
	stu_rtmp_netstream_t *ns;
	stu_rtmp_amf_t       *ao_cmd, *ao_tran, *ao_prop, *ao_id;
	u_char               *pos;
	u_char                tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t           rc;

	r = nc->conn->request;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	ns = stu_rtmp_find_netstream(r, stream_id);
	if (ns) {
		r->streams[ns->id - 1] = NULL;
	}

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_DELETE_STREAM.data, STU_RTMP_CMD_DELETE_STREAM.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, nc->transaction_id++);
	ao_prop = stu_rtmp_amf_create_null(NULL);
	ao_id = stu_rtmp_amf_create_number(NULL, stream_id);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);
	pos = stu_rtmp_amf_stringify(pos, ao_id);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, stream_id, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d, stream_id=%f.",
				STU_RTMP_CMD_DELETE_STREAM.data, nc->conn->fd, stream_id);
		stu_rtmp_close_connection(nc);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_id);

	return rc;
}

stu_int32_t
stu_rtmp_close_stream(stu_rtmp_netstream_t *ns) {
	// TODO
	return STU_OK;
}

stu_int32_t
stu_rtmp_receive_audio(stu_rtmp_netstream_t *ns, stu_bool_t flag) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_amf_t           *ao_cmd, *ao_tran, *ao_prop, *ao_flag;
	u_char                   *pos;
	u_char                    tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t               rc;

	nc = ns->connection;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_RECEIVE_AUDIO.data, STU_RTMP_CMD_RECEIVE_AUDIO.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, nc->transaction_id++);
	ao_prop = stu_rtmp_amf_create_null(NULL);
	ao_flag = stu_rtmp_amf_create_bool(NULL, flag);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);
	pos = stu_rtmp_amf_stringify(pos, ao_flag);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND_2, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, ns->id, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d, flag=%d.",
				STU_RTMP_CMD_RECEIVE_AUDIO.data, nc->conn->fd, flag);
		stu_rtmp_close_connection(nc);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_flag);

	return rc;
}

stu_int32_t
stu_rtmp_receive_video(stu_rtmp_netstream_t *ns, stu_bool_t flag) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_amf_t           *ao_cmd, *ao_tran, *ao_prop, *ao_flag;
	u_char                   *pos;
	u_char                    tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t               rc;

	nc = ns->connection;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_RECEIVE_VIDEO.data, STU_RTMP_CMD_RECEIVE_VIDEO.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, nc->transaction_id++);
	ao_prop = stu_rtmp_amf_create_null(NULL);
	ao_flag = stu_rtmp_amf_create_bool(NULL, flag);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);
	pos = stu_rtmp_amf_stringify(pos, ao_flag);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND_2, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, ns->id, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d, flag=%d.",
				STU_RTMP_CMD_RECEIVE_VIDEO.data, nc->conn->fd, flag);
		stu_rtmp_close_connection(nc);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_flag);

	return rc;
}

stu_int32_t
stu_rtmp_publish(stu_rtmp_netstream_t *ns, u_char *name, size_t len, stu_str_t *type) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_instance_t      *inst;
	stu_rtmp_stream_t        *s;
	stu_rtmp_amf_t           *ao_cmd, *ao_tran, *ao_prop, *ao_name, *ao_type;
	u_char                   *pos;
	u_char                    tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_uint32_t              hk;
	stu_int32_t               rc;

	nc = ns->connection;
	inst = nc->instance;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);
	rc = STU_ERROR;

	ns->name.data = stu_pcalloc(nc->conn->pool, len + 1);
	if (ns->name.data == NULL) {
		return STU_ERROR;
	}

	stu_strncpy(ns->name.data, name, len);
	ns->name.len = len;

	stu_mutex_lock(&inst->lock);

	hk = stu_hash_key(ns->name.data, ns->name.len, inst->streams.flags);

	s = stu_hash_find_locked(&inst->streams, hk, ns->name.data, ns->name.len);
	if (s == NULL) {
		s = stu_rtmp_stream_get(ns->name.data, ns->name.len);
		if (s == NULL) {
			stu_log_error(0, "Failed to get rtmp stream: name=%s.", ns->name.data);
			stu_mutex_unlock(&inst->lock);
			return STU_ERROR;
		}

		rc = stu_hash_insert_locked(&inst->streams, &ns->name, s);
		if (rc == STU_ERROR) {
			stu_log_error(0, "Failed to insert rtmp stream: /%s/%s.", nc->url.application.data, ns->name.data);
			stu_mutex_unlock(&inst->lock);
			return STU_ERROR;
		}
	}

	ns->stream = s;

	stu_mutex_unlock(&inst->lock);

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_PUBLISH.data, STU_RTMP_CMD_PUBLISH.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, nc->transaction_id++);
	ao_prop = stu_rtmp_amf_create_null(NULL);
	ao_name = stu_rtmp_amf_create_string(NULL, name, len);
	ao_type = stu_rtmp_amf_create_string(NULL, type->data, type->len);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);
	pos = stu_rtmp_amf_stringify(pos, ao_name);
	pos = stu_rtmp_amf_stringify(pos, ao_type);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND_2, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, ns->id, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d, stream=%s, type=%s.",
				STU_RTMP_CMD_PUBLISH.data, nc->conn->fd, name, type->data);
		stu_rtmp_close_connection(nc);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_name);
	stu_rtmp_amf_delete(ao_type);

	return rc;
}

stu_int32_t
stu_rtmp_seek(stu_rtmp_netstream_t *ns) {
	// TODO
	return STU_OK;
}

stu_int32_t
stu_rtmp_pause(stu_rtmp_netstream_t *ns, stu_bool_t flag, stu_double_t time) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_amf_t           *ao_cmd, *ao_tran, *ao_prop, *ao_flag, *ao_time;
	u_char                   *pos;
	u_char                    tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t               rc;

	nc = ns->connection;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_PAUSE.data, STU_RTMP_CMD_PAUSE.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, nc->transaction_id++);
	ao_prop = stu_rtmp_amf_create_null(NULL);
	ao_flag = stu_rtmp_amf_create_bool(NULL, flag);
	ao_time = stu_rtmp_amf_create_number(NULL, time);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);
	pos = stu_rtmp_amf_stringify(pos, ao_flag);
	pos = stu_rtmp_amf_stringify(pos, ao_time);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND_2, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, ns->id, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d, stream=%s, flag=%d, time=%f.",
				STU_RTMP_CMD_PAUSE.data, nc->conn->fd, ns->name.data, flag, time);
		stu_rtmp_close_connection(nc);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_flag);
	stu_rtmp_amf_delete(ao_time);

	return rc;
}

stu_int32_t
stu_rtmp_send_status(stu_rtmp_netstream_t *ns, stu_str_t *level, stu_str_t *code, const char *description) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_amf_t           *ao_cmd, *ao_tran, *ao_prop, *ao_info;
	u_char                   *pos;
	u_char                    tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t               rc;

	nc = ns->connection;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_ON_STATUS.data, STU_RTMP_CMD_ON_STATUS.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, 0);
	ao_prop = stu_rtmp_amf_create_null(NULL);
	ao_info = stu_rtmp_get_information(level, code, description);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);
	pos = stu_rtmp_amf_stringify(pos, ao_info);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND_2, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, ns->id, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d, level=%s, code=%s.",
				STU_RTMP_CMD_ON_STATUS.data, nc->conn->fd, level->data, code->data);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_info);

	return rc;
}


stu_int32_t
stu_rtmp_set_data_frame(stu_rtmp_netstream_t *ns, stu_str_t *key, stu_rtmp_amf_t *value, stu_bool_t remote) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_stream_t        *stream;
	stu_rtmp_amf_t           *ao_hlr, *ao_key;
	u_char                   *pos;
	u_char                    tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t               rc;

	nc = ns->connection;
	stream = ns->stream;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);
	rc = STU_OK;

	if (stu_hash_insert(&stream->data_frames, key, value) == STU_ERROR) {
		stu_log_error(0, "Failed to %s: fd=%d, key=%s, value=%p.",
				STU_RTMP_SET_DATA_FRAME.data, nc->conn->fd, key->data, value);
		return STU_ERROR;
	}

	if (remote == FALSE) {
		goto done;
	}

	ao_hlr = stu_rtmp_amf_create_string(NULL, STU_RTMP_SET_DATA_FRAME.data, STU_RTMP_SET_DATA_FRAME.len);
	ao_key = stu_rtmp_amf_create_string(NULL, key->data, key->len);

	pos = stu_rtmp_amf_stringify(pos, ao_hlr);
	pos = stu_rtmp_amf_stringify(pos, ao_key);
	pos = stu_rtmp_amf_stringify(pos, value);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND_2, 0, STU_RTMP_MESSAGE_TYPE_DATA, ns->id, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d, key=%s, value=%p.",
				STU_RTMP_SET_DATA_FRAME.data, nc->conn->fd, key->data, value);
	}

	stu_rtmp_amf_delete(ao_hlr);
	stu_rtmp_amf_delete(ao_key);

done:

	if (rc == STU_OK) {
		stu_log_debug(4, "%s: fd=%d, key=%s.", STU_RTMP_SET_DATA_FRAME.data, nc->conn->fd, key->data);
	}

	return rc;
}

stu_int32_t
stu_rtmp_clear_data_frame(stu_rtmp_netstream_t *ns, stu_str_t *key, stu_bool_t remote) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_stream_t        *stream;
	stu_rtmp_amf_t           *v, *ao_hlr, *ao_key;
	u_char                   *pos;
	u_char                    tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_uint32_t              hk;
	stu_int32_t               rc;

	nc = ns->connection;
	stream = ns->stream;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);
	rc = STU_OK;

	hk = stu_hash_key(key->data, key->len, stream->data_frames.flags);
	v = stu_hash_remove(&stream->data_frames, hk, key->data, key->len);
	stu_rtmp_amf_delete(v);

	if (remote == FALSE) {
		goto done;
	}

	ao_hlr = stu_rtmp_amf_create_string(NULL, STU_RTMP_CLEAR_DATA_FRAME.data, STU_RTMP_CLEAR_DATA_FRAME.len);
	ao_key = stu_rtmp_amf_create_string(NULL, key->data, key->len);

	pos = stu_rtmp_amf_stringify(pos, ao_hlr);
	pos = stu_rtmp_amf_stringify(pos, ao_key);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND_2, 0, STU_RTMP_MESSAGE_TYPE_DATA, ns->id, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d, key=%s.",
				STU_RTMP_CLEAR_DATA_FRAME.data, nc->conn->fd, key->data);
	}

	stu_rtmp_amf_delete(ao_hlr);
	stu_rtmp_amf_delete(ao_key);

done:

	if (rc == STU_OK) {
		stu_log_debug(4, "%s: fd=%d, key=%s.", STU_RTMP_CLEAR_DATA_FRAME.data, nc->conn->fd, key->data);
	}

	return rc;
}

stu_int32_t
stu_rtmp_send_video_frame(stu_rtmp_netstream_t *ns, stu_uint32_t timestamp, u_char *data, size_t len) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_frame_t         *f;

	nc = ns->connection;

	f = stu_rtmp_stream_append(ns->stream, STU_RTMP_MESSAGE_TYPE_VIDEO, timestamp, data, len);
	if (f == NULL) {
		stu_log_error(0, "Failed to append rtmp frame: fd=%d, ts=%u.", nc->conn->fd, timestamp);
		return STU_ERROR;
	}

	stu_rtmp_stream_drop(ns->stream, timestamp);

	return stu_rtmp_send_frame(ns);
}

stu_int32_t
stu_rtmp_send_audio_frame(stu_rtmp_netstream_t *ns, stu_uint32_t timestamp, u_char *data, size_t len) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_frame_t         *f;

	nc = ns->connection;

	f = stu_rtmp_stream_append(ns->stream, STU_RTMP_MESSAGE_TYPE_AUDIO, timestamp, data, len);
	if (f == NULL) {
		stu_log_error(0, "Failed to append rtmp frame: fd=%d, ts=%u.", nc->conn->fd, timestamp);
		return STU_ERROR;
	}

	return stu_rtmp_send_frame(ns);
}

static stu_int32_t
stu_rtmp_send_frame(stu_rtmp_netstream_t *ns) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_frame_t         *f;
	stu_list_t               *list;
	stu_list_elt_t           *elts, *e;
	stu_queue_t              *q;
	stu_int32_t               rc;

	nc = ns->connection;
	list = &ns->stream->frames;
	elts = &list->elts;

	for (q = stu_queue_head(&elts->queue); q != stu_queue_sentinel(&elts->queue); /* void */) {
		e = stu_queue_data(q, stu_list_elt_t, queue);
		f = e->value;

		rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND_2, f->timestamp, f->type, ns->id, f->payload.start, f->payload.size);
		if (rc == STU_ERROR) {
			stu_log_debug(4, "Failed to send rtmp frame: fd=%d, type=0x%02X, ts=%u, size=%u.",
					nc->conn->fd, f->type, f->timestamp, f->payload.size);
			break;
		}

		q = stu_queue_next(q);
		stu_list_remove(list, e);

		stu_free(f->payload.start);
		stu_free(f);
	}

	return STU_OK;
}


stu_int32_t
stu_rtmp_on_play(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_netstream_t     *ns;
	stu_rtmp_instance_t      *inst;
	stu_rtmp_stream_t        *s;
	stu_uint32_t              hk;
	stu_int32_t               rc;

	nc = &r->connection;
	inst = nc->instance;
	rc = STU_ERROR;

	stu_log_debug(4, "Handle command \"%s\": fd=%d, /%s/%s.",
			STU_RTMP_CMD_PLAY.data, nc->conn->fd, nc->url.application.data, r->stream_name->data);

	ns = stu_rtmp_find_netstream(r, r->chunk_in->stream_id);
	if (ns == NULL) {
		return STU_ERROR;
	}

	ns->name.data = stu_pcalloc(nc->conn->pool, r->stream_name->len + 1);
	if (ns->name.data == NULL) {
		return STU_ERROR;
	}

	stu_strncpy(ns->name.data, r->stream_name->data, r->stream_name->len);
	ns->name.len = r->stream_name->len;

	if (nc->read_access.len == 0 || stu_strncasecmp(nc->read_access.data, (u_char *) "/", 1) == 0) {
		hk = stu_hash_key(ns->name.data, ns->name.len, inst->streams.flags);

		s = stu_hash_find(&inst->streams, hk, ns->name.data, ns->name.len);
		if (s) {
			stu_rtmp_send_user_control(nc, STU_RTMP_EVENT_TYPE_STREAM_BEGIN, 0, 0, 0);

			rc = stu_rtmp_send_status(ns, &STU_RTMP_LEVEL_STATUS, &STU_RTMP_CODE_NETSTREAM_PLAY_START, "started playing");
			if (rc == STU_ERROR) {
				stu_log_error(0, "Failed to handle command \"%s\": fd=%d, level=%s, code=%s.",
						STU_RTMP_CMD_PLAY.data, nc->conn->fd, STU_RTMP_LEVEL_STATUS.data, STU_RTMP_CODE_NETSTREAM_PLAY_START.data);
			}
		} else {
			rc = stu_rtmp_send_status(ns, &STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETSTREAM_PLAY_STREAMNOTFOUND, "stream not found");
			if (rc == STU_ERROR) {
				stu_log_error(0, "Failed to handle command \"%s\": fd=%d, level=%s, code=%s.",
						STU_RTMP_CMD_PLAY.data, nc->conn->fd, STU_RTMP_LEVEL_ERROR.data, STU_RTMP_CODE_NETSTREAM_PLAY_STREAMNOTFOUND.data);
			}
		}
	} else {
		rc = stu_rtmp_send_status(ns, &STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETSTREAM_PLAY_FAILED, "access denied");
		if (rc == STU_ERROR) {
			stu_log_error(0, "Failed to handle command \"%s\": fd=%d, level=%s, code=%s.",
					STU_RTMP_CMD_PLAY.data, nc->conn->fd, STU_RTMP_LEVEL_ERROR.data, STU_RTMP_CODE_NETSTREAM_PLAY_FAILED.data);
		}
	}

	return rc;
}

stu_int32_t
stu_rtmp_on_play2(stu_rtmp_request_t *r) {
	// TODO: play2
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_release_stream(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_netstream_t     *ns;

	nc = &r->connection;

	stu_log_debug(4, "Handle command \"%s\": fd=%d, /%s/%s.",
			STU_RTMP_CMD_RELEASE_STREAM.data, nc->conn->fd, nc->url.application.data, r->stream_name->data);

	ns = stu_rtmp_find_netstream_by_name(r, r->stream_name->data, r->stream_name->len);
	if (ns) {
		r->streams[ns->id - 1] = NULL;
		stu_rtmp_close_netstream(ns);
	}

	return STU_OK;
}

stu_int32_t
stu_rtmp_on_delete_stream(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_netstream_t     *ns;

	nc = &r->connection;

	stu_log_debug(4, "Handle command \"%s\": fd=%d, /%s/%d.",
			STU_RTMP_CMD_DELETE_STREAM.data, nc->conn->fd, nc->url.application.data, r->stream_id);

	ns = stu_rtmp_find_netstream(r, r->stream_id);
	if (ns) {
		r->streams[ns->id - 1] = NULL;
		stu_rtmp_close_netstream(ns);
	}

	return STU_OK;
}

stu_int32_t
stu_rtmp_on_close_stream(stu_rtmp_request_t *r) {
	// TODO
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_receive_audio(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_netstream_t     *ns;

	nc = &r->connection;

	stu_log_debug(4, "Handle command \"%s\": fd=%d, flag=%d, /%s/%d.",
			STU_RTMP_CMD_RECEIVE_AUDIO.data, nc->conn->fd, r->flag, nc->url.application.data, r->chunk_in->stream_id);

	ns = stu_rtmp_find_netstream(r, r->chunk_in->stream_id);
	if (ns == NULL) {
		return STU_ERROR;
	}

	ns->receive_audio = r->flag;

	return STU_OK;
}

stu_int32_t
stu_rtmp_on_receive_video(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_netstream_t     *ns;

	nc = &r->connection;

	stu_log_debug(4, "Handle command \"%s\": fd=%d, flag=%d, /%s/%d.",
			STU_RTMP_CMD_RECEIVE_VIDEO.data, nc->conn->fd, r->flag, nc->url.application.data, r->chunk_in->stream_id);

	ns = stu_rtmp_find_netstream(r, r->chunk_in->stream_id);
	if (ns == NULL) {
		return STU_ERROR;
	}

	ns->receive_video = r->flag;

	return STU_OK;
}

stu_int32_t
stu_rtmp_on_fcpublish(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_amf_t           *ao_cmd, *ao_tran, *ao_prop, *ao_info;
	u_char                   *pos;
	u_char                    tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t               rc;

	nc = &r->connection;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	stu_log_debug(4, "Handle command \"%s\": fd=%d, /%s/%s.",
			STU_RTMP_CMD_FC_PUBLISH.data, nc->conn->fd, nc->url.application.data, r->stream_name->data);

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_ON_FC_PUBLISH.data, STU_RTMP_CMD_ON_FC_PUBLISH.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, 0);
	ao_prop = stu_rtmp_amf_create_null(NULL);
	ao_info = stu_rtmp_get_information(&STU_RTMP_LEVEL_STATUS, &STU_RTMP_CODE_NETSTREAM_PUBLISH_START, "FCPublish start");

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);
	pos = stu_rtmp_amf_stringify(pos, ao_info);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, 0, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to handle command \"%s\": fd=%d, level=%s, code=%s.",
				STU_RTMP_CMD_FC_PUBLISH.data, nc->conn->fd, STU_RTMP_LEVEL_STATUS.data, STU_RTMP_CODE_NETSTREAM_PUBLISH_START.data);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_info);

	return rc;
}

stu_int32_t
stu_rtmp_on_fcunpublish(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;

	nc = &r->connection;

	stu_log_debug(4, "Handle command \"%s\": fd=%d, /%s/%s.",
			STU_RTMP_CMD_FC_UNPUBLISH.data, nc->conn->fd, nc->url.application.data, r->stream_name->data);

	stu_rtmp_run_phases(r);

	return STU_OK;
}

static void
stu_rtmp_run_phases(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_phase_t         *ph;
	stu_list_elt_t           *elts;
	stu_hash_elt_t           *e;
	stu_queue_t              *q;

	nc = &r->connection;

	// TODO: use rwlock
	stu_mutex_lock(&stu_rtmp_phases.lock);

	elts = &stu_rtmp_phases.keys->elts;

	for (q = stu_queue_tail(&elts->queue); q != stu_queue_sentinel(&elts->queue); q = stu_queue_prev(q)) {
		e = stu_queue_data(q, stu_hash_elt_t, queue);
		ph = e->value;

		if (stu_strncasecmp(nc->url.application.data, ph->pattern.data, ph->pattern.len) != 0) {
			continue;
		}

		if (stu_rtmp_phase_foreach_handler(r, ph) == STU_ERROR) {
			stu_log_error(0, "Failed to run rtmp phases \"%s\": fd=%d, type=0x%02X.",
					ph->pattern.data, nc->conn->fd, r->chunk_in->type_id);
		}

		break;
	}

	stu_mutex_unlock(&stu_rtmp_phases.lock);
}

static stu_int32_t
stu_rtmp_phase_foreach_handler(stu_rtmp_request_t *r, stu_rtmp_phase_t *ph) {
	stu_rtmp_netconnection_t  *nc;
	stu_rtmp_phase_listener_t *l;
	stu_list_elt_t            *elts;
	stu_hash_elt_t            *e;
	stu_queue_t               *q;

	nc = &r->connection;
	elts = &ph->listeners->keys->elts;

	for (q = stu_queue_tail(&elts->queue); q != stu_queue_sentinel(&elts->queue); q = stu_queue_prev(q)) {
		e = stu_queue_data(q, stu_hash_elt_t, queue);
		l = e->value;

		if (l && l->close && l->close(r) == STU_ERROR) {
			stu_log_error(0, "Failed to run rtmp phase \"%s\": fd=%d, type=0x%02X.",
						l->name.data, nc->conn->fd, r->chunk_in->type_id);
			return STU_ERROR;
		}
	}

	return STU_OK;
}

stu_int32_t
stu_rtmp_on_publish(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_netstream_t     *ns;
	stu_rtmp_instance_t      *inst;
	stu_rtmp_stream_t        *s;
	stu_uint32_t              hk;
	stu_int32_t               rc;

	nc = &r->connection;
	inst = nc->instance;
	rc = STU_ERROR;

	stu_log_debug(4, "Handle command \"%s\": fd=%d, /%s/%s.",
			STU_RTMP_CMD_PUBLISH.data, nc->conn->fd, nc->url.application.data, r->stream_name->data);

	ns = stu_rtmp_find_netstream(r, r->chunk_in->stream_id);
	if (ns == NULL) {
		return STU_ERROR;
	}

	ns->name.data = stu_pcalloc(nc->conn->pool, r->stream_name->len + 1);
	if (ns->name.data == NULL) {
		return STU_ERROR;
	}

	stu_strncpy(ns->name.data, r->stream_name->data, r->stream_name->len);
	ns->name.len = r->stream_name->len;

	stu_mutex_lock(&inst->lock);

	hk = stu_hash_key(ns->name.data, ns->name.len, inst->streams.flags);

	s = stu_hash_find_locked(&inst->streams, hk, ns->name.data, ns->name.len);
	if (s == NULL) {
		s = stu_rtmp_stream_get(ns->name.data, ns->name.len);
		if (s == NULL) {
			stu_log_error(0, "Failed to get rtmp stream: name=%s.", ns->name.data);
			goto failed;
		}

		rc = stu_hash_insert_locked(&inst->streams, &ns->name, s);
		if (rc == STU_ERROR) {
			stu_log_error(0, "Failed to insert rtmp stream: /%s/%s.", nc->url.application.data, ns->name.data);
			goto failed;
		}
	}

	if (s->publishing) {
		rc = stu_rtmp_send_status(ns, &STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETSTREAM_PUBLISH_BADNAME, "publish bad name");
		if (rc == STU_ERROR) {
			stu_log_error(0, "Failed to handle command \"%s\": fd=%d, level=%s, code=%s.",
					STU_RTMP_CMD_PUBLISH.data, nc->conn->fd, STU_RTMP_LEVEL_ERROR.data, STU_RTMP_CODE_NETSTREAM_PUBLISH_BADNAME.data);
		}

		rc = STU_ERROR;
	} else {
		stu_rtmp_sink(ns, s);

		rc = stu_rtmp_send_status(ns, &STU_RTMP_LEVEL_STATUS, &STU_RTMP_CODE_NETSTREAM_PUBLISH_START, "publish start");
		if (rc == STU_ERROR) {
			stu_log_error(0, "Failed to handle command \"%s\": fd=%d, level=%s, code=%s.",
					STU_RTMP_CMD_PUBLISH.data, nc->conn->fd, STU_RTMP_LEVEL_STATUS.data, STU_RTMP_CODE_NETSTREAM_PUBLISH_START.data);
		}
	}

failed:

	stu_mutex_unlock(&inst->lock);

	return rc;
}

stu_int32_t
stu_rtmp_on_seek(stu_rtmp_request_t *r) {
	// TODO: seek
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_pause(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_netstream_t     *ns;

	nc = &r->connection;

	stu_log_debug(4, "Handle command \"%s\": fd=%d, flag=%d, time=%lf, /%s/%d.",
			STU_RTMP_CMD_PAUSE.data, nc->conn->fd, r->flag, r->milliseconds, nc->url.application.data, r->chunk_in->stream_id);

	ns = stu_rtmp_find_netstream(r, r->chunk_in->stream_id);
	if (ns == NULL) {
		return STU_ERROR;
	}

	ns->receive_audio = r->flag;

	return STU_OK;
}

stu_int32_t
stu_rtmp_on_status(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t         *ck;
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_netstream_t     *ns;
	stu_int32_t               rc;

	nc = &r->connection;
	ck = r->chunk_in;
	rc = STU_OK; // Just ignore.

	stu_log_debug(4, "Handle command \"%s\": fd=%d, /%s/%d.",
			STU_RTMP_CMD_ON_STATUS.data, nc->conn->fd, nc->url.application.data, ck->stream_id);

	ns = stu_rtmp_find_netstream(r, ck->stream_id);
	if (ns && ns->on_status) {
		rc = ns->on_status(r);
	}

	return rc;
}


void
stu_rtmp_close_netstream(stu_rtmp_netstream_t *ns) {

}

void
stu_rtmp_stream_detach(stu_rtmp_netstream_t *ns) {
	ns->stream = NULL;
}
