/*
 * stu_rtmp_filter.h
 *
 *  Created on: 2018年1月24日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_FILTER_H_
#define STUDEASE_CN_RTMP_STU_RTMP_FILTER_H_

#include "stu_rtmp.h"

#define STU_RTMP_FILTER_MAX_RECORDS  32

typedef stu_int32_t (*stu_rtmp_filter_handler_pt)(stu_rtmp_request_t *r);

struct stu_rtmp_filter_s {
	stu_str_t                   pattern;
	stu_rtmp_filter_handler_pt  handler;
};

stu_int32_t  stu_rtmp_filter_init_hash();

stu_int32_t  stu_rtmp_filter_add(stu_str_t *pattern, stu_rtmp_filter_handler_pt handler);
stu_int32_t  stu_rtmp_filter_del(stu_str_t *pattern, stu_rtmp_filter_handler_pt handler);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_FILTER_H_ */
