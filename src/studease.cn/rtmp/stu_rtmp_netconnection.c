/*
 * stu_rtmp_netconnection.c
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

extern stu_rtmp_amf_t *stu_rtmp_properties;
extern stu_rtmp_amf_t *stu_rtmp_version;


stu_int32_t
stu_rtmp_connect() {
	return STU_OK;
}

stu_int32_t
stu_rtmp_create_stream() {
	return STU_OK;
}

stu_int32_t
stu_rtmp_call() {
	return STU_OK;
}


stu_int32_t
stu_rtmp_on_connect(stu_rtmp_netconnection_t *nc) {
	stu_rtmp_amf_t *cmd, *tran, *prop, *info, *item;
	u_char         *pos;
	u_char          tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_str_t       key;
	stu_int32_t     rc;

	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	if (stu_strncasecmp(nc->read_access.data, (u_char *) "/", 1) == 0 ||
			stu_strncasecmp(nc->read_access.data + 1, nc->app_name.data, nc->app_name.len) == 0) {
		rc = stu_rtmp_accept(nc);

		cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_RESULT.data, STU_RTMP_CMD_RESULT.len);
		info = stu_rtmp_get_information(&STU_RTMP_LEVEL_STATUS, &STU_RTMP_CODE_NETCONNECTION_CONNECT_SUCCESS, "connect success");
	} else {
		stu_log_error(0, "Failed to handle \"connect\" command: Access denied.");

		rc = stu_rtmp_reject(nc);

		cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_ERROR.data, STU_RTMP_CMD_ERROR.len);
		info = stu_rtmp_get_information(&STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETCONNECTION_CONNECT_REJECTED, "connect reject");
	}

	tran = stu_rtmp_amf_create_number(NULL, 1);
	prop = stu_rtmp_amf_duplicate(stu_rtmp_properties, TRUE);

	stu_str_set(&key, "objectEncoding");
	item = stu_rtmp_amf_create_number(&key, nc->object_encoding);
	stu_rtmp_amf_add_item_to_object(info, item);

	stu_str_set(&key, "data");
	item = stu_rtmp_amf_duplicate(stu_rtmp_version, TRUE);
	if (stu_rtmp_amf_set_key(item, key.data, key.len) == STU_ERROR) {
		goto done;
	}
	stu_rtmp_amf_add_item_to_object(info, item);

	pos = stu_rtmp_amf_stringify(pos, cmd);
	pos = stu_rtmp_amf_stringify(pos, tran);
	pos = stu_rtmp_amf_stringify(pos, prop);
	pos = stu_rtmp_amf_stringify(pos, info);

	if (stu_rtmp_send_buffer(nc, tmp, pos - tmp) == STU_ERROR) {
		goto done;
	}

done:

	stu_rtmp_amf_delete(cmd);
	stu_rtmp_amf_delete(tran);
	stu_rtmp_amf_delete(prop);
	stu_rtmp_amf_delete(info);

	return rc;
}

stu_int32_t
stu_rtmp_on_close(stu_rtmp_netconnection_t *nc) {
	nc->connected = FALSE;
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_create_stream(stu_rtmp_netconnection_t *nc) {
	stu_rtmp_amf_t             *cmd, *tran, *null, *info;
	stu_rtmp_request_t         *r;
	stu_rtmp_command_message_t *m;
	stu_rtmp_stream_t          *stream;
	u_char                     *pos;
	u_char                      tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];

	r = nc->connection->request;
	m = r->message;

	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	if (stu_strncasecmp(nc->read_access.data, (u_char *) "/", 1) == 0 ||
			stu_strncasecmp(nc->read_access.data + 1, nc->app_name.data, nc->app_name.len) == 0) {
		stream = stu_pcalloc(nc->connection->pool, sizeof(stu_rtmp_stream_t));
		if (stream) {
			stu_rtmp_stream_init(stream);
			stu_rtmp_netstream_attach(&r->ns, stream);

			cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_RESULT.data, STU_RTMP_CMD_RESULT.len);
			tran = stu_rtmp_amf_create_number(NULL, m->transaction_id);
			null = stu_rtmp_amf_create_null(NULL);
			info = stu_rtmp_amf_create_number(NULL, stream->id);

			goto done;
		}
	}

	cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_ERROR.data, STU_RTMP_CMD_ERROR.len);
	tran = stu_rtmp_amf_create_number(NULL, m->transaction_id);
	null = stu_rtmp_amf_create_null(NULL);
	info = stu_rtmp_get_information(&STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETSTREAM_FAILED, "create stream failed");

done:

	pos = stu_rtmp_amf_stringify(pos, cmd);
	pos = stu_rtmp_amf_stringify(pos, tran);
	pos = stu_rtmp_amf_stringify(pos, null);
	pos = stu_rtmp_amf_stringify(pos, info);

	stu_rtmp_amf_delete(cmd);
	stu_rtmp_amf_delete(tran);
	stu_rtmp_amf_delete(null);
	stu_rtmp_amf_delete(info);

	return stu_rtmp_send_buffer(nc, tmp, pos - tmp);
}

stu_int32_t
stu_rtmp_on_result(stu_rtmp_netconnection_t *nc) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_error(stu_rtmp_netconnection_t *nc) {
	return STU_OK;
}


stu_int32_t
stu_rtmp_set_chunk_size(stu_rtmp_netconnection_t *nc, stu_int32_t size) {
	stu_rtmp_request_t *r;
	u_char             *pos;
	u_char              tmp[4];
	stu_rtmp_chunk_t    ck;

	r = nc->connection->request;

	pos = tmp;
	stu_memzero(tmp, 4);

	*(stu_uint32_t *) pos = stu_endian_32(size);
	pos += 4;

	ck.basic.fmt = 0;
	ck.basic.csid = STU_RTMP_CSID_PROTOCOL_CONTROL;

	ck.header.timestamp = 0;
	ck.header.message_len = pos - tmp;
	ck.header.type = STU_RTMP_MSG_TYPE_SET_CHUNK_SIZE;
	ck.header.stream_id = 0;

	ck.extended = FALSE;

	ck.payload.start = tmp;
	ck.payload.pos = ck.payload.last = ck.payload.start;
	ck.payload.end = pos;
	ck.payload.size = ck.header.message_len;

	if (stu_rtmp_write_by_chunk(r, &ck) == STU_ERROR) {
		stu_log_error(0, "Failed to set neer_chunk_size: %d.", size);
		return STU_ERROR;
	}

	nc->near_chunk_size = size;
	stu_log_debug(4, "set neer_chunk_size: %d.", size);

	return STU_OK;
}

stu_int32_t
stu_rtmp_send_abort(stu_rtmp_netconnection_t *nc) {
	// TODO: send abort
	return STU_OK;
}

stu_int32_t
stu_rtmp_send_ack_sequence(stu_rtmp_netconnection_t *nc) {
	stu_rtmp_request_t *r;
	u_char             *pos;
	u_char              tmp[4];
	stu_rtmp_chunk_t    ck;

	r = nc->connection->request;

	pos = tmp;
	stu_memzero(tmp, 4);

	*(stu_uint32_t *) pos = stu_endian_32(nc->stat.bytes_in);
	pos += 4;

	ck.basic.fmt = 0;
	ck.basic.csid = STU_RTMP_CSID_PROTOCOL_CONTROL;

	ck.header.timestamp = 0;
	ck.header.message_len = pos - tmp;
	ck.header.type = STU_RTMP_MSG_TYPE_ACK;
	ck.header.stream_id = 0;

	ck.extended = FALSE;

	ck.payload.start = tmp;
	ck.payload.pos = ck.payload.last = ck.payload.start;
	ck.payload.end = pos;
	ck.payload.size = ck.header.message_len;

	if (stu_rtmp_write_by_chunk(r, &ck) == STU_ERROR) {
		stu_log_error(0, "Failed to send ack sequence: %d.", nc->stat.bytes_in);
		return STU_ERROR;
	}

	stu_log_debug(4, "send ack sequence: %d.", nc->stat.bytes_in);

	return STU_OK;
}

stu_int32_t
stu_rtmp_send_user_control(stu_rtmp_netconnection_t *nc, stu_uint16_t event, stu_uint32_t stream_id, stu_uint32_t buffer_len, stu_uint32_t timestamp) {
	stu_rtmp_request_t *r;
	u_char             *pos;
	u_char              tmp[10];
	stu_rtmp_chunk_t    ck;

	r = nc->connection->request;

	pos = tmp;
	stu_memzero(tmp, 10);

	*(stu_uint16_t *) pos = stu_endian_16(event);
	pos += 2;

	if (event <= STU_RTMP_EVENT_TYPE_STREAM_IS_RECORDED) {
		*(stu_uint32_t *) pos = stu_endian_32(stream_id);
		pos += 4;
	}

	if (event == STU_RTMP_EVENT_TYPE_SET_BUFFER_LENGTH) {
		*(stu_uint32_t *) pos = stu_endian_32(buffer_len);
		pos += 4;
	}

	if (event == STU_RTMP_EVENT_TYPE_PING_REQUEST || event == STU_RTMP_EVENT_TYPE_PING_RESPONSE) {
		*(stu_uint32_t *) pos = stu_endian_32(timestamp);
		pos += 4;
	}

	ck.basic.fmt = 0;
	ck.basic.csid = STU_RTMP_CSID_PROTOCOL_CONTROL;

	ck.header.timestamp = 0;
	ck.header.message_len = pos - tmp;
	ck.header.type = STU_RTMP_MSG_TYPE_USER_CONTROL;
	ck.header.stream_id = 0;

	ck.extended = FALSE;

	ck.payload.start = tmp;
	ck.payload.pos = ck.payload.last = ck.payload.start;
	ck.payload.end = pos;
	ck.payload.size = ck.header.message_len;

	if (stu_rtmp_write_by_chunk(r, &ck) == STU_ERROR) {
		stu_log_error(0, "Failed to send user control: 0x%02X.", event);
		return STU_ERROR;
	}

	stu_log_debug(4, "send user control: 0x%02X.", event);

	return STU_OK;
}

stu_int32_t
stu_rtmp_set_ack_window_size(stu_rtmp_netconnection_t *nc, stu_uint32_t size) {
	stu_rtmp_request_t *r;
	u_char             *pos;
	u_char              tmp[4];
	stu_rtmp_chunk_t    ck;

	r = nc->connection->request;

	pos = tmp;
	stu_memzero(tmp, 4);

	*(stu_uint32_t *) pos = stu_endian_32(size);
	pos += 4;

	ck.basic.fmt = 0;
	ck.basic.csid = STU_RTMP_CSID_PROTOCOL_CONTROL;

	ck.header.timestamp = 0;
	ck.header.message_len = pos - tmp;
	ck.header.type = STU_RTMP_MSG_TYPE_ACK_WINDOW_SIZE;
	ck.header.stream_id = 0;

	ck.extended = FALSE;

	ck.payload.start = tmp;
	ck.payload.pos = ck.payload.last = ck.payload.start;
	ck.payload.end = pos;
	ck.payload.size = ck.header.message_len;

	if (stu_rtmp_write_by_chunk(r, &ck) == STU_ERROR) {
		stu_log_error(0, "Failed to set near_ack_window_size: %d.", size);
		return STU_ERROR;
	}

	nc->near_ack_window_size = size;
	stu_log_debug(4, "set near_ack_window_size: %d.", size);

	return STU_OK;
}

stu_int32_t
stu_rtmp_set_peer_bandwidth(stu_rtmp_netconnection_t *nc, stu_uint32_t bandwidth, stu_uint8_t limit_type) {
	stu_rtmp_request_t *r;
	u_char             *pos;
	u_char              tmp[5];
	stu_rtmp_chunk_t    ck;

	r = nc->connection->request;

	pos = tmp;
	stu_memzero(tmp, 5);

	*(stu_uint32_t *) pos = stu_endian_32(bandwidth);
	pos += 4;

	*pos++ = limit_type;

	ck.basic.fmt = 0;
	ck.basic.csid = STU_RTMP_CSID_PROTOCOL_CONTROL;

	ck.header.timestamp = 0;
	ck.header.message_len = pos - tmp;
	ck.header.type = STU_RTMP_MSG_TYPE_BANDWIDTH;
	ck.header.stream_id = 0;

	ck.extended = FALSE;

	ck.payload.start = tmp;
	ck.payload.pos = ck.payload.last = ck.payload.start;
	ck.payload.end = pos;
	ck.payload.size = ck.header.message_len;

	if (stu_rtmp_write_by_chunk(r, &ck) == STU_ERROR) {
		stu_log_error(0, "Failed to set far_bandwidth: ack=%d, type=%d.", bandwidth, limit_type);
		return STU_ERROR;
	}

	nc->far_bandwidth = bandwidth;
	nc->far_limit_type = limit_type;
	stu_log_debug(4, "set far_bandwidth: ack=%d, type=%d.", bandwidth, limit_type);

	return STU_OK;
}


stu_int32_t
stu_rtmp_send_buffer(stu_rtmp_netconnection_t *nc, u_char *data, size_t len) {
	stu_rtmp_request_t *r;
	stu_rtmp_chunk_t    ck;

	r = nc->connection->request;

	ck.basic.fmt = 0;
	ck.basic.csid = r->chunk_in->basic.csid;

	ck.header.timestamp = r->chunk_in->header.timestamp;
	ck.header.message_len = len;
	ck.header.type = r->chunk_in->header.type;
	ck.header.stream_id = r->chunk_in->header.stream_id;

	ck.extended = r->chunk_in->extended;

	ck.payload.start = data;
	ck.payload.pos = ck.payload.last = ck.payload.start;
	ck.payload.end = data + len;
	ck.payload.size = len;

	return stu_rtmp_write_by_chunk(r, &ck);
}

stu_rtmp_amf_t *
stu_rtmp_get_information(stu_str_t *level, stu_str_t *code, const char *description) {
	stu_rtmp_amf_t *info, *item;
	stu_str_t       key;

	info = stu_rtmp_amf_create_object(NULL);

	stu_str_set(&key, "level");
	item = stu_rtmp_amf_create_string(&key, level->data, level->len);
	stu_rtmp_amf_add_item_to_object(info, item);

	stu_str_set(&key, "code");
	item = stu_rtmp_amf_create_string(&key, code->data, code->len);
	stu_rtmp_amf_add_item_to_object(info, item);

	stu_str_set(&key, "description");
	item = stu_rtmp_amf_create_string(&key, (u_char *) description, stu_strlen(description));
	stu_rtmp_amf_add_item_to_object(info, item);

	return info;
}
