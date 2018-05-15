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

typedef struct {
	stu_uint8_t   type_id;
	stu_int32_t (*handler)(stu_rtmp_request_t *r);
} stu_rtmp_message_listener_t;

typedef struct {
	stu_str_t     name;
	stu_int32_t (*handler)(stu_rtmp_request_t *r);
} stu_rtmp_command_listener_t;

typedef struct {
	stu_str_t     name;
	stu_int32_t (*handler)(stu_rtmp_request_t *r);
} stu_rtmp_data_listener_t;

struct stu_rtmp_filter_s {
	stu_str_t     pattern;
	stu_hash_t   *listeners;
};

stu_int32_t  stu_rtmp_filter_init_hash();

stu_int32_t  stu_rtmp_filter_add(stu_str_t *pattern, stu_hash_t *listeners);
stu_int32_t  stu_rtmp_filter_del(stu_str_t *pattern);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_FILTER_H_ */
