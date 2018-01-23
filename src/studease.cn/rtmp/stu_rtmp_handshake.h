/*
 * stu_rtmp_handshake.h
 *
 *  Created on: 2018年1月12日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_HANDSHAKE_H_
#define STUDEASE_CN_RTMP_STU_RTMP_HANDSHAKE_H_

#include "stu_rtmp.h"

#define STU_RTMP_HANDSHAKE_BUFFER_SIZE     1537
#define STU_RTMP_HANDSHAKE_KEYLEN          SHA256_DIGEST_LENGTH

#define STU_RTMP_HANDSHAKE_SCHEME1         0x00
#define STU_RTMP_HANDSHAKE_SCHEME2         0x01

#define STU_RTMP_HANDSHAKE_PACKET_SIZE     1536
#define STU_RTMP_HANDSHAKE_RANDOM_SIZE     1528
#define STU_RTMP_HANDSHAKE_CHALLENGE_SIZE  128
#define STU_RTMP_HANDSHAKE_DIGEST_SIZE     32
#define STU_RTMP_HANDSHAKE_VERSION         0x5033029

typedef struct {
	stu_connection_t *connection;

	stu_uint8_t       rtmp_version;
	stu_uint32_t      time;
	stu_uint32_t      zero;

	// used for parsing rtmp handshake.
	stu_uint8_t       state;

	u_char           *start;
	u_char           *random;
} stu_rtmp_handshake_t;

void  stu_rtmp_handshake_read_handler(stu_event_t *ev);
void  stu_rtmp_handshake_write_handler(stu_event_t *ev);

stu_rtmp_handshake_t *
      stu_rtmp_create_handshake(stu_connection_t *c);
void  stu_rtmp_finalize_handshake(stu_rtmp_handshake_t *h, stu_int32_t rc);

void  stu_rtmp_free_handshake(stu_rtmp_handshake_t *h);
void  stu_rtmp_close_handshake(stu_rtmp_handshake_t *h);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_HANDSHAKE_H_ */
