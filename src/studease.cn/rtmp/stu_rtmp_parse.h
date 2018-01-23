/*
 * stu_rtmp_parse.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_PARSE_H_
#define STUDEASE_CN_RTMP_STU_RTMP_PARSE_H_

#include "stu_rtmp.h"

stu_int32_t  stu_rtmp_parse_handshake(stu_rtmp_handshake_t *h, stu_buf_t *b);
stu_int32_t  stu_rtmp_parse_chunk(stu_rtmp_request_t *r, stu_buf_t *b);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_PARSE_H_ */
