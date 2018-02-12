/*
 * stu_rtmp_netstream.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

static stu_int32_t  stu_rtmp_netstream_send_status(stu_rtmp_netstream_t *ns, stu_rtmp_amf_t *info);


void
stu_rtmp_netstream_attach(stu_rtmp_netstream_t *ns, stu_rtmp_stream_t *stream) {
	ns->stream = stream;
}


stu_int32_t
stu_rtmp_on_play(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_play2(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_delete_stream(stu_rtmp_netstream_t *ns) {
	stu_rtmp_stream_close(ns->stream);
	stu_rtmp_stream_clear(ns->stream);
	ns->stream = NULL;

	return STU_OK;
}

stu_int32_t
stu_rtmp_on_close_stream(stu_rtmp_netstream_t *ns) {
	stu_rtmp_stream_clear(ns->stream);
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_receive_audio(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_receive_video(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_publish(stu_rtmp_netstream_t *ns) {
	stu_rtmp_amf_t             *info;
	stu_rtmp_netconnection_t   *nc;
	stu_rtmp_request_t         *r;
	stu_rtmp_command_message_t *m;
	stu_hash_t                 *hash;
	stu_rtmp_stream_t          *stream;
	stu_uint32_t                hk;

	nc = ns->nc;
	r = nc->connection->request;
	m = r->message;

	hash = &nc->instance->streams;

	if (stu_strncasecmp(nc->write_access.data, (u_char *) "/", 1) != 0 &&
			stu_strncasecmp(nc->write_access.data + 1, nc->app_name.data, nc->app_name.len) != 0) {
		stu_log_error(0, "Failed to handle \"publish\" command: Access denied.");
		info = stu_rtmp_get_information(&STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETSTREAM_FAILED, "write access denied");
		goto failed;
	}

	if (ns->stream->name.data == NULL) {
		ns->stream->name.data = stu_pcalloc(nc->connection->pool, m->publishing_name.len + 1);
		if (ns->stream->name.data == NULL) {
			stu_log_error(0, "Failed to malloc rtmp stream name.");
			info = stu_rtmp_get_information(&STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETSTREAM_FAILED, "internal error");
			goto failed;
		}

		(void) stu_memcpy(ns->stream->name.data, m->publishing_name.data, m->publishing_name.len);
		ns->stream->name.len = m->publishing_name.len;
	}

	hk = stu_hash_key(m->publishing_name.data, m->publishing_name.len, hash->flags);

	stu_mutex_lock(&hash->lock);

	stream = stu_hash_find_locked(hash, hk, m->publishing_name.data, m->publishing_name.len);
	if (stream == NULL) {
		stream = hash->hooks.malloc_fn(sizeof(stu_rtmp_stream_t));
		if (stream == NULL) {
			stu_log_error(0, "Failed to malloc rtmp stream.");
			info = stu_rtmp_get_information(&STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETSTREAM_FAILED, "internal error");
			goto failed;
		}

		stream->name.data = hash->hooks.malloc_fn(m->publishing_name.len + 1);
		if (stream->name.data == NULL) {
			stu_log_error(0, "Failed to malloc rtmp stream name.");
			info = stu_rtmp_get_information(&STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETSTREAM_FAILED, "internal error");
			goto failed;
		}

		(void) stu_memcpy(stream->name.data, m->publishing_name.data, m->publishing_name.len);
		stream->name.len = m->publishing_name.len;

		if (stu_hash_insert_locked(hash, &stream->name, stream) == STU_ERROR) {
			stu_log_error(0, "Failed to insert rtmp stream.");
			info = stu_rtmp_get_information(&STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETSTREAM_FAILED, "internal error");
			goto failed;
		}
	}

	if (stream->type & STU_RTMP_STREAM_TYPE_PUBLISHING) {
		info = stu_rtmp_get_information(&STU_RTMP_LEVEL_ERROR, &STU_RTMP_CODE_NETSTREAM_PUBLISH_BADNAME, "publish bad name");
	} else {
		stream->type |= STU_RTMP_STREAM_TYPE_PUBLISHING;
		info = stu_rtmp_get_information(&STU_RTMP_LEVEL_STATUS, &STU_RTMP_CODE_NETSTREAM_PUBLISH_START, "publish start");
	}

failed:

	stu_mutex_unlock(&hash->lock);

	stu_rtmp_netstream_send_status(ns, info);

	stu_rtmp_amf_delete(info);

	return STU_OK;
}

stu_int32_t
stu_rtmp_on_seek(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_pause(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_status(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}


stu_int32_t
stu_rtmp_on_set_buffer_length(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}


stu_int32_t
stu_rtmp_on_set_data_frame(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_clear_data_frame(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_audio_frame(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}

stu_int32_t
stu_rtmp_on_video_frame(stu_rtmp_netstream_t *ns) {
	return STU_OK;
}


static stu_int32_t
stu_rtmp_netstream_send_status(stu_rtmp_netstream_t *ns, stu_rtmp_amf_t *info) {
	stu_rtmp_amf_t           *cmd, *tran, *prop;
	stu_rtmp_netconnection_t *nc;
	u_char                   *pos;
	u_char                    tmp[STU_RTMP_REQUEST_DEFAULT_SIZE];

	nc = ns->nc;

	pos = tmp;
	stu_memzero(tmp, STU_RTMP_REQUEST_DEFAULT_SIZE);

	cmd = stu_rtmp_amf_create_string(NULL, STU_RTMP_CMD_ON_STATUS.data, STU_RTMP_CMD_ON_STATUS.len);
	tran = stu_rtmp_amf_create_number(NULL, 0);
	prop = stu_rtmp_amf_create_null(NULL);

	pos = stu_rtmp_amf_stringify(pos, cmd);
	pos = stu_rtmp_amf_stringify(pos, tran);
	pos = stu_rtmp_amf_stringify(pos, prop);
	pos = stu_rtmp_amf_stringify(pos, info);

	if (stu_rtmp_send_buffer(nc, tmp, pos - tmp) == STU_ERROR) {
		stu_rtmp_close_connection(nc->connection);
	}

	stu_rtmp_amf_delete(cmd);
	stu_rtmp_amf_delete(tran);
	stu_rtmp_amf_delete(prop);

	return STU_OK;
}
