/*
 * stu_rtmp_phase.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

stu_list_t  stu_rtmp_phases;


stu_int32_t
stu_rtmp_phase_init() {
	stu_list_init(&stu_rtmp_phases, NULL);
	return STU_OK;
}
