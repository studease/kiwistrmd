/*
 * stu_flv.c
 *
 *  Created on: 2018年5月17日
 *      Author: Tony Lau
 */

#include "stu_format.h"

static u_char  stu_flv_header[] = "FLV\x1\x5\0\0\0\x9\0\0\0\0";
static u_char  stu_flv_footer[] = "\x9\0\0\x5\0\0\0\0\0\0\0\x17\x2\0\0\0\0\0\0\x10";


stu_int32_t
stu_flv_open(stu_flv_t *flv, u_long mode, u_long create, u_long access) {
	stu_file_t *file;

	file = &flv->file;

	if (stu_create_path(file) == STU_ERROR) {
		stu_log_error(0, "Failed to create flv path.");
		return STU_ERROR;
	}

	file->fd = stu_file_open(file->name.data, mode, create, access);
	if (file->fd == STU_FILE_INVALID) {
		stu_log_error(stu_errno, "Failed to " stu_file_open_n " flv file \"%s\".", file->name.data);
		return STU_ERROR;
	}

	return STU_OK;
}

stu_int32_t
stu_flv_write_header(stu_flv_t *flv) {
	stu_file_t  *file;
	stu_int32_t  rc;

	file = &flv->file;

	rc = stu_file_write(file, stu_flv_header, STU_FLV_DATA_OFFSET, 0);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to write flv header: %s.", file->name.data);
	}

	return rc;
}

stu_int32_t
stu_flv_write_tag(stu_flv_t *flv, stu_uint8_t type, stu_uint32_t ts, void *buf, size_t n) {
	stu_file_t   *file;
	u_char       *pos;
	u_char        tmp[STU_FLV_TAG_HEADER];
	stu_uint32_t  size;

	file = &flv->file;
	pos = tmp;

	flv->timestamp += ts;

	*pos++ = type;
	*pos++ = n >> 16;
	*pos++ = n >> 8;
	*pos++ = n;
	*pos++ = flv->timestamp >> 16;
	*pos++ = flv->timestamp >> 8;
	*pos++ = flv->timestamp;
	*pos++ = flv->timestamp >> 24;
	*pos++ = 0;
	*pos++ = 0;
	*pos++ = 0;

	if (stu_file_write(file, tmp, STU_FLV_TAG_HEADER, file->offset) == STU_ERROR) {
		stu_log_error(0, "Failed to write flv tag header: %s.", file->name.data);
		return STU_ERROR;
	}

	if (stu_file_write(file, buf, n, file->offset) == STU_ERROR) {
		stu_log_error(0, "Failed to write flv tag data: %s.", file->name.data);
		return STU_ERROR;
	}

	pos = tmp;
	size = STU_FLV_TAG_HEADER + n;

	*pos++ = size >> 24;
	*pos++ = size >> 16;
	*pos++ = size >> 8;
	*pos++ = size;

	if (stu_file_write(file, tmp, 4, file->offset) == STU_ERROR) {
		stu_log_error(0, "Failed to write flv tag size: %s.", file->name.data);
		return STU_ERROR;
	}

	return STU_OK;
}

stu_int32_t
stu_flv_close(stu_flv_t *flv, stu_uint32_t ts) {
	stu_file_t *file;
	u_char     *pos;

	file = &flv->file;
	pos = stu_flv_footer + 4;

	flv->timestamp += ts;

	*pos++ = flv->timestamp >> 16;
	*pos++ = flv->timestamp >> 8;
	*pos++ = flv->timestamp;
	*pos++ = flv->timestamp >> 24;

	if (stu_file_write(file, stu_flv_footer, STU_FLV_TAG_FOOTER, file->offset) == STU_ERROR) {
		stu_log_error(0, "Failed to write flv tag footer: %s.", file->name.data);
		return STU_ERROR;
	}

	return STU_OK;
}
