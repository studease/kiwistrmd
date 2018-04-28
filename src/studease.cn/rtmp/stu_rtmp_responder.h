/*
 * stu_rtmp_responder.h
 *
 *  Created on: 2018Äê4ÔÂ11ÈÕ
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_RESPONDER_H_
#define STUDEASE_CN_RTMP_STU_RTMP_RESPONDER_H_

#include "stu_rtmp.h"

typedef struct {
	stu_uint32_t  transaction_id;

	void        (*result)(stu_rtmp_request_t *r);
	void        (*status)(stu_rtmp_request_t *r);
} stu_rtmp_responder_t;

#endif /* STUDEASE_CN_RTMP_STU_RTMP_RESPONDER_H_ */
