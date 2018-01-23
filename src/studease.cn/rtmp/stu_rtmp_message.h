/*
 * stu_rtmp_message.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_MESSAGE_H_
#define STUDEASE_CN_RTMP_STU_RTMP_MESSAGE_H_

#include "stu_rtmp.h"

struct stu_rtmp_message_s {
	stu_uint8_t   type;      // 1 byte
	stu_uint32_t  timestamp; // 4 bytes
	stu_uint32_t  msid;      // 3 bytes
	stu_buf_t     payload;

	// used for parsing rtmp message.
	stu_uint8_t   state;
};

#endif /* STUDEASE_CN_RTMP_STU_RTMP_MESSAGE_H_ */
