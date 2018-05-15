/*
 * stu_rtmp_stream.c
 *
 *  Created on: 2018年5月9日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"


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

	if (stu_hash_init(&s->subscribers, STU_RTMP_SUBSCRIBER_DEFAULT_SIZE, NULL, STU_HASH_FLAGS_LOWCASE) == STU_ERROR) {
		stu_log_error(0, "Failed to init rtmp subscriber hash: %s.", s->name.data);
		goto failed;
	}

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

/*
 * Lock instance at first. And unlink publisher to this.
 */
void
stu_rtmp_stream_free(stu_rtmp_stream_t *s) {
	stu_hash_destroy_locked(&s->subscribers, (stu_hash_cleanup_pt) stu_rtmp_stream_detach);
	stu_hash_destroy_locked(&s->data_frames, (stu_hash_cleanup_pt) stu_rtmp_amf_delete);

	if (s->name.data) {
		stu_free(s->name.data);
	}

	stu_free(s);
}
