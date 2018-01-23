/*
 * stu_rtmp_netconnection.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_NETCONNECTION_H_
#define STUDEASE_CN_RTMP_STU_RTMP_NETCONNECTION_H_

#include "stu_rtmp.h"

typedef struct {
	stu_uint64_t      connect_time;

	stu_uint32_t      bytes_in;
	stu_uint32_t      bytes_out;

	stu_uint32_t      msg_in;
	stu_uint32_t      msg_out;
	stu_uint32_t      msg_dropped;
} stu_rtmp_admin_stat_t;

typedef struct {
	stu_rtmp_admin_stat_t *admin;

	stu_uint32_t      ping_rtt;

	stu_uint32_t      audio_queue_bytes;
	stu_uint32_t      video_queue_bytes;
	stu_uint32_t      so_queuebytes;
	stu_uint32_t      data_queue_bytes;

	stu_uint32_t      dropped_audio_bytes;
	stu_uint32_t      dropped_video_bytes;

	stu_uint32_t      audio_queue_msgs;
	stu_uint32_t      video_queue_msgs;
	stu_uint32_t      so_queue_msgs;
	stu_uint32_t      data_queue_msgs;

	stu_uint32_t      dropped_audio_msgs;
	stu_uint32_t      dropped_video_msgs;
} stu_rtmp_stat_t;

typedef struct {
	stu_connection_t *connection;

	stu_uint32_t      far_chunk_size;
	stu_uint32_t      near_chunk_size;
	stu_uint32_t      far_ack_window_size;
	stu_uint32_t      near_ack_window_size;
	stu_uint32_t      far_bandwidth;
	stu_uint32_t      near_bandwidth;
	stu_uint32_t      far_limit_type;
	stu_uint32_t      near_limit_type;

	stu_uint32_t      transaction_id;
	stu_queue_t       responders;

	stu_str_t         agent;
	stu_str_t         app_name;
	stu_uint64_t      audio_codecs;
	stu_str_t         audio_sample_access;
	stu_bool_t        connected;
	stu_str_t         far_id;
	stu_str_t         inst_name;
	stu_str_t         ip;
	stu_str_t         muxer_type;
	stu_str_t         near_id;
	stu_uint8_t       object_encoding;
	stu_str_t         page_url;
	stu_str_t         protocol;
	stu_str_t         protocol_version;
	stu_str_t         read_access;
	stu_str_t         referrer;
	stu_bool_t        secure;
	stu_str_t         uri;
	stu_uint64_t      video_codecs;
	stu_str_t         video_sample_access;
	stu_str_t         virtual_key;
	stu_str_t         write_access;

	stu_rtmp_stat_t   stat;
} stu_rtmp_netconnection_t;

#endif /* STUDEASE_CN_RTMP_STU_RTMP_NETCONNECTION_H_ */
