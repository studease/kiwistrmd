/*
 * ksd_core.h
 *
 *  Created on: 2018年1月11日
 *      Author: Tony Lau
 */

#ifndef KIWISTRMD_COM_CORE_KSD_CORE_H_
#define KIWISTRMD_COM_CORE_KSD_CORE_H_

#include "../../studease.cn/websocket/stu_websocket.h"
#include "../../studease.cn/rtmp/stu_rtmp.h"

typedef enum {
	PREVIEW      = 0x00,
	ENTERPRISE   = 0x01
} ksd_edition_t;

typedef struct {
	stu_str_t      name;
	ksd_edition_t  mask;
} ksd_edition_mask_t;

typedef struct {
	stu_str_t      name;
	stu_uint8_t    mask;
} ksd_mode_mask_t;

#include "ksd_conf.h"
#include "ksd_request.h"
#include "ksd_cycle.h"
#include "ksd_process.h"

#endif /* KIWISTRMD_COM_CORE_KSD_CORE_H_ */
