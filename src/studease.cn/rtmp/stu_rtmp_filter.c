/*
 * stu_rtmp_filter.c
 *
 *  Created on: 2018骞�1鏈�24鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_hash_t          stu_rtmp_filter_hash;

extern stu_hash_t   stu_rtmp_command_listener_hash;

static stu_rtmp_filter_t  rtmp_filter[] = {
	{ stu_string(""), &stu_rtmp_command_listener_hash },
	{ stu_null_string, NULL }
};


stu_int32_t
stu_rtmp_filter_init_hash() {
	stu_rtmp_filter_t *f;

	if (stu_hash_init(&stu_rtmp_filter_hash, STU_RTMP_FILTER_MAX_RECORDS, NULL, STU_HASH_FLAGS_LOWCASE) == STU_ERROR) {
		return STU_ERROR;
	}

	for (f = rtmp_filter; f->listeners; f++) {
		if (stu_rtmp_filter_add(&f->pattern, f->listeners) == STU_ERROR) {
			return STU_ERROR;
		}
	}

	return STU_OK;
}

stu_int32_t
stu_rtmp_filter_add(stu_str_t *pattern, stu_hash_t *listeners) {
	stu_rtmp_filter_t *f;
	stu_int32_t        rc;

	rc = STU_ERROR;

	f = stu_calloc(sizeof(stu_rtmp_filter_t));
	if (f == NULL) {
		goto failed;
	}

	f->pattern.data = stu_calloc(pattern->len + 1);
	if (f->pattern.data == NULL) {
		goto failed;
	}

	stu_strncpy(f->pattern.data, pattern->data, pattern->len);
	f->pattern.len = pattern->len;

	f->listeners = listeners;

	stu_mutex_lock(&stu_rtmp_filter_hash.lock);

	if (stu_hash_insert_locked(&stu_rtmp_filter_hash, pattern, f) == STU_ERROR) {
		goto failed;
	}

	rc = STU_OK;

failed:

	stu_mutex_unlock(&stu_rtmp_filter_hash.lock);

	return rc;
}

stu_int32_t
stu_rtmp_filter_del(stu_str_t *pattern) {
	stu_rtmp_filter_t *f;
	stu_uint32_t       hk;

	stu_mutex_lock(&stu_rtmp_filter_hash.lock);

	hk = stu_hash_key(pattern->data, pattern->len, stu_rtmp_filter_hash.flags);

	f = stu_hash_remove_locked(&stu_rtmp_filter_hash, hk, pattern->data, pattern->len);
	if (f) {
		stu_free(f->pattern.data);
		stu_free(f);
	}

	stu_mutex_unlock(&stu_rtmp_filter_hash.lock);

	return STU_OK;
}
