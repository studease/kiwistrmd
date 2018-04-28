/*
 * stu_rtmp_netstream.c
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"


void
stu_rtmp_stream_attach(stu_rtmp_stream_t *ns, stu_rtmp_connection_t *nc) {
	u_char     tmp[10];
	stu_str_t  key;

	stu_memzero(tmp, 10);

	key.data = tmp;
	key.len = stu_sprintf(tmp, "%u", ns->id) - tmp;

	ns->connection = nc;
	ns->type = STU_RTMP_STREAM_TYPE_IDLE;
	ns->receive_audio = TRUE;
	ns->receive_video = TRUE;

	stu_hash_init(&ns->data_frames, STU_RTMP_DATA_FRAMES_SIZE, NULL, STU_HASH_FLAGS_LOWCASE);

	stu_hash_insert_locked(&nc->streams, &key, ns);
}


/*
 * @start:    -2
 * @duration: -1
 * @reset:    TRUE
 */
stu_int32_t
stu_rtmp_play(stu_rtmp_stream_t *ns, u_char *name, size_t len, stu_double_t start, stu_double_t duration, stu_bool_t reset) {
	stu_rtmp_connection_t *nc;
	stu_rtmp_amf_t        *ao_cmd, *ao_tran, *ao_prop, *ao_name, *ao_start, *ao_duration, *ao_reset;
	u_char                *pos;
	u_char                 tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t            rc;

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
stu_rtmp_play2(stu_rtmp_stream_t *ns) {
	// TODO
	return STU_OK;
}

stu_int32_t
stu_rtmp_release_stream(stu_rtmp_connection_t *nc, u_char *name, size_t len) {
	stu_rtmp_stream_t *ns;
	stu_rtmp_amf_t    *ao_cmd, *ao_tran, *ao_prop, *ao_name;
	u_char            *pos;
	u_char             tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_uint32_t       hk, stream_id;
	stu_int32_t        rc;

	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	hk = stu_hash_key(name, len, nc->streams.flags);
	ns = stu_hash_remove_locked(&nc->streams, hk, name, len);
	stream_id = ns ? ns->id : 0;

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
stu_rtmp_delete_stream(stu_rtmp_connection_t *nc, stu_uint32_t stream_id) {
	stu_rtmp_amf_t *ao_cmd, *ao_tran, *ao_prop, *ao_id;
	u_char         *pos;
	u_char          tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_str_t       key;
	stu_uint32_t    hk;
	stu_int32_t     rc;

	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	key.data = tmp;
	key.len = stu_sprintf(tmp, "%u", stream_id) - tmp;

	hk = stu_hash_key(key.data, key.len, nc->streams.flags);
	stu_hash_remove_locked(&nc->streams, hk, key.data, key.len);

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
stu_rtmp_close_stream(stu_rtmp_stream_t *ns) {
	// TODO
	return STU_OK;
}

stu_int32_t
stu_rtmp_receive_audio(stu_rtmp_stream_t *ns, stu_bool_t flag) {
	stu_rtmp_connection_t *nc;
	stu_rtmp_amf_t        *ao_cmd, *ao_tran, *ao_prop, *ao_flag;
	u_char                *pos;
	u_char                 tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t            rc;

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
stu_rtmp_receive_video(stu_rtmp_stream_t *ns, stu_bool_t flag) {
	stu_rtmp_connection_t *nc;
	stu_rtmp_amf_t        *ao_cmd, *ao_tran, *ao_prop, *ao_flag;
	u_char                *pos;
	u_char                 tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t            rc;

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
stu_rtmp_publish(stu_rtmp_stream_t *ns, u_char *name, size_t len, stu_str_t *type) {
	stu_rtmp_connection_t *nc;
	stu_rtmp_amf_t        *ao_cmd, *ao_tran, *ao_prop, *ao_name, *ao_type;
	u_char                *pos;
	u_char                 tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t            rc;

	nc = ns->connection;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

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
stu_rtmp_seek(stu_rtmp_stream_t *ns) {
	// TODO
	return STU_OK;
}

stu_int32_t
stu_rtmp_pause(stu_rtmp_stream_t *ns, stu_bool_t flag, stu_double_t time) {
	stu_rtmp_connection_t *nc;
	stu_rtmp_amf_t        *ao_cmd, *ao_tran, *ao_prop, *ao_flag, *ao_time;
	u_char                *pos;
	u_char                 tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t            rc;

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
stu_rtmp_send_status(stu_rtmp_stream_t *ns, stu_str_t *level, stu_str_t *code, const char *description) {
	stu_rtmp_connection_t *nc;
	stu_rtmp_amf_t        *ao_cmd, *ao_tran, *ao_prop, *ao_info;
	u_char                *pos;
	u_char                 tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t            rc;

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
		stu_rtmp_close_connection(nc);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_info);

	return rc;
}


stu_int32_t
stu_rtmp_set_data_frame(stu_rtmp_stream_t *ns, stu_str_t *key, stu_rtmp_amf_t *value) {
	stu_rtmp_connection_t *nc;

	nc = ns->connection;

	if (stu_hash_insert_locked(&ns->data_frames, key, value) == STU_ERROR) {
		stu_log_error(0, "Failed to set rtmp data frame \"%s\": fd=%d.", key->data, nc->conn->fd);
		return STU_ERROR;
	}

	if (stu_strncmp(STU_RTMP_ON_META_DATA.data, key->data, key->len) == 0) {
		ns->metadata = value;
	}

	stu_log_debug(4, "set rtmp data frame \"%s\": fd=%d.", key->data, nc->conn->fd);

	return STU_OK;
}

stu_int32_t
stu_rtmp_clear_data_frame(stu_rtmp_stream_t *ns, stu_str_t *key) {
	stu_rtmp_connection_t *nc;
	stu_uint32_t           hk;

	nc = ns->connection;

	hk = stu_hash_key(key->data, key->len, ns->data_frames.flags);

	stu_hash_remove_locked(&ns->data_frames, hk, key->data, key->len);

	if (stu_strncmp(STU_RTMP_ON_META_DATA.data, key->data, key->len) == 0) {
		if (ns->metadata) {
			stu_rtmp_amf_delete(ns->metadata);
			ns->metadata = NULL;
		}
	}

	stu_log_debug(4, "clear rtmp data frame \"%s\": fd=%d.", key->data, nc->conn->fd);

	return STU_OK;
}


stu_int32_t
stu_rtmp_on_play(stu_rtmp_request_t *r) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_play2(stu_rtmp_request_t *r) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_release_stream(stu_rtmp_request_t *r) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_delete_stream(stu_rtmp_request_t *r) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_close_stream(stu_rtmp_request_t *r) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_receive_audio(stu_rtmp_request_t *r) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_receive_video(stu_rtmp_request_t *r) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_fcpublish(stu_rtmp_request_t *r) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_publish(stu_rtmp_request_t *r) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_seek(stu_rtmp_request_t *r) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_pause(stu_rtmp_request_t *r) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_status(stu_rtmp_request_t *r) {
	stu_rtmp_chunk_t      *ck;
	stu_rtmp_connection_t *nc;
	stu_rtmp_stream_t     *ns;
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
	if (ns && ns->on_status) {
		return ns->on_status(r);
	}

	return STU_OK; // Just ignore.
}
