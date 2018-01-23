/*
 * stu_endian.c
 *
 *  Created on: 2018年1月17日
 *      Author: Tony Lau
 */

#include "stu_utils.h"


stu_inline stu_uint16_t
stu_endian_16(stu_uint16_t n) {
	return (n << 8) | (n >> 8);
}

stu_inline stu_uint32_t
stu_endian_32(stu_uint32_t n) {
	return (n << 24) | ((n << 8) & 0xFF0000) | ((n >> 8) & 0xFF00) | (n >> 24);
}

stu_inline stu_uint64_t
stu_endian_64(stu_uint64_t n) {
	return (stu_uint64_t) stu_endian_32((stu_uint32_t) n) << 32 | stu_endian_32((stu_uint32_t) (n >> 32));
}
