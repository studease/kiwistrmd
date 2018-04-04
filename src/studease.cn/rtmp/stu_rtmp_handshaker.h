/*
 * stu_rtmp_handshaker.h
 *
 *  Created on: 2018骞�1鏈�12鏃�
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_HANDSHAKER_H_
#define STUDEASE_CN_RTMP_STU_RTMP_HANDSHAKER_H_

#include "stu_rtmp.h"

#define STU_RTMP_HANDSHAKER_BUFFER_SIZE     1537
#define STU_RTMP_HANDSHAKER_KEYLEN          SHA256_DIGEST_LENGTH

#define STU_RTMP_HANDSHAKER_SCHEME1         0x00
#define STU_RTMP_HANDSHAKER_SCHEME2         0x01

#define STU_RTMP_HANDSHAKER_PACKET_SIZE     1536
#define STU_RTMP_HANDSHAKER_RANDOM_SIZE     1528
#define STU_RTMP_HANDSHAKER_CHALLENGE_SIZE  128
#define STU_RTMP_HANDSHAKER_DIGEST_SIZE     32
#define STU_RTMP_HANDSHAKER_VERSION         0x5033029

typedef struct {
	stu_connection_t *connection;

	stu_uint8_t       version;
	stu_uint32_t      time;
	stu_uint32_t      zero;

	// used for parsing rtmp handshaker.
	stu_uint8_t       state;
	u_char           *start;
	u_char           *random;
} stu_rtmp_handshaker_t;

void  stu_rtmp_handshaker_read_handler(stu_event_t *ev);
void  stu_rtmp_handshaker_write_handler(stu_event_t *ev);

stu_rtmp_handshaker_t *
      stu_rtmp_create_handshaker(stu_connection_t *c);
void  stu_rtmp_finalize_handshaker(stu_rtmp_handshaker_t *h, stu_int32_t rc);

void  stu_rtmp_free_handshaker(stu_rtmp_handshaker_t *h);
void  stu_rtmp_close_handshaker(stu_rtmp_handshaker_t *h);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_HANDSHAKER_H_ */
