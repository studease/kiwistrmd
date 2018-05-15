/*
 * stu_rtmp_phase.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_list_t  stu_rtmp_phases;


stu_int32_t
stu_rtmp_phase_init() {
	stu_list_init(&stu_rtmp_phases, NULL);
	return STU_OK;
}


stu_int32_t
stu_rtmp_phase_add(stu_str_t *name, stu_rtmp_phase_handler_pt handler) {
	stu_rtmp_phase_t *ph;

	ph = stu_calloc(sizeof(stu_rtmp_phase_t));
	if (ph == NULL) {
		stu_log_error(0, "Failed to calloc rtmp phase: name=%s.", name->data);
		return STU_ERROR;
	}

	ph->name.data = stu_calloc(name->len + 1);
	if (ph->name.data == NULL) {
		return STU_ERROR;
	}

	stu_strncpy(ph->name.data, name->data, name->len);
	ph->name.len = name->len;

	ph->handler = handler;

	if (stu_list_insert_tail(&stu_rtmp_phases, ph) == NULL) {
		stu_log_error(0, "Failed to insert rtmp phase: name=%s.", name->data);
		return STU_ERROR;
	}

	return STU_OK;
}

stu_int32_t
stu_rtmp_phase_del(stu_str_t *name) {
	stu_rtmp_phase_t *ph;
	stu_list_elt_t   *elts, *e;
	stu_queue_t      *q;

	elts = &stu_rtmp_phases.elts;

	for (q = stu_queue_head(&elts->queue); q != stu_queue_sentinel(&elts->queue); q = stu_queue_next(q)) {
		e = stu_queue_data(q, stu_list_elt_t, queue);
		ph = e->value;

		if (stu_strncasecmp(ph->name.data, name->data, name->len) == 0) {
			stu_queue_remove(&e->queue);
			stu_rtmp_phases.length--;

			stu_free(ph->name.data);
			stu_free(ph);
			break;
		}
	}

	return STU_OK;
}
