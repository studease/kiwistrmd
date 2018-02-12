/*
 * stu_rtmp_instance.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_str_t  stu_rtmp_definst = stu_string("_definst_");


stu_int32_t
stu_instance_insert(stu_hash_t *hash, stu_rtmp_netconnection_t *nc) {
	stu_int32_t  rc;

	stu_mutex_lock(&hash->lock);
	rc = stu_instance_insert_locked(hash, nc);
	stu_mutex_unlock(&hash->lock);

	return rc;
}

stu_int32_t
stu_instance_insert_locked(stu_hash_t *hash, stu_rtmp_netconnection_t *nc) {
	stu_rtmp_instance_t *inst;
	stu_uint32_t         hk;

	hk = stu_hash_key(nc->inst_name.data, nc->inst_name.len, hash->flags);

	inst = stu_hash_find_locked(hash, hk, nc->inst_name.data, nc->inst_name.len);
	if (inst == NULL) {
		inst = hash->hooks.malloc_fn(sizeof(stu_rtmp_instance_t));
		if (inst == NULL) {
			stu_log_error(0, "Failed to malloc rtmp instance: %s.", nc->inst_name.data);
			return STU_ERROR;
		}

		inst->name.data = hash->hooks.malloc_fn(nc->inst_name.len + 1);
		if (inst->name.data == NULL) {
			stu_log_error(0, "Failed to malloc rtmp instance name: %s.", nc->inst_name.data);
			return STU_ERROR;
		}

		stu_strncpy(inst->name.data, nc->inst_name.data, nc->inst_name.len);
		inst->name.len = nc->inst_name.len;

		stu_hash_init(&inst->connections, STU_INSTANCE_LIST_DEFAULT_SIZE, &hash->hooks, hash->flags);
		stu_hash_init(&inst->streams, STU_INSTANCE_LIST_DEFAULT_SIZE, &hash->hooks, hash->flags);

		if (stu_hash_insert_locked(hash, &nc->inst_name, inst) == STU_ERROR) {
			stu_log_error(0, "Failed to insert rtmp instance: %s.", nc->inst_name.data);
			return STU_ERROR;
		}
	}

	nc->instance = inst;

	return stu_hash_insert(&inst->connections, &nc->far_id, nc);
}

void
stu_instance_remove(stu_hash_t *hash, stu_rtmp_netconnection_t *nc) {
	stu_mutex_lock(&hash->lock);
	stu_instance_insert_locked(hash, nc);
	stu_mutex_unlock(&hash->lock);
}

void
stu_instance_remove_locked(stu_hash_t *hash, stu_rtmp_netconnection_t *nc) {
	stu_rtmp_instance_t *inst;
	stu_uint32_t         hk;

	hk = stu_hash_key(nc->inst_name.data, nc->inst_name.len, hash->flags);

	inst = stu_hash_find_locked(hash, hk, nc->inst_name.data, nc->inst_name.len);
	if (inst) {
		stu_hash_remove(&inst->connections, hk, nc->far_id.data, nc->far_id.len);
	}
}
