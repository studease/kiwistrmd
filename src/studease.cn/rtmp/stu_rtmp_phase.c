/*
 * stu_rtmp_phase.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_hash_t  stu_rtmp_phases;

stu_rtmp_phase_listener_t  stu_rtmp_phase_listeners[] = {
	{ stu_string("flv"), stu_rtmp_phase_flv_handler, stu_rtmp_phase_flv_close },
	{ stu_string("fmp4"), stu_rtmp_phase_fmp4_handler, stu_rtmp_phase_fmp4_close },
	{ stu_null_string, NULL, NULL }
};

extern stu_hash_t  stu_rtmp_phase_listener_hash;

static stu_rtmp_phase_t  rtmp_phases[] = {
	{ stu_string(""), &stu_rtmp_phase_listener_hash },
	{ stu_null_string, NULL }
};


stu_int32_t
stu_rtmp_phase_init() {
	stu_rtmp_phase_t *ph;

	if (stu_hash_init(&stu_rtmp_phases, STU_RTMP_PHASE_MAX_RECORDS, NULL, STU_HASH_FLAGS_LOWCASE) == STU_ERROR) {
		return STU_ERROR;
	}

	for (ph = rtmp_phases; ph->listeners; ph++) {
		if (stu_rtmp_phase_add(&ph->pattern, ph->listeners) == STU_ERROR) {
			return STU_ERROR;
		}
	}

	if (stu_rtmp_phase_flv_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp phase flv.");
		return STU_ERROR;
	}

	if (stu_rtmp_phase_fmp4_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp phase fmp4.");
		return STU_ERROR;
	}

	return STU_OK;
}


stu_int32_t
stu_rtmp_phase_add(stu_str_t *pattern, stu_hash_t *listeners) {
	stu_rtmp_phase_t *ph;

	ph = stu_calloc(sizeof(stu_rtmp_phase_t));
	if (ph == NULL) {
		stu_log_error(0, "Failed to calloc rtmp phase: pattern=%s.", pattern->data);
		return STU_ERROR;
	}

	ph->pattern.data = stu_calloc(pattern->len + 1);
	if (ph->pattern.data == NULL) {
		return STU_ERROR;
	}

	stu_strncpy(ph->pattern.data, pattern->data, pattern->len);
	ph->pattern.len = pattern->len;

	ph->listeners = listeners;

	if (stu_hash_insert(&stu_rtmp_phases, pattern, ph) == STU_ERROR) {
		stu_log_error(0, "Failed to insert rtmp phase: pattern=%s.", pattern->data);
		return STU_ERROR;
	}

	return STU_OK;
}

stu_int32_t
stu_rtmp_phase_del(stu_str_t *pattern) {
	stu_rtmp_phase_t *ph;
	stu_uint32_t      hk;

	hk = stu_hash_key(pattern->data, pattern->len, stu_rtmp_phases.flags);

	ph = stu_hash_remove(&stu_rtmp_phases, hk, pattern->data, pattern->len);
	if (ph) {
		stu_free(ph->pattern.data);
		stu_free(ph);
	}

	return STU_OK;
}
