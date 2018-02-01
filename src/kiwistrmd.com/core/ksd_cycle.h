/*
 * ksd_cycle.h
 *
 *  Created on: 2018年1月30日
 *      Author: Tony Lau
 */

#ifndef KIWISTRMD_COM_CORE_KSD_CYCLE_H_
#define KIWISTRMD_COM_CORE_KSD_CYCLE_H_

#include "ksd_core.h"

#define KSD_CYCLE_DEFAULT_SIZE        4096
#define KSD_CYCLE_RECORD_DEFAULT_SIZE 1024

typedef struct {
	stu_pool_t   *pool;
	ksd_conf_t    conf;
	stu_uint32_t  auto_user_id;
} ksd_cycle_t;

stu_int32_t  ksd_cycle_init();

stu_int32_t  ksd_cycle_create_pidfile(stu_file_t *pid);
void         ksd_cycle_delete_pidfile(stu_file_t *pid);

#endif /* KIWISTRMD_COM_CORE_KSD_CYCLE_H_ */
