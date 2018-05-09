/*
 * ksd_cycle.c
 *
 *  Created on: 2018年1月30日
 *      Author: Tony Lau
 */

#include "ksd_core.h"

volatile ksd_cycle_t *ksd_cycle;

extern stu_str_t      KSD_CONF_DEFAULT_PATH;


stu_int32_t
ksd_cycle_init() {
	stu_pool_data_t *data;
	u_char          *p;
	stu_shm_t        shm;

	shm.size = KSD_CYCLE_DEFAULT_SIZE;
	if (stu_shm_alloc(&shm) == STU_ERROR) {
		stu_log_error(0, "Failed to create cycle pool.");
		return STU_ERROR;
	}

	p = shm.addr;

	ksd_cycle = (ksd_cycle_t *) p;
	p += sizeof(ksd_cycle_t);

	ksd_cycle->pool = (stu_pool_t *) p;
	p += sizeof(stu_pool_t);

	stu_mutex_init(&ksd_cycle->pool->lock, NULL);
	stu_queue_init(&ksd_cycle->pool->queue);
	stu_queue_init(&ksd_cycle->pool->large);
	ksd_cycle->pool->size = shm.addr + shm.size - p;

	data = (stu_pool_data_t *) p;
	p += sizeof(stu_pool_data_t);

	stu_queue_init(&data->queue);
	data->start = data->last = p;
	data->end = shm.addr + shm.size;
	data->failed = 0;

	stu_queue_insert_tail(&ksd_cycle->pool->queue, &data->queue);

	// errno
	if (stu_strerror_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init error strings.");
		return STU_ERROR;
	}

	// time
	stu_time_init();

	// conf
	if (ksd_conf_parse_file((ksd_conf_t *) &ksd_cycle->conf, KSD_CONF_DEFAULT_PATH.data) == STU_ERROR) {
		stu_log_error(0, "Failed to parse conf.");
		return STU_ERROR;
	}

	// log
	if (stu_log_init((stu_file_t *) &ksd_cycle->conf.log) == STU_ERROR) {
		stu_log_error(0, "Failed to init log.");
		return STU_ERROR;
	}

	// timer
	if (stu_timer_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init timer.");
		return STU_ERROR;
	}

	// event
	if (stu_event_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init event.");
		return STU_ERROR;
	}

	// connection
	if (stu_connection_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init connection.");
		return STU_ERROR;
	}

	// mq
	if (stu_mq_init((stu_str_t *) &ksd_cycle->conf.root, KSD_CYCLE_RECORD_DEFAULT_SIZE) == STU_ERROR) {
		stu_log_error(0, "Failed to init mq.");
		return STU_ERROR;
	}

	// process
	if (stu_process_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init process.");
		return STU_ERROR;
	}

	// http
	if (stu_http_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init http.");
		return STU_ERROR;
	}

	// websocket
	if (stu_websocket_init() == STU_ERROR) {
		stu_log_error(0, "Failed to init websocket.");
		return STU_ERROR;
	}

	// rtmp
	if (stu_rtmp_init() == STU_ERROR) {
		return STU_ERROR;
	}

	// request
	if (ksd_request_init() == STU_ERROR) {
		return STU_ERROR;
	}

	return STU_OK;
}


stu_int32_t
ksd_cycle_create_pidfile(stu_file_t *pid) {
	u_char  tmp[STU_MAX_PATH];

	stu_memzero(tmp, STU_MAX_PATH);

	pid->fd = stu_file_open(pid->name.data, STU_FILE_RDWR, STU_FILE_TRUNCATE, STU_FILE_DEFAULT_ACCESS);
	if (pid->fd == STU_FILE_INVALID) {
		stu_log_error(stu_errno, "Failed to " stu_file_open_n " pid file.");
		return STU_ERROR;
	}

	stu_sprintf(tmp, "%d", stu_getpid());

	if (stu_file_write(pid, tmp, stu_strlen(tmp), pid->offset) == STU_ERROR) {
		stu_log_error(stu_errno, "Failed to " stu_write_fd_n " pid file.");
		return STU_ERROR;
	}

	return STU_OK;
}

void
ksd_cycle_delete_pidfile(stu_file_t *pid) {
	stu_file_delete(pid->name.data);
}
