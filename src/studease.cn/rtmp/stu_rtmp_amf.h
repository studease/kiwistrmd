/*
 * stu_rtmp_amf.h
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_RTMP_STU_RTMP_AMF_H_
#define STUDEASE_CN_RTMP_STU_RTMP_AMF_H_

#include "stu_rtmp.h"

#define STU_RTMP_AMF0               0x00
#define STU_RTMP_AMF3               0x03

#define STU_RTMP_AMF_DOUBLE         0x00
#define STU_RTMP_AMF_BOOLEAN        0x01
#define STU_RTMP_AMF_STRING         0x02
#define STU_RTMP_AMF_OBJECT         0x03
#define STU_RTMP_AMF_MOVIE_CLIP     0x04 // Not available in remoting
#define STU_RTMP_AMF_NULL           0x05
#define STU_RTMP_AMF_UNDEFINED      0x06
#define STU_RTMP_AMF_REFERENCE      0x07
#define STU_RTMP_AMF_ECMA_ARRAY     0x08
#define STU_RTMP_AMF_END_OF_OBJECT  0x09
#define STU_RTMP_AMF_STRICT_ARRAY   0x0A
#define STU_RTMP_AMF_DATE           0x0B
#define STU_RTMP_AMF_LONG_STRING    0x0C
#define STU_RTMP_AMF_UNSUPPORTED    0x0D
#define STU_RTMP_AMF_RECORD_SET     0x0E // Remoting server-to-client only
#define STU_RTMP_AMF_XML            0x0F
#define STU_RTMP_AMF_TYPED_OBJECT   0x10 // Class instance
#define STU_RTMP_AMF_AMF3_DATA      0x11 // Sent by Flash player 9+

typedef struct stu_rtmp_amf_s stu_rtmp_amf_t;

struct stu_rtmp_amf_s {
	stu_uint8_t     type;
	stu_str_t       key;
	uintptr_t       value;
	stu_double_t    timestamp;
	stu_uint16_t    timeoffset;
	stu_uint32_t    cost;
	stu_bool_t      ended;

	stu_rtmp_amf_t *prev;
	stu_rtmp_amf_t *next;
};

typedef struct {
	void *(*malloc_fn)(size_t size);
	void  (*free_fn)(void *ptr);
} stu_rtmp_amf_hooks_t;

void            stu_rtmp_amf_init_hooks(stu_rtmp_amf_hooks_t *hooks);

stu_rtmp_amf_t *stu_rtmp_amf_create(stu_uint8_t type, stu_str_t *key);
stu_rtmp_amf_t *stu_rtmp_amf_create_number(stu_str_t *key, stu_double_t num);
stu_rtmp_amf_t *stu_rtmp_amf_create_bool(stu_str_t *key, stu_bool_t bool);
stu_rtmp_amf_t *stu_rtmp_amf_create_true(stu_str_t *key);
stu_rtmp_amf_t *stu_rtmp_amf_create_false(stu_str_t *key);
stu_rtmp_amf_t *stu_rtmp_amf_create_string(stu_str_t *key, u_char *value, size_t len);
stu_rtmp_amf_t *stu_rtmp_amf_create_object(stu_str_t *key);
stu_rtmp_amf_t *stu_rtmp_amf_create_null(stu_str_t *key);
stu_rtmp_amf_t *stu_rtmp_amf_create_undefined(stu_str_t *key);
stu_rtmp_amf_t *stu_rtmp_amf_create_ecma_array(stu_str_t *key);
stu_rtmp_amf_t *stu_rtmp_amf_create_strict_array(stu_str_t *key);
stu_rtmp_amf_t *stu_rtmp_amf_create_date(stu_str_t *key, stu_double_t ts, stu_uint16_t off);

stu_int32_t     stu_rtmp_amf_set_key(stu_rtmp_amf_t *item, u_char *data, size_t len);
stu_rtmp_amf_t *stu_rtmp_amf_duplicate(stu_rtmp_amf_t *item, stu_bool_t recurse);

void            stu_rtmp_amf_add_item_to_array(stu_rtmp_amf_t *array, stu_rtmp_amf_t *item);
void            stu_rtmp_amf_add_item_to_object(stu_rtmp_amf_t *object, stu_rtmp_amf_t *item);

stu_rtmp_amf_t *stu_rtmp_amf_get_array_item_at(stu_rtmp_amf_t *array, stu_int32_t index);
stu_rtmp_amf_t *stu_rtmp_amf_get_object_item_by(stu_rtmp_amf_t *object, stu_str_t *key);

stu_rtmp_amf_t *stu_rtmp_amf_remove_item_from_array(stu_rtmp_amf_t *array, stu_int32_t index);
stu_rtmp_amf_t *stu_rtmp_amf_remove_item_from_object(stu_rtmp_amf_t *object, stu_str_t *key);

void            stu_rtmp_amf_delete(stu_rtmp_amf_t *item);

void            stu_rtmp_amf_delete_item_from_array(stu_rtmp_amf_t *array, stu_int32_t index);
void            stu_rtmp_amf_delete_item_from_object(stu_rtmp_amf_t *object, stu_str_t *key);

stu_rtmp_amf_t *stu_rtmp_amf_parse(u_char *data, size_t len);
u_char         *stu_rtmp_amf_stringify(u_char *dst, stu_rtmp_amf_t *item);

#endif /* STUDEASE_CN_RTMP_STU_RTMP_AMF_H_ */
