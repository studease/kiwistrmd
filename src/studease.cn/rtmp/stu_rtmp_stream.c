/*
 * stu_rtmp_stream.c
 *
 *  Created on: 2018年5月9日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

static stu_rtmp_frame_t *stu_rtmp_frame_create(stu_uint8_t type, stu_uint32_t timestamp, stu_rtmp_frame_info_t *info, u_char *data, size_t len);


stu_rtmp_stream_t *
stu_rtmp_stream_get(u_char *name, size_t len) {
	stu_rtmp_stream_t *s;

	s = stu_calloc(sizeof(stu_rtmp_stream_t));
	if (s == NULL) {
		stu_log_error(0, "Failed to calloc rtmp stream: name=%s.", name);
		return NULL;
	}

	if (name && len) {
		s->name.data = stu_calloc(len + 1);
		if (s->name.data == NULL) {
			stu_log_error(0, "Failed to calloc rtmp stream name: %s.", name);
			goto failed;
		}

		stu_strncpy(s->name.data, name, len);
		s->name.len = len;
	}

	stu_list_init(&s->frames, NULL);

	if (stu_hash_init(&s->data_frames, STU_RTMP_DATA_FRAMES_SIZE, NULL, STU_HASH_FLAGS_LOWCASE) == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp data frame hash: %s.", s->name.data);
		goto failed;
	}

	return s;

failed:

	if (s) {
		if (s->name.data) {
			stu_free(s->name.data);
		}

		stu_free(s);
	}

	return NULL;
}

stu_rtmp_frame_t *
stu_rtmp_stream_append(stu_rtmp_stream_t *s, stu_uint8_t type, stu_uint32_t timestamp, stu_rtmp_frame_info_t *info, u_char *data, size_t len) {
	stu_rtmp_frame_t *f;

	f = stu_rtmp_frame_create(type, timestamp, info, data, len);
	if (f == NULL) {
		stu_log_error(0, "Failed to create rtmp frame: type=0x%02X, ts=%u, size=%u.", type, timestamp, len);
		return NULL;
	}

	if (s->frames.length == 0) {
		s->time = f->timestamp;
	}

	s->duration = f->timestamp;

	stu_list_insert_tail(&s->frames, f);

	return f;
}

void
stu_rtmp_stream_drop(stu_rtmp_stream_t *s, stu_uint32_t timestamp) {
	stu_rtmp_frame_t *f;
	stu_list_t       *list;
	stu_list_elt_t   *elts, *e;
	stu_queue_t      *q;

	list = &s->frames;
	elts = &list->elts;

	for (q = stu_queue_tail(&elts->queue); q != stu_queue_sentinel(&elts->queue); q = stu_queue_prev(q)) {
		e = stu_queue_data(q, stu_list_elt_t, queue);
		f = e->value;

		if (f->type != STU_RTMP_MESSAGE_TYPE_VIDEO || f->info.frame_type != 0x1 || f->timestamp >= timestamp) {
			continue;
		}

		s->time = f->timestamp;

		for (q = stu_queue_prev(q); q != stu_queue_sentinel(&elts->queue); q = stu_queue_prev(q)) {
			e = stu_queue_data(q, stu_list_elt_t, queue);
			f = e->value;

			stu_list_remove(list, e);

			stu_free(f->payload.start);
			stu_free(f);
		}

		break;
	}
}

/*
 * Lock instance at first. And unlink publisher to this.
 */
void
stu_rtmp_stream_free(stu_rtmp_stream_t *s) {
	stu_hash_destroy_locked(&s->data_frames, (stu_hash_cleanup_pt) stu_rtmp_amf_delete);

	if (s->name.data) {
		stu_free(s->name.data);
	}

	stu_free(s);
}


static stu_rtmp_frame_t *
stu_rtmp_frame_create(stu_uint8_t type, stu_uint32_t timestamp, stu_rtmp_frame_info_t *info, u_char *data, size_t len) {
	stu_rtmp_frame_t *f;

	if (len == 0) {
		return NULL;
	}

	f = stu_calloc(sizeof(stu_rtmp_frame_t));
	if (f == NULL) {
		stu_log_error(0, "Failed to calloc rtmp frame: fd=%d, ts=%u.");
		return NULL;
	}

	f->type = type;
	f->timestamp = timestamp;
	f->info = *info;

	f->payload.start = stu_calloc(len);
	if (f->payload.start == NULL) {
		stu_log_error(0, "Failed to calloc rtmp frame payload: fd=%d, ts=%u.");
		stu_free(f);
		return NULL;
	}

	f->payload.pos = f->payload.start;
	f->payload.end = f->payload.start + len;
	f->payload.size = len;

	f->payload.last = stu_memcpy(f->payload.start, data, len);

	return f;
}
