/*
 * stu_rtmp_parse.h
 *
 *  Created on: 2018骞�1鏈�16鏃�
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_PARSE_H_
#define STUDEASE_CN_RTMP_STU_RTMP_PARSE_H_

#include "stu_rtmp.h"

stu_int32_t  stu_rtmp_parse_handshaker(stu_rtmp_handshaker_t *h, stu_buf_t *src);
stu_int32_t  stu_rtmp_parse_chunk(stu_rtmp_request_t *r, stu_buf_t *src);
stu_int32_t  stu_rtmp_parse_url(stu_rtmp_url_t *url, u_char *src, size_t len);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_PARSE_H_ */
