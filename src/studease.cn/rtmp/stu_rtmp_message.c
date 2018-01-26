/*
 * stu_rtmp_message.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_str_t  STU_RTMP_CMD_CONNECT       = stu_string("connect");
stu_str_t  STU_RTMP_CMD_CLOSE         = stu_string("close");
stu_str_t  STU_RTMP_CMD_CREATE_STREAM = stu_string("createStream");
stu_str_t  STU_RTMP_CMD_RESULT        = stu_string("_result");
stu_str_t  STU_RTMP_CMD_ERROR         = stu_string("_error");

stu_str_t  STU_RTMP_CMD_PLAY          = stu_string("play");
stu_str_t  STU_RTMP_CMD_PLAY2         = stu_string("play2");
stu_str_t  STU_RTMP_CMD_DELETE_STREAM = stu_string("deleteStream");
stu_str_t  STU_RTMP_CMD_CLOSE_STREAM  = stu_string("closeStream");
stu_str_t  STU_RTMP_CMD_RECEIVE_AUDIO = stu_string("receiveAudio");
stu_str_t  STU_RTMP_CMD_RECEIVE_VIDEO = stu_string("receiveVideo");
stu_str_t  STU_RTMP_CMD_PUBLISH       = stu_string("publish");
stu_str_t  STU_RTMP_CMD_SEEK          = stu_string("seek");
stu_str_t  STU_RTMP_CMD_PAUSE         = stu_string("pause");
stu_str_t  STU_RTMP_CMD_ON_STATUS     = stu_string("onStatus");

