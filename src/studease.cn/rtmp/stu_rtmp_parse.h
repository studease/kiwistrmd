/*
 * stu_rtmp_parse.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_PARSE_H_
#define STUDEASE_CN_RTMP_STU_RTMP_PARSE_H_

#include "stu_rtmp.h"

#define STU_RTMP_URL_FLAG_PROTOCOL 0x01
#define STU_RTMP_URL_FLAG_HOST     0x02
#define STU_RTMP_URL_FLAG_PORT     0x04
#define STU_RTMP_URL_FLAG_APP      0x08
#define STU_RTMP_URL_FLAG_INST     0x10

stu_int32_t  stu_rtmp_parse_handshake(stu_rtmp_handshake_t *h, stu_buf_t *b);
stu_int32_t  stu_rtmp_parse_chunk(stu_rtmp_request_t *r, stu_buf_t *b);
stu_int32_t  stu_rtmp_parse_url(stu_str_t *url, stu_str_t *dst, stu_uint8_t flag);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_PARSE_H_ */
