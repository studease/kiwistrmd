/*
 * stu_rtmp_chunk.h
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_CHUNK_H_
#define STUDEASE_CN_RTMP_STU_RTMP_CHUNK_H_

#include "stu_rtmp.h"

#define STU_RTMP_CHUNK_DEFAULT_SIZE     128
#define STU_RTMP_CHUNK_SIZE             4096

#define STU_RTMP_CSID_PROTOCOL_CONTROL  0x02
#define STU_RTMP_CSID_COMMAND           0x03
#define STU_RTMP_CSID_COMMAND_2         0x04
#define STU_RTMP_CSID_STREAM            0x05
#define STU_RTMP_CSID_VIDEO             0x06
#define STU_RTMP_CSID_AUDIO             0x07
#define STU_RTMP_CSID_AV                0x08

typedef struct {
	stu_queue_t   queue;

	stu_uint8_t   fmt;       // 2 bits
	stu_uint32_t  csid;      // 6 [ + 8 | 16 ] bits

	stu_uint32_t  timestamp; // 3 | 4 bytes
	stu_uint8_t   type_id;   // 1 byte
	stu_uint32_t  stream_id; // 4 bytes

	stu_buf_t     payload;

	// used for parsing chunk.
	stu_uint8_t   state;
	unsigned      extended:1;
} stu_rtmp_chunk_t;

stu_int32_t  stu_rtmp_chunk_append(stu_rtmp_chunk_t *ck, u_char *data, size_t len);
stu_int32_t  stu_rtmp_chunk_grow(stu_rtmp_chunk_t *ck, size_t size);
void         stu_rtmp_chunk_cleanup(stu_rtmp_chunk_t *ck);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_CHUNK_H_ */
