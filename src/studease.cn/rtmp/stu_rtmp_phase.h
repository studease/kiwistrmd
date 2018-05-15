/*
 * stu_rtmp_phase.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_PHASE_H_
#define STUDEASE_CN_RTMP_STU_RTMP_PHASE_H_

#include "stu_rtmp.h"

typedef stu_int32_t (*stu_rtmp_phase_handler_pt)(stu_rtmp_request_t *r);

struct stu_rtmp_phase_s {
	stu_str_t                  name;
	stu_rtmp_phase_handler_pt  handler;
};

stu_int32_t  stu_rtmp_phase_init();

stu_int32_t  stu_rtmp_phase_add(stu_str_t *name, stu_rtmp_phase_handler_pt handler);
stu_int32_t  stu_rtmp_phase_del(stu_str_t *name);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_PHASE_H_ */
