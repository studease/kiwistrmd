/*
 * stu_rtmp_phase.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_PHASE_H_
#define STUDEASE_CN_RTMP_STU_RTMP_PHASE_H_

#include "stu_rtmp.h"

typedef stu_int32_t (*stu_rtmp_phase_handler_pt)(stu_rtmp_message_t *r);

struct stu_rtmp_phase_s {
	stu_rtmp_phase_handler_pt  handler;
};

stu_int32_t  stu_rtmp_phase_init();

#endif /* STUDEASE_CN_RTMP_STU_RTMP_PHASE_H_ */
