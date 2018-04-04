/*
 * stu_rtmp_amf.c
 *
 *  Created on: 2018年1月16日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

static void *(*stu_rtmp_amf_malloc_fn)(size_t size) = stu_calloc;
static void  (*stu_rtmp_amf_free_fn)(void *ptr) = stu_free;

static size_t  stu_rtmp_amf_parse_value(stu_rtmp_amf_t *item, u_char *data, size_t len, u_char **err);
static size_t  stu_rtmp_amf_parse_number(stu_rtmp_amf_t *item, u_char *data, size_t len, u_char **err);
static size_t  stu_rtmp_amf_parse_string(stu_rtmp_amf_t *item, u_char *data, size_t len, u_char **err);
static size_t  stu_rtmp_amf_parse_object(stu_rtmp_amf_t *object, u_char *data, size_t len, u_char **err);
static size_t  stu_rtmp_amf_parse_ecma_array(stu_rtmp_amf_t *array, u_char *data, size_t len, u_char **err);
static size_t  stu_rtmp_amf_parse_strict_array(stu_rtmp_amf_t *array, u_char *data, size_t len, u_char **err);
static size_t  stu_rtmp_amf_parse_date(stu_rtmp_amf_t *item, u_char *data, size_t len, u_char **err);
static size_t  stu_rtmp_amf_parse_long_string(stu_rtmp_amf_t *item, u_char *data, size_t len, u_char **err);

static u_char *stu_rtmp_amf_print_value(u_char *dst, stu_rtmp_amf_t *item);
static u_char *stu_rtmp_amf_print_number(u_char *dst, stu_rtmp_amf_t *item);
static u_char *stu_rtmp_amf_print_bool(u_char *dst, stu_rtmp_amf_t *item);
static u_char *stu_rtmp_amf_print_string(u_char *dst, stu_rtmp_amf_t *item);
static u_char *stu_rtmp_amf_print_object(u_char *dst, stu_rtmp_amf_t *object);
static u_char *stu_rtmp_amf_print_null(u_char *dst, stu_rtmp_amf_t *item);
static u_char *stu_rtmp_amf_print_undefined(u_char *dst, stu_rtmp_amf_t *item);
static u_char *stu_rtmp_amf_print_ecma_array(u_char *dst, stu_rtmp_amf_t *array);
static u_char *stu_rtmp_amf_print_strict_array(u_char *dst, stu_rtmp_amf_t *array);
static u_char *stu_rtmp_amf_print_date(u_char *dst, stu_rtmp_amf_t *item);
static u_char *stu_rtmp_amf_print_long_string(u_char *dst, stu_rtmp_amf_t *item);

static u_char *stu_rtmp_amf_print_key(u_char *dst, stu_rtmp_amf_t *item);


void
stu_rtmp_amf_init_hooks(stu_rtmp_amf_hooks_t *hooks) {
	if (hooks == NULL || hooks->malloc_fn == NULL) {
		stu_rtmp_amf_malloc_fn = stu_calloc;
		stu_rtmp_amf_free_fn = stu_free;
	} else {
		stu_rtmp_amf_malloc_fn = hooks->malloc_fn;
		stu_rtmp_amf_free_fn = hooks->free_fn;
	}
}


stu_rtmp_amf_t *
stu_rtmp_amf_create(stu_uint8_t type, stu_str_t *key) {
	stu_rtmp_amf_t *item;
	size_t          size;

	size = sizeof(stu_rtmp_amf_t);
	item = (stu_rtmp_amf_t *) stu_rtmp_amf_malloc_fn(size);
	if (item == NULL) {
		return NULL;
	}

	stu_memzero(item, size);
	item->prev = item;
	item->type = type;

	if (key == NULL) {
		goto done;
	}

	if (stu_rtmp_amf_set_key(item, key->data, key->len) != STU_OK) {
		stu_rtmp_amf_delete(item);
		return NULL;
	}

done:

	return item;
}

stu_rtmp_amf_t *
stu_rtmp_amf_create_number(stu_str_t *key, stu_double_t num) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_create(STU_RTMP_AMF_DOUBLE, key);
	if (item == NULL) {
		return NULL;
	}

	item->value = (uintptr_t) stu_rtmp_amf_malloc_fn(8);
	if ((void *) item->value == NULL) {
		stu_rtmp_amf_delete(item);
		return NULL;
	}

	*(stu_double_t *) item->value = num;

	return item;
}

stu_rtmp_amf_t *
stu_rtmp_amf_create_bool(stu_str_t *key, stu_bool_t bool) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_create(STU_RTMP_AMF_BOOLEAN, key);
	if (item == NULL) {
		return NULL;
	}

	item->value = bool;

	return item;
}

stu_rtmp_amf_t *
stu_rtmp_amf_create_true(stu_str_t *key) {
	return stu_rtmp_amf_create_bool(key, TRUE);
}

stu_rtmp_amf_t *
stu_rtmp_amf_create_false(stu_str_t *key) {
	return stu_rtmp_amf_create_bool(key, FALSE);
}

stu_rtmp_amf_t *
stu_rtmp_amf_create_string(stu_str_t *key, u_char *value, size_t len) {
	stu_rtmp_amf_t *item;
	stu_str_t      *str;
	size_t          size;

	if (len >= 0xFFFF) {
		return stu_rtmp_amf_create_long_string(key, value, len);
	}

	item = stu_rtmp_amf_create(len < 0xFFFF ? STU_RTMP_AMF_STRING : STU_RTMP_AMF_LONG_STRING, key);
	if (item == NULL) {
		return NULL;
	}

	size = sizeof(stu_str_t);
	item->value = (uintptr_t) stu_rtmp_amf_malloc_fn(size);
	if ((void *) item->value == NULL) {
		stu_rtmp_amf_delete(item);
		return NULL;
	}

	str = (stu_str_t *) item->value;
	str->data = stu_rtmp_amf_malloc_fn(len + 1);
	if (str->data == NULL) {
		stu_rtmp_amf_delete(item);
		return NULL;
	}

	stu_strncpy(str->data, value, len);
	str->len = len;

	return item;
}

stu_rtmp_amf_t *
stu_rtmp_amf_create_object(stu_str_t *key) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_create(STU_RTMP_AMF_OBJECT, key);
	if (item == NULL) {
		return NULL;
	}

	return item;
}

stu_rtmp_amf_t *
stu_rtmp_amf_create_null(stu_str_t *key) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_create(STU_RTMP_AMF_NULL, key);
	if (item == NULL) {
		return NULL;
	}

	return item;
}

stu_rtmp_amf_t *
stu_rtmp_amf_create_undefined(stu_str_t *key) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_create(STU_RTMP_AMF_UNDEFINED, key);
	if (item == NULL) {
		return NULL;
	}

	return item;
}

stu_rtmp_amf_t *
stu_rtmp_amf_create_ecma_array(stu_str_t *key) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_create(STU_RTMP_AMF_ECMA_ARRAY, key);
	if (item == NULL) {
		return NULL;
	}

	return item;
}

stu_rtmp_amf_t *
stu_rtmp_amf_create_strict_array(stu_str_t *key) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_create(STU_RTMP_AMF_STRICT_ARRAY, key);
	if (item == NULL) {
		return NULL;
	}

	return item;
}

stu_rtmp_amf_t *
stu_rtmp_amf_create_date(stu_str_t *key, stu_double_t ts, stu_uint16_t off) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_create(STU_RTMP_AMF_DATE, key);
	if (item == NULL) {
		return NULL;
	}

	item->value = (uintptr_t) stu_rtmp_amf_malloc_fn(8);
	if ((void *) item->value == NULL) {
		stu_rtmp_amf_delete(item);
		return NULL;
	}

	*(stu_double_t *) item->value = ts + off * 60 * 1000;

	item->timestamp = ts;
	item->timeoffset = off;

	return item;
}


stu_int32_t
stu_rtmp_amf_set_key(stu_rtmp_amf_t *item, u_char *data, size_t len) {
	if (data != NULL) {
		if (item->key.len < len) {
			if (item->key.data != NULL) {
				stu_rtmp_amf_free_fn(item->key.data);
			}

			item->key.data = (u_char *) stu_rtmp_amf_malloc_fn(len + 1);
			if (item->key.data == NULL) {
				return STU_ERROR;
			}
		}

		stu_strncpy(item->key.data, data, len);
		item->key.len = len;
	}

	return STU_OK;
}

stu_rtmp_amf_t *
stu_rtmp_amf_duplicate(stu_rtmp_amf_t *item, stu_bool_t recurse) {
	stu_rtmp_amf_t *copy, *child, *newchild;
	stu_str_t      *str;
	stu_double_t    n;
	stu_bool_t      b;

	if (item == NULL) {
		return NULL;
	}

	switch (item->type) {
	case STU_RTMP_AMF_DOUBLE:
		n = *(stu_double_t *) item->value;
		copy = stu_rtmp_amf_create_number(&item->key, n);
		break;

	case STU_RTMP_AMF_BOOLEAN:
		b = (stu_bool_t) item->value;
		copy = stu_rtmp_amf_create_bool(&item->key, b);
		break;

	case STU_RTMP_AMF_STRING:
	case STU_RTMP_AMF_LONG_STRING:
		str = (stu_str_t *) item->value;
		copy = stu_rtmp_amf_create_string(&item->key, str->data, str->len);
		break;

	case STU_RTMP_AMF_OBJECT:
	case STU_RTMP_AMF_ECMA_ARRAY:
	case STU_RTMP_AMF_STRICT_ARRAY:
		copy = stu_rtmp_amf_create(item->type, &item->key);
		if (recurse && copy != NULL) {
			for (child = (stu_rtmp_amf_t *) item->value; child; child = child->next) {
				newchild = stu_rtmp_amf_duplicate(child, recurse);
				if (newchild == NULL) {
					goto failed;
				}

				stu_rtmp_amf_add_item_to_object(copy, newchild);
			}
		}
		break;

	case STU_RTMP_AMF_DATE:
		copy = stu_rtmp_amf_create_date(&item->key, item->timestamp, item->timeoffset);
		break;

	default:
		copy = stu_rtmp_amf_create(item->type, &item->key);
		break;
	}

	return copy;

failed:

	stu_rtmp_amf_delete(copy);

	return NULL;
}


void
stu_rtmp_amf_add_item_to_array(stu_rtmp_amf_t *array, stu_rtmp_amf_t *item) {
	stu_rtmp_amf_add_item_to_object(array, item);
}

void
stu_rtmp_amf_add_item_to_object(stu_rtmp_amf_t *object, stu_rtmp_amf_t *item) {
	stu_rtmp_amf_t *child;

	if (object == NULL || item == NULL) {
		return;
	}

	if ((void *) object->value == NULL) {
		object->value = (uintptr_t) item;
		item->prev = item; // already set when creating
		return;
	}

	child = (stu_rtmp_amf_t *) object->value;
	child->prev->next = item;
	item->prev = child->prev;
	child->prev = item;
}


stu_rtmp_amf_t *
stu_rtmp_amf_get_array_item_at(stu_rtmp_amf_t *array, stu_int32_t index) {
	stu_rtmp_amf_t *item;

	if (array == NULL) {
		return NULL;
	}

	for (item = (stu_rtmp_amf_t *) array->value; item && index; item = item->next, index--) {
		/* void */
	}

	return item;
}

stu_rtmp_amf_t *
stu_rtmp_amf_get_object_item_by(stu_rtmp_amf_t *object, stu_str_t *key) {
	stu_rtmp_amf_t *item;

	if (object == NULL) {
		return NULL;
	}

	for (item = (stu_rtmp_amf_t *) object->value; item; item = item->next) {
		if (item->key.len != key->len) {
			continue;
		}

		if (stu_strncmp(item->key.data, key->data, key->len) == 0) {
			break;
		}
	}

	return item;
}


stu_rtmp_amf_t *
stu_rtmp_amf_remove_item_from_array(stu_rtmp_amf_t *array, stu_int32_t index) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_get_array_item_at(array, index);
	if (item != NULL) {
		item->prev->next = item->next;
		if (item->next != NULL) {
			item->next->prev = item->prev;
		}

		item->prev = item;
		item->next = NULL;
	}

	return item;
}

stu_rtmp_amf_t *
stu_rtmp_amf_remove_item_from_object(stu_rtmp_amf_t *object, stu_str_t *key) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_get_object_item_by(object, key);
	if (item != NULL) {
		item->prev->next = item->next;
		if (item->next != NULL) {
			item->next->prev = item->prev;
		}

		item->prev = item;
		item->next = NULL;
	}

	return item;
}


void
stu_rtmp_amf_delete(stu_rtmp_amf_t *item) {
	stu_rtmp_amf_t *child;
	stu_str_t      *str;

	if (item == NULL) {
		return;
	}

	switch (item->type) {
	case STU_RTMP_AMF_STRING:
	case STU_RTMP_AMF_LONG_STRING:
		str = (stu_str_t *) item->value;
		if (str != NULL && str->data != NULL) {
			stu_rtmp_amf_free_fn(str->data);
		}
		/* no break */

	case STU_RTMP_AMF_DOUBLE:
	case STU_RTMP_AMF_DATE:
		if ((void *) item->value != NULL) {
			stu_rtmp_amf_free_fn((void *) item->value);
		}
		break;

	case STU_RTMP_AMF_OBJECT:
	case STU_RTMP_AMF_ECMA_ARRAY:
	case STU_RTMP_AMF_STRICT_ARRAY:
		for (child = (stu_rtmp_amf_t *) item->value; child; child = child->next) {
			stu_rtmp_amf_delete(child);
		}
		break;

	default:
		break;
	}

	if (item->key.data != NULL) {
		stu_rtmp_amf_free_fn(item->key.data);
	}

	stu_rtmp_amf_free_fn(item);
}

void
stu_rtmp_amf_delete_item_from_array(stu_rtmp_amf_t *array, stu_int32_t index) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_remove_item_from_array(array, index);
	stu_rtmp_amf_delete(item);
}

void
stu_rtmp_amf_delete_item_from_object(stu_rtmp_amf_t *object, stu_str_t *key) {
	stu_rtmp_amf_t *item;

	item = stu_rtmp_amf_remove_item_from_object(object, key);
	stu_rtmp_amf_delete(item);
}


stu_rtmp_amf_t *
stu_rtmp_amf_parse(u_char *data, size_t len) {
	stu_rtmp_amf_t *root;
	u_char         *err;
	size_t          n;

	err = NULL;

	root = stu_rtmp_amf_create(STU_RTMP_AMF_UNSUPPORTED, NULL);
	if (root == NULL) {
		return NULL;
	}

	n = stu_rtmp_amf_parse_value(root, data, len, &err);
	if (err != NULL) {
		stu_log_debug(4, "Failed to parse AMF: %s", err);
		stu_rtmp_amf_delete(root);
		return NULL;
	}

	root->cost = n;

	return root;
}

static size_t
stu_rtmp_amf_parse_value(stu_rtmp_amf_t *item, u_char *data, size_t len, u_char **err) {
	u_char       *p;
	size_t        n;
	stu_uint32_t  cost;

	p = data;
	cost = 0;

	if (len < 1) {
		stu_log_debug(4, "Data not enough while decoding AMF value.");
		*err = p;
		goto failed;
	}

	item->type = *p++;
	cost++;

	switch (item->type) {
	case STU_RTMP_AMF_DOUBLE:
		n = stu_rtmp_amf_parse_number(item, p, len - cost, err);
		cost += n;
		break;

	case STU_RTMP_AMF_BOOLEAN:
		item->value = *p ? TRUE : FALSE;
		cost += 1;
		break;

	case STU_RTMP_AMF_STRING:
		n = stu_rtmp_amf_parse_string(item, p, len - cost, err);
		cost += n;
		break;

	case STU_RTMP_AMF_OBJECT:
		n = stu_rtmp_amf_parse_object(item, p, len - cost, err);
		cost += n;
		break;

	case STU_RTMP_AMF_NULL:
	case STU_RTMP_AMF_UNDEFINED:
		break;

	case STU_RTMP_AMF_ECMA_ARRAY:
		n = stu_rtmp_amf_parse_ecma_array(item, p, len - cost, err);
		cost += n;
		break;

	case STU_RTMP_AMF_END_OF_OBJECT:
		item->ended = TRUE;
		break;

	case STU_RTMP_AMF_STRICT_ARRAY:
		n = stu_rtmp_amf_parse_strict_array(item, p, len - cost, err);
		cost += n;
		break;

	case STU_RTMP_AMF_DATE:
		n = stu_rtmp_amf_parse_date(item, p, len - cost, err);
		cost += n;
		break;

	case STU_RTMP_AMF_LONG_STRING:
		n = stu_rtmp_amf_parse_long_string(item, p, len - cost, err);
		cost += n;
		break;

	default:
		stu_log_error(0, "Skipping unsupported AMF value type(%x)", item->type);
		cost = len;
		*err = p;
		break;
	}

failed:

	return cost;
}

static size_t
stu_rtmp_amf_parse_number(stu_rtmp_amf_t *item, u_char *data, size_t len, u_char **err) {
	u_char       *p;
	stu_uint32_t  cost;

	if (len < 8) {
		stu_log_error(0, "Data not enough while parsing AMF number.");
		goto failed;
	}

	p = data;
	cost = 0;

	item->value = (uintptr_t) stu_rtmp_amf_malloc_fn(8);
	if ((void *) item->value == NULL) {
		goto failed;
	}

	*(stu_uint64_t *) item->value = stu_endian_64(*(stu_uint64_t *) p);
	cost += 8;

	return cost;

failed:

	*err = p;

	return cost;
}

static size_t
stu_rtmp_amf_parse_string(stu_rtmp_amf_t *item, u_char *data, size_t len, u_char **err) {
	u_char       *p;
	stu_str_t    *str;
	stu_uint32_t  cost;

	if (len < 2) {
		stu_log_error(0, "Data not enough while parsing AMF string.");
		goto failed;
	}

	p = data;
	cost = 0;

	item->value = (uintptr_t) stu_rtmp_amf_malloc_fn(sizeof(stu_str_t));
	if ((void *) item->value == NULL) {
		goto failed;
	}

	str = (stu_str_t *) item->value;
	str->len = stu_endian_16(*(stu_uint16_t *) p);
	p += 2;
	cost += 2;

	str->data = stu_rtmp_amf_malloc_fn(str->len + 1);
	if (str->data == NULL) {
		goto failed;
	}

	stu_strncpy(str->data, p, str->len);
	p += str->len;
	cost += str->len;

	return cost;

failed:

	*err = p;

	return cost;
}

static size_t
stu_rtmp_amf_parse_object(stu_rtmp_amf_t *object, u_char *data, size_t len, u_char **err) {
	stu_rtmp_amf_t *item;
	u_char         *p;
	size_t          n;
	stu_uint16_t    m;
	stu_uint32_t    cost;

	if (len < 3) {
		stu_log_error(0, "Data not enough while parsing AMF object.");
		goto failed;
	}

	p = data;
	cost = 0;

	for ( ; object->ended == FALSE && object->cost < len; ) {
		m = stu_endian_16(*(stu_uint16_t *) p);
		p += 2;
		cost += 2;

		if (m == 0 && *p == STU_RTMP_AMF_END_OF_OBJECT) {
			object->ended = TRUE;
			p += 1;
			cost += 1;
			break;
		}

		item = stu_rtmp_amf_create(STU_RTMP_AMF_UNSUPPORTED, NULL);
		if (item == NULL) {
			goto failed;
		}

		if (stu_rtmp_amf_set_key(item, p, m) != STU_OK) {
			stu_rtmp_amf_delete(item);
			goto failed;
		}

		p += m;
		cost += m;

		n = stu_rtmp_amf_parse_value(item, p, len - cost, err);
		p += n;
		cost += n;

		stu_rtmp_amf_add_item_to_object(object, item);
	}

	return cost;

failed:

	*err = p;

	return cost;
}

static size_t
stu_rtmp_amf_parse_ecma_array(stu_rtmp_amf_t *array, u_char *data, size_t len, u_char **err) {
	stu_rtmp_amf_t *item;
	u_char         *p;
	size_t          n;
	stu_uint32_t    i, m, cost;

	if (len < 4) {
		stu_log_error(0, "Data not enough while parsing AMF ECMA array.");
		goto failed;
	}

	p = data;
	cost = 0;

	m = stu_endian_32(*(stu_uint32_t *) p);
	p += 4;
	cost += 4;

	for (i = 0; i < m && array->ended == FALSE && cost < len; i++) {
		n = stu_endian_16(*(stu_uint16_t *) p);
		p += 2;
		cost += 2;

		if (n == 0 && *p == STU_RTMP_AMF_END_OF_OBJECT) {
			array->ended = TRUE;
			p += 1;
			cost += 1;
			break;
		}

		item = stu_rtmp_amf_create(STU_RTMP_AMF_UNSUPPORTED, NULL);
		if (item == NULL) {
			goto failed;
		}

		if (stu_rtmp_amf_set_key(item, p, n) != STU_OK) {
			stu_rtmp_amf_delete(item);
			goto failed;
		}

		p += n;
		cost += n;

		n = stu_rtmp_amf_parse_value(item, p, len - cost, err);
		p += n;
		cost += n;

		stu_rtmp_amf_add_item_to_object(array, item);
	}

	return cost;

failed:

	*err = p;

	return cost;
}

static size_t
stu_rtmp_amf_parse_strict_array(stu_rtmp_amf_t *array, u_char *data, size_t len, u_char **err) {
	stu_rtmp_amf_t *item;
	u_char         *p;
	size_t          n;
	stu_uint32_t    i, m, cost;

	if (len < 4) {
		stu_log_error(0, "Data not enough while parsing AMF ECMA array.");
		goto failed;
	}

	p = data;
	cost = 0;

	m = stu_endian_32(*(stu_uint32_t *) p);
	p += 4;
	cost += 4;

	for (i = 0; i < m && array->ended == FALSE && cost < len; i++) {
		if (*p == STU_RTMP_AMF_END_OF_OBJECT) {
			array->ended = TRUE;
			p += 1;
			cost += 1;
			break;
		}

		item = stu_rtmp_amf_create(STU_RTMP_AMF_UNSUPPORTED, NULL);
		if (item == NULL) {
			goto failed;
		}

		n = stu_rtmp_amf_parse_value(item, p, len - cost, err);
		p += n;
		cost += n;

		stu_rtmp_amf_add_item_to_object(array, item);
	}

	return cost;

failed:

	*err = p;

	return cost;
}

static size_t
stu_rtmp_amf_parse_date(stu_rtmp_amf_t *item, u_char *data, size_t len, u_char **err) {
	u_char       *p;
	stu_double_t  off;
	stu_uint32_t  cost;

	if (len < 10) {
		stu_log_error(0, "Data not enough while parsing AMF date.");
		goto failed;
	}

	p = data;
	cost = 0;

	item->value = (uintptr_t) stu_rtmp_amf_malloc_fn(8);
	if ((void *) item->value == NULL) {
		goto failed;
	}

	item->timestamp = stu_endian_64(*(stu_uint64_t *) p);
	p += 8;
	cost += 8;

	item->timeoffset = off = stu_endian_16(*(stu_uint16_t *) p);
	p += 2;
	cost += 2;

	*(stu_double_t *) item->value = item->timestamp + off * 60 * 1000;

	return cost;

failed:

	*err = p;

	return cost;
}

static size_t
stu_rtmp_amf_parse_long_string(stu_rtmp_amf_t *item, u_char *data, size_t len, u_char **err) {
	u_char       *p;
	stu_str_t    *str;
	stu_uint32_t  cost;

	if (len < 4) {
		stu_log_error(0, "Data not enough while parsing AMF string.");
		goto failed;
	}

	p = data;
	cost = 0;

	item->value = (uintptr_t) stu_rtmp_amf_malloc_fn(sizeof(stu_str_t));
	if ((void *) item->value == NULL) {
		goto failed;
	}

	str = (stu_str_t *) item->value;
	str->len = stu_endian_32(*(stu_uint32_t *) p);
	p += 4;
	cost += 4;

	str->data = stu_rtmp_amf_malloc_fn(str->len + 1);
	if (str->data == NULL) {
		goto failed;
	}

	stu_strncpy(str->data, p, str->len);
	p += str->len;
	cost += str->len;

	return cost;

failed:

	*err = p;

	return cost;
}


u_char *
stu_rtmp_amf_stringify(u_char *dst, stu_rtmp_amf_t *item) {
	return stu_rtmp_amf_print_value(dst, item);
}

static u_char *
stu_rtmp_amf_print_value(u_char *dst, stu_rtmp_amf_t *item) {
	u_char *p;

	switch (item->type) {
	case STU_RTMP_AMF_DOUBLE:
		p = stu_rtmp_amf_print_number(dst, item);
		break;

	case STU_RTMP_AMF_BOOLEAN:
		p = stu_rtmp_amf_print_bool(dst, item);
		break;

	case STU_RTMP_AMF_STRING:
		p = stu_rtmp_amf_print_string(dst, item);
		break;

	case STU_RTMP_AMF_OBJECT:
		p = stu_rtmp_amf_print_object(dst, item);
		break;

	case STU_RTMP_AMF_NULL:
		p = stu_rtmp_amf_print_null(dst, item);
		break;

	case STU_RTMP_AMF_UNDEFINED:
		p = stu_rtmp_amf_print_undefined(dst, item);
		break;

	case STU_RTMP_AMF_ECMA_ARRAY:
		p = stu_rtmp_amf_print_ecma_array(dst, item);
		break;

	case STU_RTMP_AMF_STRICT_ARRAY:
		p = stu_rtmp_amf_print_strict_array(dst, item);
		break;

	case STU_RTMP_AMF_DATE:
		p = stu_rtmp_amf_print_date(dst, item);
		break;

	default:
		break;
	}

	return p;
}

static u_char *
stu_rtmp_amf_print_number(u_char *dst, stu_rtmp_amf_t *item) {
	u_char       *p;
	stu_uint64_t  n;

	p = dst;

	*p++ = STU_RTMP_AMF_DOUBLE;

	n = stu_endian_64(*(stu_uint64_t *) item->value);
	p = stu_memcpy(p, &n, 8);

	return p;
}

static u_char *
stu_rtmp_amf_print_bool(u_char *dst, stu_rtmp_amf_t *item) {
	u_char *p;

	p = dst;

	*p++ = STU_RTMP_AMF_BOOLEAN;
	*p++ = item->value ? TRUE : FALSE;

	return p;
}

static u_char *
stu_rtmp_amf_print_string(u_char *dst, stu_rtmp_amf_t *item) {
	u_char       *p;
	stu_str_t    *str;
	stu_uint16_t  n;

	p = dst;
	str = (stu_str_t *) item->value;

	if (str->len >= 0xFFFF) {
		return stu_rtmp_amf_print_long_string(dst, item);
	}

	*p++ = STU_RTMP_AMF_STRING;

	n = stu_endian_16((stu_uint16_t) str->len);
	p = stu_memcpy(p, &n, 2);

	p = stu_memcpy(p, str->data, str->len);

	return p;
}

static u_char *
stu_rtmp_amf_print_object(u_char *dst, stu_rtmp_amf_t *object) {
	u_char         *p;
	stu_rtmp_amf_t *item;
	stu_uint16_t    n;

	p = dst;
	n = 0;

	*p++ = STU_RTMP_AMF_OBJECT;

	for (item = (stu_rtmp_amf_t *) object->value; item; item = item->next) {
		p = stu_rtmp_amf_print_key(p, item);
		p = stu_rtmp_amf_print_value(p, item);
	}

	p = stu_memcpy(p, &n, 2);
	*p++ = STU_RTMP_AMF_END_OF_OBJECT;

	return p;
}

static u_char *
stu_rtmp_amf_print_null(u_char *dst, stu_rtmp_amf_t *item) {
	u_char *p;

	p = dst;

	*p++ = STU_RTMP_AMF_NULL;

	return p;
}

static u_char *
stu_rtmp_amf_print_undefined(u_char *dst, stu_rtmp_amf_t *item) {
	u_char *p;

	p = dst;

	*p++ = STU_RTMP_AMF_UNDEFINED;

	return p;
}

static u_char *
stu_rtmp_amf_print_ecma_array(u_char *dst, stu_rtmp_amf_t *array) {
	u_char         *p;
	stu_rtmp_amf_t *item;
	stu_uint32_t   *len, i;
	stu_uint16_t    n;

	p = dst;
	n = 0;

	*p++ = STU_RTMP_AMF_ECMA_ARRAY;

	len = (stu_uint32_t *) p;
	p += 4;

	for (item = (stu_rtmp_amf_t *) array->value, i = 0; item; item = item->next, i++) {
		p = stu_rtmp_amf_print_key(p, item);
		p = stu_rtmp_amf_print_value(p, item);
	}

	*len = stu_endian_32(i);

	p = stu_memcpy(p, &n, 2);
	*p++ = STU_RTMP_AMF_END_OF_OBJECT;

	return p;
}

static u_char *
stu_rtmp_amf_print_strict_array(u_char *dst, stu_rtmp_amf_t *array) {
	u_char         *p;
	stu_rtmp_amf_t *item;
	stu_uint32_t   *len, i;
	stu_uint16_t    n;

	p = dst;
	n = 0;

	*p++ = STU_RTMP_AMF_ECMA_ARRAY;

	len = (stu_uint32_t *) p;

	for (item = (stu_rtmp_amf_t *) array->value, i = 0; item; item = item->next, i++) {
		p = stu_rtmp_amf_print_value(p, item);
	}

	*len = stu_endian_32(i);

	p = stu_memcpy(p, &n, 2);
	*p++ = STU_RTMP_AMF_END_OF_OBJECT;

	return p;
}

static u_char *
stu_rtmp_amf_print_date(u_char *dst, stu_rtmp_amf_t *item) {
	u_char       *p;
	stu_uint64_t  n;
	stu_uint16_t  m;

	p = dst;

	*p++ = STU_RTMP_AMF_DATE;

	n = stu_endian_64(*(stu_uint64_t *) &item->timestamp);
	p = stu_memcpy(p, &n, 8);

	m = stu_endian_64(*(stu_uint16_t *) &item->timeoffset);
	p = stu_memcpy(p, &m, 2);

	return p;
}

static u_char *
stu_rtmp_amf_print_long_string(u_char *dst, stu_rtmp_amf_t *item) {
	u_char       *p;
	stu_str_t    *str;
	stu_uint32_t  n;

	p = dst;
	str = (stu_str_t *) item->value;

	*p++ = STU_RTMP_AMF_LONG_STRING;

	n = stu_endian_32((stu_uint32_t) str->len);
	p = stu_memcpy(p, &n, 4);

	p = stu_memcpy(p, str->data, str->len);

	return p;
}


static u_char *
stu_rtmp_amf_print_key(u_char *dst, stu_rtmp_amf_t *item) {
	u_char       *p;
	stu_uint16_t  n;

	p = dst;

	n = stu_endian_16((stu_uint16_t) item->key.len);
	p = stu_memcpy(p, &n, 2);

	p = stu_memcpy(p, item->key.data, item->key.len);

	return p;
}
