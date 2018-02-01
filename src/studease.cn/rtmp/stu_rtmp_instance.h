/*
 * stu_rtmp_instance.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_INSTANCE_H_
#define STUDEASE_CN_RTMP_STU_RTMP_INSTANCE_H_

#include "stu_rtmp.h"

#define STU_INSTANCE_LIST_DEFAULT_SIZE  128

#define STU_INSTANCE_NAME_MAX_LEN       16

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
	stu_str_t             name;
	stu_hash_t            connections;
	stu_hash_t            streams;
	stu_uint8_t           state;

	stu_rtmp_inst_stat_t  stat;

	unsigned              record:1;
};

stu_int32_t  stu_instance_insert(stu_hash_t *hash, stu_rtmp_netconnection_t *nc);
stu_int32_t  stu_instance_insert_locked(stu_hash_t *hash, stu_rtmp_netconnection_t *nc);

void         stu_instance_remove(stu_rtmp_netconnection_t *nc);
void         stu_instance_remove_locked(stu_rtmp_netconnection_t *nc);

void         stu_instance_broadcast(stu_rtmp_instance_t *inst, u_char *data, size_t len, off_t off);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_INSTANCE_H_ */
