/*
 * stu_rtmp_filter.c
 *
 *  Created on: 2018骞�1鏈�24鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_hash_t          stu_rtmp_filters;

extern stu_hash_t   stu_rtmp_filter_listener_hash;

static stu_rtmp_filter_t  rtmp_filters[] = {
	{ stu_string(""), &stu_rtmp_filter_listener_hash },
	{ stu_null_string, NULL }
};


stu_int32_t
stu_rtmp_filter_init_hash() {
	stu_rtmp_filter_t *f;

	if (stu_hash_init(&stu_rtmp_filters, STU_RTMP_FILTER_MAX_RECORDS, NULL, STU_HASH_FLAGS_LOWCASE) == STU_ERROR) {
		return STU_ERROR;
	}

	for (f = rtmp_filters; f->listeners; f++) {
		if (stu_rtmp_filter_add(&f->pattern, f->listeners) == STU_ERROR) {
			return STU_ERROR;
		}
	}

	return STU_OK;
}

stu_int32_t
stu_rtmp_filter_add(stu_str_t *pattern, stu_hash_t *listeners) {
	stu_rtmp_filter_t *f;

	f = stu_calloc(sizeof(stu_rtmp_filter_t));
	if (f == NULL) {
		return STU_ERROR;
	}

	f->pattern.data = stu_calloc(pattern->len + 1);
	if (f->pattern.data == NULL) {
		return STU_ERROR;
	}

	stu_strncpy(f->pattern.data, pattern->data, pattern->len);
	f->pattern.len = pattern->len;

	f->listeners = listeners;

	if (stu_hash_insert(&stu_rtmp_filters, pattern, f) == STU_ERROR) {
		stu_log_error(0, "Failed to insert rtmp filter: pattern=%s.", pattern->data);
		return STU_ERROR;
	}

	return STU_OK;
}

stu_int32_t
stu_rtmp_filter_del(stu_str_t *pattern) {
	stu_rtmp_filter_t *f;
	stu_uint32_t       hk;

	hk = stu_hash_key(pattern->data, pattern->len, stu_rtmp_filters.flags);

	f = stu_hash_remove(&stu_rtmp_filters, hk, pattern->data, pattern->len);
	if (f) {
		stu_free(f->pattern.data);
		stu_free(f);
	}

	return STU_OK;
}
