/*
 * stu_bitstream.h
 *
 *  Created on: 2018年6月13日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_AV_STU_BITSTREAM_H_
#define STUDEASE_CN_AV_STU_BITSTREAM_H_

#include "../utils/stu_endian.h"
#include "stu_av.h"

#define STU_BITSTREAM_MIN_CACHE_BITS 25

typedef struct {
	u_char      *start;
	u_char      *end;
	stu_int32_t  index;
	stu_int32_t  bits;
} stu_bitstream_t;

static stu_inline stu_int32_t
stu_bitstream_init(stu_bitstream_t *s, u_char *buf, stu_int32_t bytes) {
	stu_int32_t  bits, size;

	if (bytes > INT_MAX / 8 || bytes < 0) {
		bytes = -1;
	}

	bits = bytes * 8;

	if (buf == NULL || bits >= INT_MAX - 7 || bits < 0) {
		return STU_ERROR;
	}

	size = (bits + 7) >> 3;

	s->start = buf;
	s->end = buf + size;
	s->index = 0;
	s->bits = bits;

	return STU_OK;
}

static stu_inline stu_int32_t
stu_bitstream_left(stu_bitstream_t *s) {
	return s->bits - s->index;
}

/*
 * Show 0-25 bits.
 */
static stu_inline stu_uint32_t
stu_bitstream_show_bits(stu_bitstream_t *s, stu_int32_t n) {
	stu_uint32_t  cache;

	if (n == 0) {
		return 0;
	}

	cache = stu_endian_32(*(stu_uint32_t *) (s->start + (s->index >> 3))) << (s->index & 7);

	return cache >> (32 - n);
}

/*
 * Read 0-25 bits.
 */
static stu_inline stu_uint32_t
stu_bitstream_get_bits(stu_bitstream_t *s, stu_int32_t n) {
	register stu_uint32_t  tmp;

	tmp = stu_bitstream_show_bits(s, n);
	s->index += n;

	return tmp;
}

/*
 * Show 0-32 bits.
 */
static stu_inline stu_uint32_t
stu_bitstream_show_bits_long(stu_bitstream_t *s, stu_int32_t n) {
	if (n <= STU_BITSTREAM_MIN_CACHE_BITS) {
		return stu_bitstream_show_bits(s, n);
	} else {
		return stu_bitstream_show_bits(s, 16) << (n - 16) | stu_bitstream_show_bits(s, n - 16);
	}
}

/*
 * Read 0-32 bits.
 */
static stu_inline stu_uint32_t
stu_bitstream_get_bits_long(stu_bitstream_t *s, stu_int32_t n) {
	register stu_uint32_t  tmp;

	tmp = stu_bitstream_show_bits_long(s, n);
	s->index += n;

	return tmp;
}

static stu_inline void
stu_bitstream_skip_bits(stu_bitstream_t *s, stu_int32_t n) {
	s->index += n;
}

static stu_inline void
stu_bitstream_destroy(stu_bitstream_t *s) {
	if (s->start) {
		stu_free(s->start);
	}

	s->start = NULL;
	s->end = NULL;
	s->index = 0;
	s->bits = 0;
}

#endif /* STUDEASE_CN_AV_STU_BITSTREAM_H_ */
