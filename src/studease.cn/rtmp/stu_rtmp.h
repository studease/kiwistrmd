/*
 * stu_rtmp.h
 *
 *  Created on: 2018骞�1鏈�12鏃�
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_H_
#define STUDEASE_CN_RTMP_STU_RTMP_H_

#include "../stu_config.h"
#include "../core/stu_core.h"

#define STU_RTMP_VERSION_3  0x03

typedef struct stu_rtmp_instance_s    stu_rtmp_instance_t;
typedef struct stu_rtmp_application_s stu_rtmp_application_t;
typedef struct stu_rtmp_request_s     stu_rtmp_request_t;
typedef struct stu_rtmp_filter_s      stu_rtmp_filter_t;
typedef struct stu_rtmp_phase_s       stu_rtmp_phase_t;
typedef struct stu_rtmp_upstream_s    stu_rtmp_upstream_t;

#include "stu_rtmp_handshaker.h"
#include "stu_rtmp_amf.h"
#include "stu_rtmp_chunk.h"
#include "stu_rtmp_message.h"
#include "stu_rtmp_netconnection.h"
#include "stu_rtmp_stream.h"
#include "stu_rtmp_netstream.h"
#include "stu_rtmp_instance.h"
#include "stu_rtmp_application.h"
#include "stu_rtmp_filter.h"
#include "stu_rtmp_phase.h"
#include "stu_rtmp_request.h"
#include "stu_rtmp_parse.h"
#include "stu_rtmp_upstream.h"

stu_int32_t  stu_rtmp_init();
stu_int32_t  stu_rtmp_listen(stu_fd_t evfd, uint16_t port);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_H_ */
