/*
 * stu_rtmp_request.h
 *
 *  Created on: 2018年1月22日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_REQUEST_H_
#define STUDEASE_CN_RTMP_STU_RTMP_REQUEST_H_

#include "stu_rtmp.h"

#define STU_RTMP_REQUEST_DEFAULT_SIZE  4096 + 14

#define STU_RTMP_UNKNOWN               0x0001
#define STU_RTMP_PUSH                  0x0002
#define STU_RTMP_PULL                  0x0004

typedef void (*stu_rtmp_event_handler_pt)(stu_rtmp_request_t *r);

typedef struct {
	stu_str_t                  name;
	stu_uint16_t               mask;
} stu_rtmp_method_bitmask_t;

struct stu_rtmp_request_s {
	stu_rtmp_netconnection_t   nc;

	stu_rtmp_event_handler_pt  read_event_handler;
	stu_rtmp_event_handler_pt  write_event_handler;

	stu_queue_t                chunks;
	stu_rtmp_chunk_t          *chunk_in;
	void                      *message;

	stu_int32_t                status;

	// used for parsing rtmp request.
	stu_uint8_t                state;
};

void  stu_rtmp_request_read_handler(stu_event_t *ev);
void  stu_rtmp_request_write_handler(stu_rtmp_request_t *r);

stu_rtmp_request_t *
      stu_rtmp_create_request(stu_connection_t *c);
void  stu_rtmp_process_request(stu_rtmp_request_t *r);
void  stu_rtmp_finalize_request(stu_rtmp_request_t *r, stu_int32_t rc);

void  stu_rtmp_free_request(stu_rtmp_request_t *r);
void  stu_rtmp_close_request(stu_rtmp_request_t *r);
void  stu_rtmp_close_connection(stu_connection_t *c);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_REQUEST_H_ */
