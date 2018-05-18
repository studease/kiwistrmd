/*
 * stu_rtmp_flv_recorder.c
 *
 *  Created on: 2018年5月17日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

static stu_flv_t *stu_rtmp_phase_get_flv(stu_rtmp_request_t *r);

extern stu_str_t  stu_rtmp_root;

stu_hash_t        stu_rtmp_phase_flv_hash;


stu_int32_t
stu_rtmp_phase_flv_init() {
	if (stu_hash_init(&stu_rtmp_phase_flv_hash, STU_RTMP_PHASE_FLV_MAXIMUM, NULL, STU_HASH_FLAGS_LOWCASE) == STU_ERROR) {
		return STU_ERROR;
	}

	return STU_OK;
}


stu_int32_t
stu_rtmp_phase_flv_handler(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_chunk_t         *ck;
	stu_flv_t                *flv;
	stu_buf_t                *buf;
	stu_int32_t               rc;

	nc = &r->connection;
	ck = r->chunk_in;
	buf = &ck->payload;

	flv = stu_rtmp_phase_get_flv(r);
	if (flv == NULL) {
		stu_log_error(0, "Failed to get flv: fd=%d.", nc->conn->fd);
		return STU_ERROR;
	}

	if (flv->file.offset == 0 && stu_flv_write_header(flv) == STU_ERROR) {
		return STU_ERROR;
	}

	rc = stu_flv_write_tag(flv, ck->type_id, ck->timestamp, buf->pos, buf->last - buf->pos);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to write flv tag: fd=%d.", nc->conn->fd);
	}

	return rc;
}

stu_int32_t
stu_rtmp_phase_flv_close(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_chunk_t         *ck;
	stu_flv_t                *flv;
	stu_int32_t               rc;

	nc = &r->connection;
	ck = r->chunk_in;

	flv = stu_rtmp_phase_get_flv(r);
	if (flv == NULL) {
		stu_log_error(0, "Failed to get flv: fd=%d.", nc->conn->fd);
		return STU_ERROR;
	}

	rc = stu_flv_close(flv, ck->timestamp);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to close flv: fd=%d.", nc->conn->fd);
	}

	return rc;
}


static stu_flv_t *
stu_rtmp_phase_get_flv(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_netstream_t     *ns;
	stu_flv_t                *flv;
	u_char                   *pos;
	u_char                    tmp[STU_MAX_PATH];
	stu_uint32_t              hk;
	size_t                    len;

	nc = &r->connection;
	pos = tmp;
	stu_memzero(tmp, STU_MAX_PATH);

	ns = stu_rtmp_find_netstream(r, r->chunk_in->stream_id);
	if (ns == NULL) {
		return NULL;
	}

	pos = stu_strncpy(pos, stu_rtmp_root.data, stu_rtmp_root.len);
	*pos++ = '/';

	pos = stu_strncpy(pos, nc->url.application.data, nc->url.application.len);
	*pos++ = '/';

	pos = stu_strncpy(pos, nc->url.instance.data, nc->url.instance.len);
	*pos++ = '/';

	pos = stu_sprintf(pos, "%s.flv", ns->name.data);

	len = pos - tmp;
	hk = stu_hash_key(tmp, len, stu_rtmp_phase_flv_hash.flags);

	flv = stu_hash_find(&stu_rtmp_phase_flv_hash, hk, tmp, len);
	if (flv == NULL) {
		flv = stu_calloc(sizeof(stu_flv_t));
		if (flv == NULL) {
			stu_log_error(0, "Failed to calloc flv: %s.", tmp);
			return NULL;
		}

		flv->file.name.data = stu_calloc(len + 1);
		if (flv->file.name.data == NULL) {
			stu_log_error(0, "Failed to calloc flv file name: %s.", tmp);
			return NULL;
		}

		stu_strncpy(flv->file.name.data, tmp, len);
		flv->file.name.len = len;

		if (stu_flv_open(flv, STU_FILE_RDWR, STU_FILE_TRUNCATE, STU_FILE_DEFAULT_ACCESS) == STU_ERROR) {
			stu_log_error(0, "Failed to " stu_file_open_n " flv file \"%s\".", flv->file.name.data);
			return NULL;
		}

		if (stu_hash_insert(&stu_rtmp_phase_flv_hash, &flv->file.name, flv) == STU_ERROR) {
			stu_log_error(0, "Failed to insert flv: %s.", tmp);
			return NULL;
		}
	}

	return flv;
}
