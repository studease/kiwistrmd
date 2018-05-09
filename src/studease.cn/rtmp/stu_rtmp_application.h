/*
 * stu_rtmp_application.h
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_APPLICATION_H_
#define STUDEASE_CN_RTMP_STU_RTMP_APPLICATION_H_

#include "stu_rtmp.h"

#define STU_RTMP_APP_LIST_DEFAULT_SIZE          128
#define STU_RTMP_APP_DEFAULT_SIZE               8

typedef stu_int32_t (*stu_rtmp_application_handler_pt)(stu_rtmp_application_t *app);

typedef struct {
	stu_uint64_t         launch_time;
	stu_uint64_t         up_time;

	stu_uint32_t         bytes_in;
	stu_uint32_t         bytes_out;

	stu_uint32_t         msg_in;
	stu_uint32_t         msg_out;
	stu_uint32_t         msg_dropped;

	stu_uint32_t         total_instances_loaded;
	stu_uint32_t         total_instances_unloaded;

	stu_uint32_t         total_connects;
	stu_uint32_t         total_disconnects;

	stu_uint32_t         accepted;
	stu_uint32_t         rejected;
	stu_uint32_t         connected;
	stu_uint32_t         normal_connects;
	stu_uint32_t         virtual_connects;
	stu_uint32_t         admin_connects;
	stu_uint32_t         debug_connects;

	stu_uint32_t         swf_verification_attempts;
	stu_uint32_t         swf_verification_matches;
	stu_uint32_t         swf_verification_failures;
} stu_rtmp_app_stat_t;

struct stu_rtmp_application_s {
	stu_mutex_t          lock;
	stu_queue_t          queue;

	stu_str_t            name;
	stu_hash_t           instances;

	stu_rtmp_app_stat_t  stat;

	unsigned             record:1;
};

extern stu_hash_t  stu_rtmp_apps;

extern stu_rtmp_application_handler_pt  stu_rtmp_on_application_start;
extern stu_rtmp_application_handler_pt  stu_rtmp_on_application_stop;

stu_int32_t  stu_rtmp_application_init_hash();

stu_int32_t  stu_rtmp_application_init(stu_rtmp_application_t *app, u_char *name, size_t len);
void         stu_rtmp_application_cleanup(stu_rtmp_application_t *app);

stu_int32_t  stu_rtmp_accept(stu_rtmp_netconnection_t *nc);
stu_int32_t  stu_rtmp_reject(stu_rtmp_netconnection_t *nc);

stu_int32_t  stu_rtmp_application_insert(stu_rtmp_netconnection_t *nc);
stu_int32_t  stu_rtmp_application_insert_locked(stu_rtmp_netconnection_t *nc);
void         stu_rtmp_application_remove(stu_rtmp_netconnection_t *nc);
void         stu_rtmp_application_remove_locked(stu_rtmp_netconnection_t *nc);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_APPLICATION_H_ */
