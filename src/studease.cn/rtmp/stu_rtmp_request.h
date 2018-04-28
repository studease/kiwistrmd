/*
 * stu_rtmp_request.h
 *
 *  Created on: 2018骞�1鏈�22鏃�
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_REQUEST_H_
#define STUDEASE_CN_RTMP_STU_RTMP_REQUEST_H_

#include "stu_rtmp.h"

#define STU_RTMP_REQUEST_DEFAULT_SIZE  4096 + 14

#define STU_RTMP_UNKNOWN               0x0000
#define STU_RTMP_PUSH                  0x0001
#define STU_RTMP_PULL                  0x0002

typedef void (*stu_rtmp_event_handler_pt)(stu_rtmp_request_t *r);

typedef struct {
	stu_str_t                   name;
	stu_uint16_t                mask;
} stu_rtmp_method_bitmask_t;

typedef struct {
	stu_uint8_t                 type_id;
	stu_int32_t               (*handler)(stu_rtmp_request_t *r);
} stu_rtmp_message_listener_t;

typedef struct {
	stu_str_t                   name;
	stu_int32_t               (*handler)(stu_rtmp_request_t *r);
} stu_rtmp_command_listener_t;

typedef struct {
	stu_str_t                   name;
	stu_int32_t               (*handler)(stu_rtmp_request_t *r);
} stu_rtmp_data_listener_t;

struct stu_rtmp_request_s {
	stu_rtmp_connection_t       connection;

	stu_rtmp_event_handler_pt   read_event_handler;
	stu_rtmp_event_handler_pt   write_event_handler;

	stu_queue_t                 chunks;
	stu_rtmp_chunk_t           *chunk_in;

	stu_uint32_t                chunk_size;
	stu_uint32_t                chunk_stream_id;
	stu_uint32_t                ack;
	stu_uint16_t                event_type;
	stu_uint32_t                timestamp;
	stu_uint32_t                buffer_length;
	stu_uint32_t                ack_window_size;
	stu_uint32_t                bandwidth;
	stu_uint8_t                 limit_type;
	unsigned                    format:4;      // 1111 0000
	unsigned                    sample_rate:2; // 0000 1100
	unsigned                    sample_size:1; // 0000 0010
	unsigned                    channels:1;    // 0000 0001
	unsigned                    frame_type:4;  // 0xF0
	unsigned                    codec:4;       // 0x0F
	stu_uint8_t                 data_type;
	stu_str_t                  *data_handler;
	stu_str_t                  *data_key;
	stu_rtmp_amf_t             *data_value;

	stu_str_t                  *command;
	stu_uint32_t                transaction_id;
	stu_rtmp_amf_t             *command_obj;
	stu_rtmp_amf_t             *arguments;
	stu_uint32_t                stream_id;
	stu_str_t                  *stream_name;
	stu_double_t                start;
	stu_double_t                duration;
	stu_bool_t                  reset;
	stu_bool_t                  flag;
	stu_str_t                  *publishing_name;
	stu_str_t                  *publishing_type;
	stu_double_t                milliseconds;

	// used for parsing rtmp request.
	stu_uint8_t                 state;
};

void  stu_rtmp_request_read_handler(stu_event_t *ev);
void  stu_rtmp_request_write_handler(stu_rtmp_request_t *r);

stu_rtmp_request_t *
      stu_rtmp_create_request(stu_connection_t *c);
void  stu_rtmp_process_request(stu_rtmp_request_t *r);
void  stu_rtmp_finalize_request(stu_rtmp_request_t *r, stu_int32_t rc);

void  stu_rtmp_free_request(stu_rtmp_request_t *r);
void  stu_rtmp_close_request(stu_rtmp_request_t *r);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_REQUEST_H_ */
