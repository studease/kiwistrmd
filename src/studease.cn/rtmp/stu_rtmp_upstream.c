/*
 * stu_rtmp_upstream.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_rtmp_method_bitmask_t  stu_rtmp_upstream_method_mask[] = {
	{ stu_string("PUSH"),  STU_RTMP_PUSH },
	{ stu_string("PULL"), STU_RTMP_PULL },
	{ stu_null_string, 0 }
};


