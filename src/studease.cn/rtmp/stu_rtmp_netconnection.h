/*
 * stu_rtmp_netconnection.h
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_NETCONNECTION_H_
#define STUDEASE_CN_RTMP_STU_RTMP_NETCONNECTION_H_

#include "stu_rtmp.h"

#define STU_RTMP_NETCONNECTION_ID_LEN    16
#define STU_RTMP_NETSTREAM_MAXIMAM       4

#define STU_RTMP_LISTENER_DEFAULT_SIZE   32
#define STU_RTMP_RESPONDER_DEFAULT_SIZE  8

typedef struct {
	u_char                 *data;
	size_t                  len;

	stu_str_t               protocol;
	stu_str_t               host;
	uint16_t                port;
	stu_str_t               application;
	stu_str_t               instance;
} stu_rtmp_url_t;

typedef struct {
	stu_uint64_t            connect_time;

	stu_uint32_t            bytes_in;
	stu_uint32_t            bytes_out;

	stu_uint32_t            msg_in;
	stu_uint32_t            msg_out;
	stu_uint32_t            msg_dropped;

	stu_uint32_t            ping_rtt;

	stu_uint32_t            audio_queue_bytes;
	stu_uint32_t            video_queue_bytes;
	stu_uint32_t            so_queuebytes;
	stu_uint32_t            data_queue_bytes;

	stu_uint32_t            dropped_audio_bytes;
	stu_uint32_t            dropped_video_bytes;

	stu_uint32_t            audio_queue_msgs;
	stu_uint32_t            video_queue_msgs;
	stu_uint32_t            so_queue_msgs;
	stu_uint32_t            data_queue_msgs;

	stu_uint32_t            dropped_audio_msgs;
	stu_uint32_t            dropped_video_msgs;
} stu_rtmp_conn_stat_t;

struct stu_rtmp_netconnection_s {
	stu_connection_t       *conn;
	stu_rtmp_application_t *application;
	stu_rtmp_instance_t    *instance;

	stu_hash_t              commands;
	stu_hash_t              responders;

	stu_uint32_t            stream_id;
	stu_uint32_t            transaction_id;

	stu_int32_t             far_chunk_size;
	stu_uint32_t            far_ack_window_size;
	stu_uint32_t            far_bandwidth;
	stu_uint8_t             far_limit_type;

	stu_int32_t             near_chunk_size;
	stu_uint32_t            near_ack_window_size;
	stu_uint32_t            near_bandwidth;
	stu_uint8_t             near_limit_type;

	stu_str_t               agent;
	stu_uint64_t            audio_codecs;
	stu_str_t               audio_sample_access;
	stu_bool_t              connected;
	stu_str_t               far_id;
	stu_str_t               id;
	stu_str_t               ip;
	stu_str_t               muxer_type;
	stu_str_t               near_id;
	stu_uint8_t             object_encoding;
	stu_str_t               page_url;
	stu_str_t               protocol_version;
	stu_str_t               read_access;
	stu_str_t               referrer;
	stu_bool_t              secure;
	stu_rtmp_url_t          url;
	stu_uint64_t            video_codecs;
	stu_str_t               video_sample_access;
	stu_str_t               virtual_key;
	stu_str_t               write_access;

	stu_rtmp_conn_stat_t    stat;

	// used for parsing rtmp chunk.
	stu_uint8_t             state;
	stu_uint32_t            pre_stream_id;
};

void            stu_rtmp_connection_init(stu_rtmp_netconnection_t *nc, stu_connection_t *c);

stu_int32_t     stu_rtmp_connect(stu_rtmp_netconnection_t *nc, u_char *url, size_t len, stu_rtmp_responder_t *res, ...);
stu_int32_t     stu_rtmp_call(stu_rtmp_netconnection_t *nc, u_char *command, size_t len, stu_rtmp_responder_t *res, ...);
stu_int32_t     stu_rtmp_close(stu_rtmp_netconnection_t *nc);
stu_int32_t     stu_rtmp_create_stream(stu_rtmp_netconnection_t *nc, stu_rtmp_responder_t *res);

stu_int32_t     stu_rtmp_set_chunk_size(stu_rtmp_netconnection_t *nc, stu_int32_t size);
stu_int32_t     stu_rtmp_send_abort(stu_rtmp_netconnection_t *nc);
stu_int32_t     stu_rtmp_send_ack_sequence(stu_rtmp_netconnection_t *nc);
stu_int32_t     stu_rtmp_send_user_control(stu_rtmp_netconnection_t *nc, stu_uint16_t event, stu_uint32_t stream_id, stu_uint32_t buf_len, stu_uint32_t timestamp);
stu_int32_t     stu_rtmp_set_ack_window_size(stu_rtmp_netconnection_t *nc, stu_uint32_t size);
stu_int32_t     stu_rtmp_set_peer_bandwidth(stu_rtmp_netconnection_t *nc, stu_uint32_t bandwidth, stu_uint8_t limit);

stu_int32_t     stu_rtmp_send_result(stu_rtmp_netconnection_t *nc, stu_double_t tran, stu_rtmp_amf_t *prop, stu_rtmp_amf_t *info);
stu_int32_t     stu_rtmp_send_error(stu_rtmp_netconnection_t *nc, stu_double_t tran, stu_rtmp_amf_t *prop, stu_rtmp_amf_t *info);
stu_int32_t     stu_rtmp_send_buffer(stu_rtmp_netconnection_t *nc, stu_uint32_t csid, stu_uint32_t timestamp, stu_uint8_t type, stu_uint32_t stream_id, u_char *data, size_t len);
stu_rtmp_amf_t *stu_rtmp_get_information(stu_str_t *level, stu_str_t *code, const char *description);

stu_int32_t     stu_rtmp_on_connect(stu_rtmp_request_t *r);
stu_int32_t     stu_rtmp_on_close(stu_rtmp_request_t *r);
stu_int32_t     stu_rtmp_on_create_stream(stu_rtmp_request_t *r);
stu_int32_t     stu_rtmp_on_result(stu_rtmp_request_t *r);
stu_int32_t     stu_rtmp_on_error(stu_rtmp_request_t *r);

void            stu_rtmp_close_connection(stu_rtmp_netconnection_t *nc);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_NETCONNECTION_H_ */
