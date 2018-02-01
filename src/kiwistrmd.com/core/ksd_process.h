/*
 * ksd_process.h
 *
 *  Created on: 2018年1月30日
 *      Author: Tony Lau
 */

#ifndef KIWISTRMD_COM_CORE_KSD_PROCESS_H_
#define KIWISTRMD_COM_CORE_KSD_PROCESS_H_

#include "ksd_core.h"

void  ksd_process_master_cycle();
void  ksd_process_worker_cycle(stu_int32_t threads, void *data);

#endif /* KIWISTRMD_COM_CORE_KSD_PROCESS_H_ */
