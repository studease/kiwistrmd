/*
 * ksd_conf.h
 *
 *  Created on: 2018年1月11日
 *      Author: Tony Lau
 */

#ifndef KIWISTRMD_COM_CORE_KSD_CONF_H_
#define KIWISTRMD_COM_CORE_KSD_CONF_H_

#include "ksd_core.h"

#define KCD_CONF_MAX_SIZE  4096

typedef struct {
	stu_file_t     log;
	stu_file_t     pid;

	ksd_edition_t  edition;
	stu_uint8_t    mode;

	stu_bool_t     master_process;
	stu_int32_t    worker_processes;
	stu_int32_t    worker_threads;
	stu_uint8_t    debug;

	uint16_t       port;
	stu_str_t      root;
	stu_str_t      cors;

	uint16_t       http_port;
	stu_str_t      http_root;
	stu_str_t      http_cors;

	stu_bool_t     push_stat;
	stu_msec_t     push_stat_interval; // seconds

	stu_hash_t     target;             // stu_rtmp_target_t *
	stu_list_t     ident;              // stu_upstream_server_t *
	stu_list_t     stat;               // stu_upstream_server_t *
} ksd_conf_t;

stu_int32_t  ksd_conf_parse_file(ksd_conf_t *conf, u_char *name);

#endif /* KIWISTRMD_COM_CORE_KSD_CONF_H_ */
