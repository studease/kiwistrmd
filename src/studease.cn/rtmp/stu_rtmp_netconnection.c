/*
 * stu_rtmp_connection.c
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

extern stu_rtmp_amf_t *stu_rtmp_properties;
extern stu_rtmp_amf_t *stu_rtmp_version;

const stu_str_t  STU_RTMP_KEY_APP = stu_string("app");
const stu_str_t  STU_RTMP_KEY_FLASH_VER = stu_string("flashVer");
const stu_str_t  STU_RTMP_KEY_SWF_URL = stu_string("swfUrl");
const stu_str_t  STU_RTMP_KEY_TC_URL = stu_string("tcUrl");
const stu_str_t  STU_RTMP_KEY_FPAD = stu_string("fpad");
const stu_str_t  STU_RTMP_KEY_AUDIO_CODECS = stu_string("audioCodecs");
const stu_str_t  STU_RTMP_KEY_VIDEO_CODECS = stu_string("videoCodecs");
const stu_str_t  STU_RTMP_KEY_VIDEO_FUNCTION = stu_string("videoFunction");
const stu_str_t  STU_RTMP_KEY_OBJECT_ENCODING = stu_string("objectEncoding");
const stu_str_t  STU_RTMP_KEY_DATA = stu_string("data");

const stu_str_t  STU_RTMP_VAL_FLASH_VER = stu_string("FMLE/3.0 (compatible; FMSc/1.0)");

#define SUPPORT_SND_NONE         0x0001
#define SUPPORT_SND_ADPCM        0x0002
#define SUPPORT_SND_MP3          0x0004
#define SUPPORT_SND_INTEL        0x0008
#define SUPPORT_SND_UNUSED       0x0010
#define SUPPORT_SND_NELLY8       0x0020
#define SUPPORT_SND_NELLY        0x0040
#define SUPPORT_SND_G711A        0x0080
#define SUPPORT_SND_G711U        0x0100
#define SUPPORT_SND_NELLY16      0x0200
#define SUPPORT_SND_AAC          0x0400
#define SUPPORT_SND_SPEEX        0x0800
#define SUPPORT_SND_ALL          0x0FFF

#define SUPPORT_VID_UNUSED       0x0001
#define SUPPORT_VID_JPEG         0x0002
#define SUPPORT_VID_SORENSON     0x0004
#define SUPPORT_VID_HOMEBREW     0x0008
#define SUPPORT_VID_VP6          0x0010
#define SUPPORT_VID_VP6ALPHA     0x0020
#define SUPPORT_VID_HOMWBREWV    0x0040
#define SUPPORT_VID_H264         0x0080
#define SUPPORT_VID_ALL          0x0FFF

#define SUPPORT_VID_CLIENT_SEEK  1

static stu_int32_t  stu_rtmp_rand_id(stu_rtmp_netconnection_t *nc, size_t len);
static stu_int32_t  stu_rtmp_add_responder(stu_rtmp_netconnection_t *nc, stu_rtmp_responder_t *responder);


void
stu_rtmp_netconnection_init(stu_rtmp_netconnection_t *nc, stu_connection_t *c) {
	nc->conn = c;
	nc->far_chunk_size = STU_RTMP_CHUNK_DEFAULT_SIZE;
	nc->near_chunk_size = STU_RTMP_CHUNK_DEFAULT_SIZE;
	nc->stream_id = 1;
	nc->transaction_id = 1;

	stu_rtmp_rand_id(nc, STU_RTMP_NETCONNECTION_ID_LEN);

	stu_hash_init(&nc->commands, STU_RTMP_LISTENER_DEFAULT_SIZE, NULL, STU_HASH_FLAGS_LOWCASE);
	stu_hash_init(&nc->netstreams, STU_RTMP_NETSTREAM_DEFAULT_SIZE, NULL, STU_HASH_FLAGS_LOWCASE);
	stu_hash_init(&nc->responders, STU_RTMP_RESPONDER_DEFAULT_SIZE, NULL, STU_HASH_FLAGS_LOWCASE);
}

static stu_int32_t
stu_rtmp_rand_id(stu_rtmp_netconnection_t *nc, size_t len) {
	u_char *pos;
	u_char  tmp[] = "0123456789ABCDEF";

	nc->id.data = stu_pcalloc(nc->conn->pool, len + 1);
	if (nc->id.data == NULL) {
		return STU_ERROR;
	}

	nc->id.len = len;

	for (pos = nc->id.data; len; len--) {
		*pos++ = tmp[rand() % 16];
	}

	return STU_OK;
}


/*
 * Arguments should end of NULL.
 */
stu_int32_t
stu_rtmp_connect(stu_rtmp_netconnection_t *nc, u_char *url, size_t len, stu_rtmp_responder_t *res, ...) {
	stu_connection_t *c;
	stu_rtmp_amf_t   *ao_cmd, *ao_tran, *ao_prop, *ao_args, *ao_item;
	u_char           *pos;
	u_char            tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t       rc;
	va_list           args;

	c = nc->conn;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	if (stu_rtmp_parse_url(&nc->url, url, len) == STU_ERROR) {
		stu_log_error(0, "Failed to parse rtmp url: %s.", url);
		return STU_ERROR;
	}

	if (stu_rtmp_add_responder(nc, res) == STU_ERROR) {
		stu_log_error(0, "Failed to add responder: fd=%d, id=%d.", c->fd, nc->transaction_id);
		return STU_ERROR;
	}

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_CONNECT.data, STU_RTMP_CMD_CONNECT.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, nc->transaction_id++);
	ao_prop = stu_rtmp_amf_create_object(NULL);
	ao_args = NULL;

	ao_item = stu_rtmp_amf_create_string((stu_str_t *) &STU_RTMP_KEY_APP, nc->url.application.data, nc->url.application.len);
	stu_rtmp_amf_add_item_to_object(ao_prop, ao_item);

	ao_item = stu_rtmp_amf_create_string((stu_str_t *) &STU_RTMP_KEY_FLASH_VER, STU_RTMP_VAL_FLASH_VER.data, STU_RTMP_VAL_FLASH_VER.len);
	stu_rtmp_amf_add_item_to_object(ao_prop, ao_item);

	ao_item = stu_rtmp_amf_create_string((stu_str_t *) &STU_RTMP_KEY_SWF_URL, nc->url.data, nc->url.len);
	stu_rtmp_amf_add_item_to_object(ao_prop, ao_item);

	ao_item = stu_rtmp_amf_create_string((stu_str_t *) &STU_RTMP_KEY_TC_URL, nc->url.data, nc->url.len);
	stu_rtmp_amf_add_item_to_object(ao_prop, ao_item);

	ao_item = stu_rtmp_amf_create_bool((stu_str_t *) &STU_RTMP_KEY_FPAD, FALSE);
	stu_rtmp_amf_add_item_to_object(ao_prop, ao_item);

	ao_item = stu_rtmp_amf_create_number((stu_str_t *) &STU_RTMP_KEY_AUDIO_CODECS, SUPPORT_SND_AAC);
	stu_rtmp_amf_add_item_to_object(ao_prop, ao_item);

	ao_item = stu_rtmp_amf_create_number((stu_str_t *) &STU_RTMP_KEY_VIDEO_CODECS, SUPPORT_VID_H264);
	stu_rtmp_amf_add_item_to_object(ao_prop, ao_item);

	ao_item = stu_rtmp_amf_create_number((stu_str_t *) &STU_RTMP_KEY_VIDEO_FUNCTION, SUPPORT_VID_CLIENT_SEEK);
	stu_rtmp_amf_add_item_to_object(ao_prop, ao_item);

	ao_item = stu_rtmp_amf_create_number((stu_str_t *) &STU_RTMP_KEY_OBJECT_ENCODING, STU_RTMP_AMF0);
	stu_rtmp_amf_add_item_to_object(ao_prop, ao_item);

	va_start(args, res);

	for (ao_item = va_arg(args, stu_rtmp_amf_t *); ao_item != NULL; /* void */) {
		if (ao_args == NULL) {
			ao_args = stu_rtmp_amf_create_object(NULL);
		}

		stu_rtmp_amf_add_item_to_object(ao_args, ao_item);
	}

	va_end(args);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);
	if (ao_args) {
		pos = stu_rtmp_amf_stringify(pos, ao_args);
	}

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, 0, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d, url=%s.",
				STU_RTMP_CMD_CONNECT.data, c->fd, url);
		stu_rtmp_close_connection(nc);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_args);

	return rc;
}

/*
 * Arguments should end of NULL.
 */
stu_int32_t
stu_rtmp_call(stu_rtmp_netconnection_t *nc, u_char *command, size_t len, stu_rtmp_responder_t *res, ...) {
	stu_connection_t *c;
	stu_rtmp_amf_t   *ao_cmd, *ao_tran, *ao_prop, *ao_args, *ao_item;
	u_char           *pos;
	u_char            tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t       rc;
	va_list           args;

	c = nc->conn;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	if (stu_rtmp_add_responder(nc, res) == STU_ERROR) {
		stu_log_error(0, "Failed to add responder: fd=%d, id=%d.", c->fd, nc->transaction_id);
		return STU_ERROR;
	}

	ao_cmd = stu_rtmp_amf_create_string(NULL, command, len);
	ao_tran = stu_rtmp_amf_create_number(NULL, res ? nc->transaction_id++ : 0);
	ao_prop = stu_rtmp_amf_create_null(NULL);
	ao_args = stu_rtmp_amf_create_object(NULL);

	va_start(args, res);

	for (ao_item = va_arg(args, stu_rtmp_amf_t *); ao_item != NULL; /* void */) {
		stu_rtmp_amf_add_item_to_object(ao_args, ao_item);
	}

	va_end(args);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);
	pos = stu_rtmp_amf_stringify(pos, ao_args);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, 0, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d.", command, c->fd);
		stu_rtmp_close_connection(nc);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_args);

	return rc;
}

stu_int32_t
stu_rtmp_close(stu_rtmp_netconnection_t *nc) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_create_stream(stu_rtmp_netconnection_t *nc, stu_rtmp_responder_t *res) {
	stu_connection_t *c;
	stu_rtmp_amf_t   *ao_cmd, *ao_tran, *ao_prop;
	u_char           *pos;
	u_char            tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t       rc;

	c = nc->conn;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	if (stu_rtmp_add_responder(nc, res) == STU_ERROR) {
		stu_log_error(0, "Failed to add responder: fd=%d, id=%d.", c->fd, nc->transaction_id);
		return STU_ERROR;
	}

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_CREATE_STREAM.data, STU_RTMP_CMD_CREATE_STREAM.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, nc->transaction_id++);
	ao_prop = stu_rtmp_amf_create_null(NULL);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, ao_prop);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, 0, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d.", STU_RTMP_CMD_CREATE_STREAM.data, c->fd);
		stu_rtmp_close_connection(nc);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);
	stu_rtmp_amf_delete(ao_prop);

	return rc;
}


stu_int32_t
stu_rtmp_set_chunk_size(stu_rtmp_netconnection_t *nc, stu_int32_t size) {
	stu_connection_t *c;
	u_char           *pos;
	u_char            tmp[4];
	stu_int32_t       rc;

	c = nc->conn;
	pos = tmp;
	stu_memzero(tmp, 4);

	*(stu_uint32_t *) pos = stu_endian_32(size);
	pos += 4;

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_PROTOCOL_CONTROL, 0, STU_RTMP_MESSAGE_TYPE_SET_CHUNK_SIZE, 0, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send protocol control 0x%02X: fd=%d, size=%d.",
				STU_RTMP_MESSAGE_TYPE_SET_CHUNK_SIZE, c->fd, size);
		stu_rtmp_close_connection(nc);
		goto failed;
	}

	nc->near_chunk_size = size;

	stu_log_debug(4, "set neer_chunk_size: %d.", size);

failed:

	return rc;
}

stu_int32_t
stu_rtmp_send_abort(stu_rtmp_netconnection_t *nc) {
	// TODO: Send abort message.
	return STU_OK;
}

stu_int32_t
stu_rtmp_send_ack_sequence(stu_rtmp_netconnection_t *nc) {
	stu_connection_t *c;
	u_char           *pos;
	u_char            tmp[4];
	stu_int32_t       rc;

	c = nc->conn;
	pos = tmp;
	stu_memzero(tmp, 4);

	*(uint32_t *) pos = stu_endian_32(nc->stat.bytes_in);
	pos += 4;

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_PROTOCOL_CONTROL, 0, STU_RTMP_MESSAGE_TYPE_ACK, 0, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send protocol control 0x%02X: fd=%d, ack=%d.",
				STU_RTMP_MESSAGE_TYPE_ACK, c->fd, nc->stat.bytes_in);
		stu_rtmp_close_connection(nc);
		goto failed;
	}

	stu_log_debug(4, "sent ack sequence: %d.", nc->stat.bytes_in);

failed:

	return rc;
}

stu_int32_t
stu_rtmp_send_user_control(stu_rtmp_netconnection_t *nc, stu_uint16_t event, stu_uint32_t stream_id, stu_uint32_t buf_len, stu_uint32_t timestamp) {
	stu_connection_t *c;
	u_char           *pos;
	u_char            tmp[10];
	stu_int32_t       rc;

	c = nc->conn;
	pos = tmp;
	stu_memzero(tmp, 10);

	*(uint16_t *) pos = stu_endian_16(event);
	pos += 2;

	if (event <= STU_RTMP_EVENT_TYPE_STREAM_IS_RECORDED) {
		*(stu_uint32_t *) pos = stu_endian_32(stream_id);
		pos += 4;
	}

	if (event == STU_RTMP_EVENT_TYPE_SET_BUFFER_LENGTH) {
		*(stu_uint32_t *) pos = stu_endian_32(buf_len);
		pos += 4;
	}

	if (event == STU_RTMP_EVENT_TYPE_PING_REQUEST || event == STU_RTMP_EVENT_TYPE_PING_RESPONSE) {
		*(stu_uint32_t *) pos = stu_endian_32(timestamp);
		pos += 4;
	}

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_PROTOCOL_CONTROL, 0, STU_RTMP_MESSAGE_TYPE_USER_CONTROL, 0, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send protocol control 0x%02X: fd=%d, event=0x%02X.",
				STU_RTMP_MESSAGE_TYPE_USER_CONTROL, c->fd, event);
		stu_rtmp_close_connection(nc);
		goto failed;
	}

	stu_log_debug(4, "sent user control: event=0x%02X.", event);

failed:

	return rc;
}

stu_int32_t
stu_rtmp_set_ack_window_size(stu_rtmp_netconnection_t *nc, stu_uint32_t size) {
	stu_connection_t *c;
	u_char           *pos;
	u_char            tmp[4];
	stu_int32_t       rc;

	c = nc->conn;
	pos = tmp;
	stu_memzero(tmp, 4);

	*(uint32_t *) pos = stu_endian_32(size);
	pos += 4;

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_PROTOCOL_CONTROL, 0, STU_RTMP_MESSAGE_TYPE_ACK_WINDOW_SIZE, 0, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send protocol control 0x%02X: fd=%d, ack=%d.",
				STU_RTMP_MESSAGE_TYPE_ACK_WINDOW_SIZE, c->fd, size);
		stu_rtmp_close_connection(nc);
		goto failed;
	}

	nc->near_ack_window_size = size;

	stu_log_debug(4, "set near_ack_window_size: %d.", size);

failed:

	return rc;
}

stu_int32_t
stu_rtmp_set_peer_bandwidth(stu_rtmp_netconnection_t *nc, stu_uint32_t bandwidth, stu_uint8_t limit) {
	stu_connection_t *c;
	u_char           *pos;
	u_char            tmp[5];
	stu_int32_t       rc;

	c = nc->conn;
	pos = tmp;
	stu_memzero(tmp, 5);

	*(uint32_t *) pos = stu_endian_32(bandwidth);
	pos += 4;

	*pos++ = limit;

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_PROTOCOL_CONTROL, 0, STU_RTMP_MESSAGE_TYPE_BANDWIDTH, 0, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send protocol control 0x%02X: fd=%d, ack=%d, type=0x%02X.",
				STU_RTMP_MESSAGE_TYPE_BANDWIDTH, c->fd, bandwidth, limit);
		stu_rtmp_close_connection(nc);
		goto failed;
	}

	nc->far_bandwidth = bandwidth;
	nc->far_limit_type = limit;

	stu_log_debug(4, "set far_bandwidth: ack=%d, type=0x%02X.", bandwidth, limit);

failed:

	return rc;
}


stu_int32_t
stu_rtmp_send_result(stu_rtmp_netconnection_t *nc, stu_double_t tran, stu_rtmp_amf_t *prop, stu_rtmp_amf_t *info) {
	stu_connection_t *c;
	stu_rtmp_amf_t   *ao_cmd, *ao_tran;
	u_char           *pos;
	u_char            tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t       rc;

	c = nc->conn;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_RESULT.data, STU_RTMP_CMD_RESULT.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, tran);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, prop);
	pos = stu_rtmp_amf_stringify(pos, info);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, 0, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d.", STU_RTMP_CMD_RESULT.data, c->fd);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);

	return rc;
}

stu_int32_t
stu_rtmp_send_error(stu_rtmp_netconnection_t *nc, stu_double_t tran, stu_rtmp_amf_t *prop, stu_rtmp_amf_t *info) {
	stu_connection_t *c;
	stu_rtmp_amf_t   *ao_cmd, *ao_tran;
	u_char           *pos;
	u_char            tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_int32_t       rc;

	c = nc->conn;
	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	ao_cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_ERROR.data, STU_RTMP_CMD_ERROR.len);
	ao_tran = stu_rtmp_amf_create_number(NULL, tran);

	pos = stu_rtmp_amf_stringify(pos, ao_cmd);
	pos = stu_rtmp_amf_stringify(pos, ao_tran);
	pos = stu_rtmp_amf_stringify(pos, prop);
	pos = stu_rtmp_amf_stringify(pos, info);

	rc = stu_rtmp_send_buffer(nc, STU_RTMP_CSID_COMMAND, 0, STU_RTMP_MESSAGE_TYPE_COMMAND, 0, tmp, pos - tmp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to send command \"%s\": fd=%d.", STU_RTMP_CMD_ERROR.data, c->fd);
	}

	stu_rtmp_amf_delete(ao_cmd);
	stu_rtmp_amf_delete(ao_tran);

	return rc;
}

stu_int32_t
stu_rtmp_send_buffer(stu_rtmp_netconnection_t *nc, stu_uint32_t csid, stu_uint32_t timestamp, stu_uint8_t type, stu_uint32_t stream_id, u_char *data, size_t len) {
	stu_connection_t *c;
	u_char           *pos;
	u_char            tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];
	stu_uint8_t       fmt;
	stu_int32_t       n;
	size_t            i, size;

	c = nc->conn;
	fmt = nc->pre_stream_id == stream_id ? 1 : 0;
	nc->pre_stream_id = stream_id;

	for (i = 0; i < len; /* void */) {
		pos = tmp;

		if (csid < 64) {
			*pos++ = fmt << 6 | csid;
		} else if (csid < 320) {
			*pos++ = fmt << 6 | 0x00;
			*pos++ = csid - 64;
		} else if (csid < 65600) {
			*pos++ = fmt << 6 | 0x01;
			*pos++ = csid;
			*pos++ = csid >> 8;
		} else {
			stu_log_error(0, "Failed to write rtmp chunk: CSID \"%d\" out of range.", csid);
			return STU_ERROR;
		}

		if (fmt < 3) {
			if (timestamp < 0xFFFFFF) {
				*pos++ = timestamp >> 16;
				*pos++ = timestamp >> 8;
				*pos++ = timestamp;
			} else {
				*pos++ = 0xFF;
				*pos++ = 0xFF;
				*pos++ = 0xFF;
			}
		}

		if (fmt < 2) {
			*pos++ = len >> 16;
			*pos++ = len >> 8;
			*pos++ = len;

			*pos++ = type;
		}

		if (fmt == 0) {
			*pos++ = stream_id;
			*pos++ = stream_id >> 8;
			*pos++ = stream_id >> 16;
			*pos++ = stream_id >> 24;
		}

		// extended timestamp
		if (timestamp >= 0xFFFFFF) {
			*pos++ = timestamp >> 24;
			*pos++ = timestamp >> 16;
			*pos++ = timestamp >> 8;
			*pos++ = timestamp;
		}

		// payload data
		size = stu_min(len - i, nc->near_chunk_size);
		pos = stu_memcpy(pos, data, size);

		n = c->send(c, tmp, pos - tmp);
		if (n == -1) {
			stu_log_error(stu_errno, "Failed to write rtmp chunk.");
			return STU_ERROR;
		}

		i += size;
		nc->stat.bytes_out += size;

		fmt = 3;
	}

	return STU_OK;
}

stu_rtmp_amf_t *
stu_rtmp_get_information(stu_str_t *level, stu_str_t *code, const char *description) {
	stu_rtmp_amf_t *info, *item;

	info = stu_rtmp_amf_create_object(NULL);

	item = stu_rtmp_amf_create_string(&STU_RTMP_LEVEL, level->data, level->len);
	stu_rtmp_amf_add_item_to_object(info, item);

	item = stu_rtmp_amf_create_string(&STU_RTMP_CODE, code->data, code->len);
	stu_rtmp_amf_add_item_to_object(info, item);

	item = stu_rtmp_amf_create_string(&STU_RTMP_DESCRIPTION, (u_char *) description, stu_strlen(description));
	stu_rtmp_amf_add_item_to_object(info, item);

	return info;
}


stu_int32_t
stu_rtmp_on_connect(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_amf_t           *ao_prop, *ao_info, *ao_item;
	stu_int32_t               rc;

	nc = &r->connection;

	stu_log_debug(4, "Handle command \"%s\": access=%s, url=%s.", STU_RTMP_CMD_CONNECT.data, nc->read_access.data, nc->url.data);

	if (nc->read_access.len == 0 || stu_strncasecmp(nc->read_access.data, (u_char *) "/", 1) == 0) {
		rc = stu_rtmp_accept(nc);
		ao_info = stu_rtmp_get_information(&STU_RTMP_LEVEL_STATUS, &STU_RTMP_CODE_NETCONNECTION_CONNECT_SUCCESS, "connect success");
	} else {
		rc = stu_rtmp_reject(nc);
		ao_info = stu_rtmp_get_information(&STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETCONNECTION_CONNECT_REJECTED, "connect reject");
	}

	ao_prop = stu_rtmp_amf_duplicate(stu_rtmp_properties, TRUE);

	ao_item = stu_rtmp_amf_create_number((stu_str_t *) &STU_RTMP_KEY_OBJECT_ENCODING, nc->object_encoding);
	stu_rtmp_amf_add_item_to_object(ao_info, ao_item);

	ao_item = stu_rtmp_amf_duplicate(stu_rtmp_version, TRUE);
	stu_rtmp_amf_set_key(ao_item, STU_RTMP_KEY_DATA.data, STU_RTMP_KEY_DATA.len);
	stu_rtmp_amf_add_item_to_object(ao_info, ao_item);

	if (stu_rtmp_send_result(nc, r->transaction_id, ao_prop, ao_info) == STU_ERROR) {
		rc = STU_ERROR;
		stu_log_error(0, "Failed to handle command \"%s\": fd=%d.", STU_RTMP_CMD_CONNECT.data, nc->conn->fd);
	}

	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_info);

	return rc;
}

stu_int32_t
stu_rtmp_on_close(stu_rtmp_request_t *r) {
	// TODO: Handle close command.
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_create_stream(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_netstream_t     *ns;
	stu_rtmp_amf_t           *ao_prop, *ao_info;
	const char               *desc;

	nc = &r->connection;

	stu_log_debug(4, "Handle command \"%s\": access=%s, url=%s.", STU_RTMP_CMD_CREATE_STREAM.data, nc->read_access.data, nc->url.data);

	if (nc->stream_id > STU_RTMP_NETSTREAM_MAXIMAM) {
		desc = "too many streams created";
		goto failed;
	}

	if (nc->read_access.len == 0 || stu_strncasecmp(nc->read_access.data, (u_char *) "/", 1) == 0) {
		ns = stu_pcalloc(nc->conn->pool, sizeof(stu_rtmp_netstream_t));
		if (ns) {
			ns->id = nc->stream_id++;
			ns->on_status = NULL;
			r->streams[ns->id - 1] = ns;

			stu_rtmp_attach(ns, nc);

			ao_prop = stu_rtmp_amf_create_null(NULL);
			ao_info = stu_rtmp_amf_create_number(NULL, ns->id);

			if (stu_rtmp_send_result(nc, r->transaction_id, ao_prop, ao_info) == STU_ERROR) {
				stu_log_error(0, "Failed to handle command \"%s\": fd=%d.", STU_RTMP_CMD_CREATE_STREAM.data, nc->conn->fd);
			}

			goto done;
		}
	}

	desc = "failed to create stream";

failed:

	ao_prop = stu_rtmp_amf_create_null(NULL);
	ao_info = stu_rtmp_get_information(&STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETSTREAM_FAILED, desc);

	if (stu_rtmp_send_error(nc, r->transaction_id, ao_prop, ao_info) == STU_ERROR) {
		stu_log_error(0, "Failed to handle command \"%s\": fd=%d.", STU_RTMP_CMD_CREATE_STREAM.data, nc->conn->fd);
	}

done:

	stu_rtmp_amf_delete(ao_prop);
	stu_rtmp_amf_delete(ao_info);

	return STU_OK;
}

stu_int32_t
stu_rtmp_on_result(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_responder_t     *res;
	u_char                    tmp[10];
	stu_str_t                 key;
	stu_uint32_t              hk;

	nc = &r->connection;
	stu_memzero(tmp, 10);

	key.data = tmp;
	key.len = stu_sprintf(tmp, "%u", r->transaction_id) - tmp;

	hk = stu_hash_key(key.data, key.len, nc->responders.flags);

	res = stu_hash_find_locked(&nc->responders, hk, key.data, key.len);
	if (res) {
		if (res->result) {
			res->result(r);
		}

		if (nc->responders.destroyed == FALSE) {
			stu_hash_remove_locked(&nc->responders, hk, key.data, key.len);
			stu_free(res);
		}
	}

	return STU_OK;
}

stu_int32_t
stu_rtmp_on_error(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_responder_t     *res;
	u_char                    tmp[10];
	stu_str_t                 key;
	stu_uint32_t              hk;

	nc = &r->connection;
	stu_memzero(tmp, 10);

	key.data = tmp;
	key.len = stu_sprintf(tmp, "%u", r->transaction_id) - tmp;

	hk = stu_hash_key(key.data, key.len, nc->responders.flags);

	res = stu_hash_find_locked(&nc->responders, hk, key.data, key.len);
	if (res) {
		if (res->status) {
			res->status(r);
		}

		stu_hash_remove_locked(&nc->responders, hk, key.data, key.len);
		stu_free(res);
	}

	return STU_OK;
}


void
stu_rtmp_close_connection(stu_rtmp_netconnection_t *nc) {
	stu_hash_destroy(&nc->commands, NULL);
	stu_hash_destroy(&nc->netstreams, NULL);
	stu_hash_destroy(&nc->responders, stu_free);

	stu_connection_close(nc->conn);
}


static stu_int32_t
stu_rtmp_add_responder(stu_rtmp_netconnection_t *nc, stu_rtmp_responder_t *res) {
	stu_connection_t *c;
	u_char            tmp[10];
	stu_str_t         key;

	c = nc->conn;
	stu_memzero(tmp, 10);

	if (res) {
		key.data = tmp;
		key.len = stu_sprintf(tmp, "%u", nc->transaction_id) - tmp;

		if (stu_hash_insert_locked(&nc->responders, &key, res) == STU_ERROR) {
			stu_log_error(0, "Failed to insert responder into hash: fd=%d, id=%d.", c->fd, nc->transaction_id);
			return STU_ERROR;
		}

		res->transaction_id = nc->transaction_id;
	}

	return STU_OK;
}
