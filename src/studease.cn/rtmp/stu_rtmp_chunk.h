/*
 * stu_rtmp_chunk.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_CHUNK_H_
#define STUDEASE_CN_RTMP_STU_RTMP_CHUNK_H_

#include "stu_rtmp.h"

#define STU_RTMP_CHUNK_DEFAULT_SIZE     4096

#define STU_RTMP_CSID_PROTOCOL_CONTROL  0x02
#define STU_RTMP_CSID_COMMAND           0x03
#define STU_RTMP_CSID_COMMAND_2         0x04
#define STU_RTMP_CSID_STREAM            0x05
#define STU_RTMP_CSID_VIDEO             0x06
#define STU_RTMP_CSID_AUDIO             0x07
#define STU_RTMP_CSID_AV                0x08

typedef struct {
	stu_uint8_t              fmt;         // 2 bits
	stu_uint32_t             csid;        // 6 [ + 8 | 16 ] bits
} stu_rtmp_chunk_basic_t;

typedef struct {
	stu_uint32_t             timestamp;   // 3 | 4 bytes
	stu_uint32_t             message_len; // 3 bytes
	stu_uint8_t              type;        // 1 byte
	stu_uint32_t             stream_id;   // 4 bytes
} stu_rtmp_chunk_header_t;

typedef struct {
	stu_queue_t              queue;

	stu_rtmp_chunk_basic_t   basic;
	stu_rtmp_chunk_header_t  header;
	stu_buf_t                payload;

	// used for parsing chunk.
	stu_uint8_t              state;
	unsigned                 extended:1;
} stu_rtmp_chunk_t;

stu_int32_t       stu_rtmp_write_by_chunk(stu_rtmp_request_t *r, stu_rtmp_chunk_t *ck);

stu_rtmp_chunk_t *stu_rtmp_get_chunk(stu_rtmp_request_t *r, stu_uint32_t csid);
void              stu_rtmp_free_chunk(stu_rtmp_request_t *r, stu_uint32_t csid);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_CHUNK_H_ */
