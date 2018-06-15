/*
 * stu_golomb.h
 *
 *  Created on: 2018年6月13日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_AV_STU_GOLOMB_H_
#define STUDEASE_CN_AV_STU_GOLOMB_H_

#include "stu_av.h"

typedef stu_bitstream_t stu_golomb_t;

/*
 * Read an unsigned Exp-Golomb code.
 */
static stu_inline stu_uint32_t
stu_golomb_get_ue(stu_golomb_t *gb) {
	stu_int32_t  leading_zero_bits, b;

	leading_zero_bits = -1;

	for (b = 0; !b; leading_zero_bits++) {
		b = stu_bitstream_get_bits(gb, 1);
	}

	return (1 << leading_zero_bits) - 1 + stu_bitstream_get_bits_long(gb, leading_zero_bits);
}

/*
 * Read an signed Exp-Golomb code.
 */
static stu_inline stu_int32_t
stu_golomb_get_se(stu_golomb_t *gb) {
	stu_uint32_t  tmp;

	tmp = stu_golomb_get_ue(gb);
	if (tmp & 0x01) {
		return (tmp + 1) >> 1;
	}

	return - (tmp >> 1);
}

#endif /* STUDEASE_CN_AV_STU_GOLOMB_H_ */
