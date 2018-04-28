/*
 * stu_rtmp_instance.h
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_INSTANCE_H_
#define STUDEASE_CN_RTMP_STU_RTMP_INSTANCE_H_

#include "stu_rtmp.h"

#define STU_RTMP_INST_DEFAULT_SIZE  1024

typedef struct {
	stu_uint64_t          launch_time;
	stu_uint64_t          up_time;

	stu_uint32_t          bytes_in;
	stu_uint32_t          bytes_out;

	stu_uint32_t          msg_in;
	stu_uint32_t          msg_out;
	stu_uint32_t          msg_dropped;

	stu_uint32_t          total_connects;
	stu_uint32_t          total_disconnects;

	stu_uint32_t          accepted;
	stu_uint32_t          rejected;
	stu_uint32_t          connected;
	stu_uint32_t          normal_connects;
	stu_uint32_t          virtual_connects;
	stu_uint32_t          admin_connects;
	stu_uint32_t          debug_connects;

	stu_uint32_t          swf_verification_attempts;
	stu_uint32_t          swf_verification_matches;
	stu_uint32_t          swf_verification_failures;
} stu_rtmp_inst_stat_t;

struct stu_rtmp_instance_s {
	stu_mutex_t           lock;

	stu_str_t             name;
	stu_hash_t            connections;
	stu_hash_t            streams;
	stu_uint8_t           state;

	stu_rtmp_inst_stat_t  stat;

	unsigned              record:1;
};

extern stu_str_t  stu_rtmp_definst;

stu_int32_t  stu_rtmp_instance_init(stu_rtmp_instance_t *inst, u_char *name, size_t len);
void         stu_rtmp_instance_cleanup(stu_rtmp_instance_t *inst);

void         stu_rtmp_instance_broadcast(stu_rtmp_instance_t *inst, u_char *data, size_t len, off_t off);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_INSTANCE_H_ */
