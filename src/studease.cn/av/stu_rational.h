/*
 * stu_rational.h
 *
 *  Created on: 2018年5月31日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_AV_STU_RATIONAL_H_
#define STUDEASE_CN_AV_STU_RATIONAL_H_

#include "stu_av.h"

/*
 * rational number numerator/denominator
 */
typedef struct {
	stu_int32_t  num; // numerator
	stu_int32_t  den; // denominator
} stu_rational_t;

#endif /* STUDEASE_CN_AV_STU_RATIONAL_H_ */
