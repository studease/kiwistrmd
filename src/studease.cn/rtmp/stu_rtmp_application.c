/*
 * stu_rtmp_application.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_hash_t  stu_rtmp_applications;


stu_int32_t
stu_rtmp_application_init_hash() {
	if (stu_hash_init(&stu_rtmp_applications, STU_RTMP_APPLICATION_LIST_DEFAULT_SIZE, NULL, STU_HASH_FLAGS_LOWCASE|STU_HASH_FLAGS_REPLACE) == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp application hash.");
		return STU_ERROR;
	}

	return STU_OK;
}


stu_int32_t
stu_rtmp_accept(stu_rtmp_netconnection_t *nc) {
	if (stu_rtmp_application_insert(nc) == STU_ERROR) {
		stu_log_error(0, "Failed to insert rtmp netconnection: app=%s, inst=%s.", nc->app_name.data, nc->inst_name.data);
		return STU_ERROR;
	}

	// TODO

	return STU_OK;
}

stu_int32_t
stu_rtmp_reject(stu_rtmp_netconnection_t *nc) {
	stu_rtmp_close_connection(nc->connection);
	return STU_OK;
}

stu_int32_t
stu_rtmp_application_on_connect(stu_rtmp_request_t *r) {
	stu_rtmp_amf_t *info;
	stu_int32_t     rc;

	info = stu_rtmp_get_information();
	if (stu_strncasecmp(r->nc.read_access.data, "/", 1) == 0 ||
			stu_strncasecmp(r->nc.read_access.data + 1, r->nc.app_name.data, r->nc.app_name.len) == 0) {
		rc = stu_rtmp_accept(&r->nc);
	} else {
		rc = stu_rtmp_reject(&r->nc);
	}

	return rc;
}



stu_int32_t
stu_rtmp_application_insert(stu_rtmp_netconnection_t *nc) {
	stu_int32_t  rc;

	stu_mutex_lock(&stu_rtmp_applications.lock);
	rc = stu_rtmp_application_insert_locked(nc);
	stu_mutex_unlock(&stu_rtmp_applications.lock);

	return rc;
}

stu_int32_t
stu_rtmp_application_insert_locked(stu_rtmp_netconnection_t *nc) {
	stu_rtmp_application_t *app;
	stu_uint32_t            hk;

	hk = stu_hash_key(nc->app_name.data, nc->app_name.len, stu_rtmp_applications.flags);

	app = stu_hash_find_locked(&stu_rtmp_applications, hk, nc->app_name.data, nc->app_name.len);
	if (app == NULL) {
		app = stu_rtmp_applications.hooks.malloc_fn(sizeof(stu_rtmp_application_t));
		if (app == NULL) {
			stu_log_error(0, "Failed to malloc rtmp application: %s.", nc->app_name.data);
			return STU_ERROR;
		}

		app->name.data = stu_rtmp_applications.hooks.malloc_fn(nc->app_name.len + 1);
		if (app->name.data == NULL) {
			stu_log_error(0, "Failed to malloc rtmp application name: %s.", nc->app_name.data);
			return STU_ERROR;
		}

		stu_strncpy(app->name.data, nc->app_name.data, nc->app_name.len);
		app->name.len = nc->app_name.len;

		stu_hash_init(&app->instances, STU_INSTANCE_LIST_DEFAULT_SIZE, &stu_rtmp_applications.hooks, stu_rtmp_applications.flags);

		if (stu_hash_insert_locked(&stu_rtmp_applications, &nc->app_name, app) == STU_ERROR) {
			stu_log_error(0, "Failed to insert rtmp application: %s.", nc->app_name.data);
			return STU_ERROR;
		}
	}

	nc->application = app;

	return stu_instance_insert(&app->instances, nc);
}

void
stu_rtmp_application_remove(stu_rtmp_netconnection_t *nc) {

}

void
stu_rtmp_application_remove_locked(stu_rtmp_netconnection_t *nc) {

}
