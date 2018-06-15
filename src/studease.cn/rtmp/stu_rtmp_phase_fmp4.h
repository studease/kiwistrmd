/*
 * stu_rtmp_fmp4_muxer.h
 *
 *  Created on: 2018年5月17日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_PHASE_FMP4_H_
#define STUDEASE_CN_RTMP_STU_RTMP_PHASE_FMP4_H_

#include "stu_rtmp.h"

#define STU_RTMP_PHASE_FMP4_MAXIMUM  128

stu_int32_t  stu_rtmp_phase_fmp4_init();

stu_int32_t  stu_rtmp_phase_fmp4_handler(stu_rtmp_request_t *r);
stu_int32_t  stu_rtmp_phase_fmp4_close(stu_rtmp_request_t *r);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_PHASE_FMP4_H_ */
