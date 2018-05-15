/*
 * stu_rtmp_application.c
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

static stu_int32_t  stu_rtmp_application_start(stu_rtmp_application_t *app);
static stu_int32_t  stu_rtmp_application_stop(stu_rtmp_application_t *app);

stu_hash_t   stu_rtmp_apps;
stu_queue_t  stu_rtmp_app_freed;

stu_rtmp_application_handler_pt  stu_rtmp_application_on_start = stu_rtmp_application_start;
stu_rtmp_application_handler_pt  stu_rtmp_application_on_stop = stu_rtmp_application_stop;


stu_int32_t
stu_rtmp_application_init_hash() {
	stu_queue_init(&stu_rtmp_app_freed);

	if (stu_hash_init(&stu_rtmp_apps, STU_RTMP_APP_LIST_DEFAULT_SIZE, NULL, STU_HASH_FLAGS_LOWCASE) == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp application hash.");
		return STU_ERROR;
	}

	return STU_OK;
}


stu_int32_t
stu_rtmp_application_init(stu_rtmp_application_t *app, u_char *name, size_t len) {
	if (app->name.data) {
		stu_rtmp_apps.hooks.free_fn(app->name.data);
		stu_str_null(&app->name);
	}

	if (name && len) {
		app->name.data = stu_rtmp_apps.hooks.malloc_fn(len + 1);
		if (app->name.data == NULL) {
			stu_log_error(0, "Failed to malloc rtmp application name: %s.", name);
			return STU_ERROR;
		}

		stu_strncpy(app->name.data, name, len);
		app->name.len = len;
	}

	if (stu_hash_init(&app->instances, STU_RTMP_APP_DEFAULT_SIZE, &stu_rtmp_apps.hooks, stu_rtmp_apps.flags) == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp instance hash: %s.", app->name.data);
		return STU_ERROR;
	}

	if (stu_rtmp_application_on_start(app) == STU_ERROR) {
		stu_log_error(0, "Failed to start rtmp application: %s.", app->name.data);
		return STU_ERROR;
	}

	return STU_OK;
}

void
stu_rtmp_application_cleanup(stu_rtmp_application_t *app) {
	stu_uint32_t  hk;

	// remove application
	stu_mutex_lock(&stu_rtmp_apps.lock);

	hk = stu_hash_key(app->name.data, app->name.len, stu_rtmp_apps.flags);
	stu_hash_remove_locked(&stu_rtmp_apps, hk, app->name.data, app->name.len);

	// cleanup
	stu_mutex_lock(&app->lock);

	stu_rtmp_application_on_stop(app);

	stu_hash_destroy_locked(&app->instances, (stu_hash_cleanup_pt) stu_rtmp_instance_cleanup);

	if (app->name.data) {
		stu_rtmp_apps.hooks.free_fn(app->name.data);
		stu_str_null(&app->name);
	}

	stu_mutex_unlock(&app->lock);
	stu_mutex_unlock(&stu_rtmp_apps.lock);
}


stu_int32_t
stu_rtmp_accept(stu_rtmp_netconnection_t *nc) {
	if (stu_rtmp_application_insert(nc) == STU_ERROR) {
		stu_log_error(0, "Failed to insert rtmp connection: app=%s, inst=%s.",
				nc->url.application.data, nc->url.instance.data);
		return STU_ERROR;
	}

	nc->connected = TRUE;

	stu_rtmp_set_ack_window_size(nc, 2500000);
	stu_rtmp_set_peer_bandwidth(nc, 2500000, STU_RTMP_BANDWIDTH_LIMIT_TYPE_DYNAMIC);
	stu_rtmp_send_user_control(nc, STU_RTMP_EVENT_TYPE_STREAM_BEGIN, 0, 0, 0);
	stu_rtmp_set_chunk_size(nc, STU_RTMP_CHUNK_SIZE);

	return STU_OK;
}

stu_int32_t
stu_rtmp_reject(stu_rtmp_netconnection_t *nc) {
	nc->conn->error = TRUE;
	return STU_ERROR;
}


stu_int32_t
stu_rtmp_application_insert(stu_rtmp_netconnection_t *nc) {
	stu_int32_t  rc;

	stu_mutex_lock(&stu_rtmp_apps.lock);
	rc = stu_rtmp_application_insert_locked(nc);
	stu_mutex_unlock(&stu_rtmp_apps.lock);

	return rc;
}

stu_int32_t
stu_rtmp_application_insert_locked(stu_rtmp_netconnection_t *nc) {
	stu_rtmp_application_t *app;
	stu_rtmp_instance_t    *inst;
	stu_uint32_t            hk;
	stu_int32_t             rc;

	rc = STU_ERROR;

	// find app
	hk = stu_hash_key(nc->url.application.data, nc->url.application.len, stu_rtmp_apps.flags);

	app = stu_hash_find_locked(&stu_rtmp_apps, hk, nc->url.application.data, nc->url.application.len);
	if (app == NULL) {
		app = stu_rtmp_apps.hooks.malloc_fn(sizeof(stu_rtmp_application_t));
		if (app == NULL) {
			stu_log_error(0, "Failed to malloc rtmp application: %s.", nc->url.application.data);
			return STU_ERROR;
		}

		if (stu_rtmp_application_init(app, nc->url.application.data, nc->url.application.len) == STU_ERROR) {
			stu_log_error(0, "Failed to init rtmp application: %s.", nc->url.application.data);
			return STU_ERROR;
		}

		if (stu_hash_insert_locked(&stu_rtmp_apps, &nc->url.application, app) == STU_ERROR) {
			stu_log_error(0, "Failed to insert rtmp application: %s.", nc->url.application.data);
			return STU_ERROR;
		}
	}

	stu_mutex_lock(&app->lock);

	// find inst
	hk = stu_hash_key(nc->url.instance.data, nc->url.instance.len, app->instances.flags);

	inst = stu_hash_find_locked(&app->instances, hk, nc->url.instance.data, nc->url.instance.len);
	if (inst == NULL) {
		inst = app->instances.hooks.malloc_fn(sizeof(stu_rtmp_instance_t));
		if (inst == NULL) {
			stu_log_error(0, "Failed to malloc rtmp instance: %s.", nc->url.instance.data);
			goto failed;
		}

		if (stu_rtmp_instance_init(inst, nc->url.instance.data, nc->url.instance.len) == STU_ERROR) {
			stu_log_error(0, "Failed to init rtmp instance: %s.", nc->url.instance.data);
			goto failed;
		}

		if (stu_hash_insert_locked(&app->instances, &nc->url.instance, inst) == STU_ERROR) {
			stu_log_error(0, "Failed to insert rtmp instance: %s.", nc->url.instance.data);
			goto failed;
		}
	}

	// insert conn
	stu_mutex_lock(&inst->lock);

	rc = stu_hash_insert_locked(&inst->connections, &nc->id, nc);
	if (rc == STU_OK) {
		nc->application = app;
		nc->instance = inst;
	}

	stu_mutex_unlock(&inst->lock);

failed:

	stu_mutex_unlock(&app->lock);

	return rc;
}

void
stu_rtmp_application_remove(stu_rtmp_netconnection_t *nc) {
	stu_mutex_lock(&stu_rtmp_apps.lock);
	stu_rtmp_application_remove_locked(nc);
	stu_mutex_unlock(&stu_rtmp_apps.lock);
}

void
stu_rtmp_application_remove_locked(stu_rtmp_netconnection_t *nc) {
	stu_rtmp_application_t *app;
	stu_rtmp_instance_t    *inst;
	stu_uint32_t            hk;

	hk = stu_hash_key(nc->url.application.data, nc->url.application.len, stu_rtmp_apps.flags);

	app = stu_hash_find_locked(&stu_rtmp_apps, hk, nc->url.application.data, nc->url.application.len);
	if (app == NULL) {
		return;
	}

	stu_mutex_lock(&app->lock);

	hk = stu_hash_key(nc->url.instance.data, nc->url.instance.len, app->instances.flags);

	inst = stu_hash_find_locked(&app->instances, hk, nc->url.instance.data, nc->url.instance.len);
	if (inst) {
		stu_mutex_lock(&inst->lock);
		stu_hash_remove_locked(&inst->connections, hk, nc->id.data, nc->id.len);
		stu_mutex_unlock(&inst->lock);
	}

	stu_mutex_unlock(&app->lock);
}

static stu_int32_t
stu_rtmp_application_start(stu_rtmp_application_t *app) {
	stu_log("rtmp application start: %s", app->name.data);
	return STU_OK;
}

static stu_int32_t
stu_rtmp_application_stop(stu_rtmp_application_t *app) {
	stu_log("rtmp application stop: %s", app->name.data);
	return STU_OK;
}
