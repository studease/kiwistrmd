/*
 * stu_rtmp_instance.c
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_str_t  stu_rtmp_definst = stu_string("_definst_");


stu_int32_t
stu_rtmp_instance_init(stu_rtmp_instance_t *inst, u_char *name, size_t len) {
	if (inst->name.data) {
		stu_rtmp_apps.hooks.free_fn(inst->name.data);
		stu_str_null(&inst->name);
	}

	if (name && len) {
		inst->name.data = stu_rtmp_apps.hooks.malloc_fn(len + 1);
		if (inst->name.data == NULL) {
			stu_log_error(0, "Failed to malloc rtmp instance name: %s.", name);
			return STU_ERROR;
		}

		stu_strncpy(inst->name.data, name, len);
		inst->name.len = len;
	}

	if (stu_hash_init(&inst->connections, STU_RTMP_INST_DEFAULT_SIZE, &stu_rtmp_apps.hooks, stu_rtmp_apps.flags) == STU_ERROR) {
		stu_log_error(0, "Failed to init connection hash of rtmp instance: %s.", inst->name.data);
		return STU_ERROR;
	}

	if (stu_hash_init(&inst->streams, STU_RTMP_INST_DEFAULT_SIZE, &stu_rtmp_apps.hooks, stu_rtmp_apps.flags) == STU_ERROR) {
		stu_log_error(0, "Failed to init stream hash of rtmp instance: %s.", inst->name.data);
		return STU_ERROR;
	}

	return STU_OK;
}

void
stu_rtmp_instance_cleanup(stu_rtmp_instance_t *inst) {
	stu_mutex_lock(&inst->lock);
	stu_hash_destroy_locked(&inst->connections, (stu_hash_cleanup_pt) stu_rtmp_close_connection);
	stu_hash_destroy_locked(&inst->streams, (stu_hash_cleanup_pt) stu_rtmp_close_stream);
	stu_mutex_unlock(&inst->lock);
}
