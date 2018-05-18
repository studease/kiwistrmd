/*
 * stu_rtmp_phase.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_PHASE_H_
#define STUDEASE_CN_RTMP_STU_RTMP_PHASE_H_

#include "stu_rtmp.h"

#define STU_RTMP_PHASE_MAX_RECORDS  32

typedef stu_int32_t (*stu_rtmp_phase_handler_pt)(stu_rtmp_request_t *r);

struct stu_rtmp_phase_s {
	stu_str_t     pattern;
	stu_hash_t   *listeners;
};

typedef struct {
	stu_str_t     name;
	stu_int32_t (*handler)(stu_rtmp_request_t *r);
	stu_int32_t (*close)(stu_rtmp_request_t *r);
} stu_rtmp_phase_listener_t;

stu_int32_t  stu_rtmp_phase_init();

stu_int32_t  stu_rtmp_phase_add(stu_str_t *name, stu_hash_t *listeners);
stu_int32_t  stu_rtmp_phase_del(stu_str_t *name);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_PHASE_H_ */
