/*
 * stu_flv.h
 *
 *  Created on: 2018年5月15日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_FORMAT_STU_FLV_H_
#define STUDEASE_CN_FORMAT_STU_FLV_H_

#include "stu_format.h"

#define STU_FLV_TAG_HEADER   11
#define STU_FLV_TAG_FOOTER   20
#define STU_FLV_DATA_OFFSET  13

typedef struct {
	stu_uint8_t   type;
	stu_uint32_t  size;      // 3 bytes
	stu_uint32_t  timestamp;
	stu_uint32_t  stream_id; // 3 bytes
	u_char       *data;
} stu_flv_tag_t;

typedef struct {
	stu_file_t    file;
	stu_uint32_t  timestamp;
} stu_flv_t;

stu_int32_t  stu_flv_open(stu_flv_t *flv, u_long mode, u_long create, u_long access);
stu_int32_t  stu_flv_flush(stu_flv_t *flv, stu_uint8_t type, stu_uint32_t timestamp, u_char *data, size_t len);
stu_int32_t  stu_flv_close(stu_flv_t *flv);

#endif /* STUDEASE_CN_FORMAT_STU_FLV_H_ */
