/*
 * stu_rtmp_fmp4_muxer.c
 *
 *  Created on: 2018年5月17日
 *      Author: Tony Lau
 */

#include "stu_rtmp.h"

static stu_mp4_t   *stu_rtmp_phase_get_fmp4(stu_rtmp_request_t *r);
static stu_int32_t  stu_rtmp_phase_fmp4_data_handler(stu_rtmp_request_t *r, stu_mp4_t *mp4);
static stu_int32_t  stu_rtmp_phase_fmp4_frame_handler(stu_rtmp_request_t *r, stu_mp4_t *mp4);

static stu_int32_t  stu_fmp4_get_init_segment(stu_buf_t *dst, stu_mp4_track_t *track);
static stu_int32_t  stu_fmp4_get_video_segment(stu_buf_t *dst, stu_mp4_track_t *track);
static stu_int32_t  stu_fmp4_get_audio_segment(stu_buf_t *dst, stu_mp4_track_t *track);

extern stu_str_t  stu_rtmp_root;

stu_hash_t        stu_rtmp_phase_fmp4_hash;


stu_int32_t
stu_rtmp_phase_fmp4_init() {
	if (stu_hash_init(&stu_rtmp_phase_fmp4_hash, STU_RTMP_PHASE_FMP4_MAXIMUM, NULL, STU_HASH_FLAGS_LOWCASE) == STU_ERROR) {
		return STU_ERROR;
	}

	return STU_OK;
}


stu_int32_t
stu_rtmp_phase_fmp4_handler(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_chunk_t         *ck;
	stu_mp4_t                *mp4;
	stu_int32_t               rc;

	nc = &r->connection;
	ck = r->chunk_in;
	rc = STU_OK;

	mp4 = stu_rtmp_phase_get_fmp4(r);
	if (mp4 == NULL) {
		stu_log_error(0, "Failed to get fmp4: fd=%d.", nc->conn->fd);
		return STU_ERROR;
	}

	mp4->timestamp += ck->timestamp;

	switch (ck->type_id) {
	case STU_RTMP_MESSAGE_TYPE_AMF3_DATA:
	case STU_RTMP_MESSAGE_TYPE_DATA:
		if (stu_strncmp(STU_RTMP_ON_META_DATA.data, r->data_key->data, r->data_key->len) != 0) {
			break;
		}

		rc = stu_rtmp_phase_fmp4_data_handler(r, mp4);
		if (rc == STU_ERROR) {
			stu_log_error(0, "Failed to handle rtmp data frame on fmp4 phase.");
		}
		break;

	case STU_RTMP_MESSAGE_TYPE_VIDEO:
	case STU_RTMP_MESSAGE_TYPE_AUDIO:
		if (stu_rtmp_phase_fmp4_frame_handler(r, mp4) == STU_ERROR) {
			stu_log_error(0, "Failed to handle rtmp frame on fmp4 phase.");
			return STU_ERROR;
		}

		rc = stu_file_write(&mp4->file, mp4->buffer.start, mp4->buffer.size, mp4->file.offset);
		if (rc == STU_ERROR) {
			stu_log_error(0, "Failed to write fmp4 segment: fd=%d.", nc->conn->fd);
		}
		break;

	default:
		/* Just ignore */
		break;
	}

	return rc;
}

static stu_int32_t
stu_rtmp_phase_fmp4_data_handler(stu_rtmp_request_t *r, stu_mp4_t *mp4) {
	stu_rtmp_amf_t *item;
	stu_str_t       key;
	stu_double_t    fps;

	stu_str_set(&key, "width");
	item = stu_rtmp_amf_get_object_item_by(r->data_value, &key);
	if (item) {
		mp4->width = *(stu_double_t *) item->value;
	}

	stu_str_set(&key, "height");
	item = stu_rtmp_amf_get_object_item_by(r->data_value, &key);
	if (item) {
		mp4->height = *(stu_double_t *) item->value;
	}

	stu_str_set(&key, "framerate");
	item = stu_rtmp_amf_get_object_item_by(r->data_value, &key);
	if (item) {
		fps = *(stu_double_t *) item->value;
		mp4->framerate.num = fps * 1000;
		mp4->framerate.den = 1000;
	}

	stu_str_set(&key, "duration");
	item = stu_rtmp_amf_get_object_item_by(r->data_value, &key);
	if (item) {
		mp4->duration = *(stu_double_t *) item->value;
	}

	stu_str_set(&key, "filesize");
	item = stu_rtmp_amf_get_object_item_by(r->data_value, &key);
	if (item) {
		mp4->filesize = *(stu_double_t *) item->value;
	}

	return STU_OK;
}

static stu_int32_t
stu_rtmp_phase_fmp4_frame_handler(stu_rtmp_request_t *r, stu_mp4_t *mp4) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_chunk_t         *ck;
	stu_mp4_track_t          *track;
	stu_av_codec_context_t   *ctx;
	stu_buf_t                *buf;
	stu_int32_t               rc;

	nc = &r->connection;
	ck = r->chunk_in;
	buf = &ck->payload;

	if (r->frame_info.data_type == 0) {
		track = stu_mp4_create_track(mp4, ck->type_id,
				ck->type_id == STU_RTMP_MESSAGE_TYPE_AUDIO ? r->frame_info.format : r->frame_info.codec);
	} else {
		track = stu_mp4_get_track(mp4, ck->type_id);
	}

	if (track == NULL) {
		stu_log_error(0, "Failed to get track.");
		return STU_ERROR;
	}

	ctx = track->ctx;

	if (ctx->parse_pt(ctx, mp4->timestamp, buf->start, buf->size) == STU_ERROR) {
		stu_log_error(0, "Failed to parse av packet.");
		return STU_ERROR;
	}

	if (r->frame_info.data_type == 0) {
		rc = stu_fmp4_get_init_segment(&mp4->buffer, track);
	} else {
		if (ck->type_id == STU_RTMP_MESSAGE_TYPE_AUDIO) {
			rc = stu_fmp4_get_audio_segment(&mp4->buffer, track);
		} else {
			rc = stu_fmp4_get_video_segment(&mp4->buffer, track);
		}
	}

	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to get mp4 segment: fd=%d.", nc->conn->fd);
		return STU_ERROR;
	}

	return stu_mp4_flush(mp4);
}

static stu_int32_t
stu_fmp4_get_init_segment(stu_buf_t *dst, stu_mp4_track_t *track) {
	stu_mp4_box_t *ftyp, *moov;

	ftyp = stu_mp4_ftyp(track);
	moov = stu_mp4_moov(track);

	return stu_mp4_merge(dst, ftyp, moov, NULL);
}

static stu_int32_t
stu_fmp4_get_video_segment(stu_buf_t *dst, stu_mp4_track_t *track) {
	stu_av_codec_context_t *ctx;
	stu_mp4_box_t          *moof, *mdat;
	stu_int32_t             delta;

	ctx = track->ctx;
	ctx->dts -= ctx->timebase;

	delta = ctx->dts - ctx->expected_dts;
	ctx->dts = ctx->expected_dts;
	ctx->pts = ctx->cts + ctx->dts;
	ctx->duration = ctx->ref_sample_duration + delta;
	ctx->expected_dts = ctx->dts + ctx->duration;

	ctx->flags.is_leading = 0;
	ctx->flags.sample_depends_on = ctx->keyframe ? 2 : 1;
	ctx->flags.sample_is_depended_on = ctx->keyframe ? 1 : 0;
	ctx->flags.sample_has_redundancy = 0;
	ctx->flags.is_non_sync = ctx->keyframe ? 0 : 1;

	// workaround for chrome < 50: force first sample as a random access point
	// see https://bugs.chromium.org/p/chromium/issues/detail?id=229412
	/*if (_forceFirstIDR) {
		var flags = mp4Samples[0].flags;
		flags.dependsOn = 2;
		flags.isNonSync = 0;
	}*/

	track->sequence_number++;

	moof = stu_mp4_moof(track);
	mdat = stu_mp4_mdat(track);

	return stu_mp4_merge(dst, moof, mdat, NULL);
}

static stu_int32_t
stu_fmp4_get_audio_segment(stu_buf_t *dst, stu_mp4_track_t *track) {
	stu_av_codec_context_t *ctx;
	stu_mp4_box_t          *moof, *mdat;
	stu_uint32_t            delta;

	ctx = track->ctx;
	ctx->cts = 0;
	ctx->dts -= ctx->timebase;

	delta = ctx->dts - ctx->expected_dts;
	ctx->dts = ctx->expected_dts;
	ctx->pts = ctx->cts + ctx->dts;
	ctx->duration = ctx->ref_sample_duration + delta;
	ctx->expected_dts = ctx->dts + ctx->duration;

	ctx->flags.is_leading = 0;
	ctx->flags.sample_depends_on = ctx->keyframe ? 2 : 1;
	ctx->flags.sample_is_depended_on = ctx->keyframe ? 1 : 0;
	ctx->flags.sample_has_redundancy = 0;
	ctx->flags.is_non_sync = 0;

	track->sequence_number++;

	moof = stu_mp4_moof(track);
	mdat = stu_mp4_mdat(track);

	return stu_mp4_merge(dst, moof, mdat, NULL);
}

stu_int32_t
stu_rtmp_phase_fmp4_close(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_mp4_t                *mp4;
	stu_int32_t               rc;

	nc = &r->connection;

	mp4 = stu_rtmp_phase_get_fmp4(r);
	if (mp4 == NULL) {
		stu_log_error(0, "Failed to get fmp4: fd=%d.", nc->conn->fd);
		return STU_ERROR;
	}

	rc = stu_mp4_close(mp4);
	if (rc == STU_ERROR) {
		stu_log_error(0, "Failed to close fmp4: fd=%d.", nc->conn->fd);
	}

	return rc;
}


static stu_mp4_t *
stu_rtmp_phase_get_fmp4(stu_rtmp_request_t *r) {
	stu_rtmp_netconnection_t *nc;
	stu_rtmp_netstream_t     *ns;
	stu_mp4_t                *mp4;
	u_char                   *pos;
	u_char                    tmp[STU_MAX_PATH];
	stu_uint32_t              hk;
	size_t                    len;

	nc = &r->connection;
	pos = tmp;
	stu_memzero(tmp, STU_MAX_PATH);

	ns = stu_rtmp_find_netstream(r, r->chunk_in->stream_id);
	if (ns == NULL) {
		return NULL;
	}

	pos = stu_strncpy(pos, stu_rtmp_root.data, stu_rtmp_root.len);
	*pos++ = '/';

	pos = stu_strncpy(pos, nc->url.application.data, nc->url.application.len);
	*pos++ = '/';

	pos = stu_strncpy(pos, nc->url.instance.data, nc->url.instance.len);
	*pos++ = '/';

	pos = stu_sprintf(pos, "%s.mp4", ns->name.data);

	len = pos - tmp;
	hk = stu_hash_key(tmp, len, stu_rtmp_phase_fmp4_hash.flags);

	mp4 = stu_hash_find(&stu_rtmp_phase_fmp4_hash, hk, tmp, len);
	if (mp4 == NULL) {
		mp4 = stu_calloc(sizeof(stu_mp4_t));
		if (mp4 == NULL) {
			stu_log_error(0, "Failed to calloc fmp4: %s.", tmp);
			return NULL;
		}

		mp4->framerate.num = 23976;
		mp4->framerate.den = 1000;

		mp4->file.name.data = stu_calloc(len + 1);
		if (mp4->file.name.data == NULL) {
			stu_log_error(0, "Failed to calloc fmp4 file name: %s.", tmp);
			return NULL;
		}

		stu_strncpy(mp4->file.name.data, tmp, len);
		mp4->file.name.len = len;

		if (stu_mp4_open(mp4, STU_FILE_RDWR, STU_FILE_TRUNCATE, STU_FILE_DEFAULT_ACCESS) == STU_ERROR) {
			stu_log_error(0, "Failed to " stu_file_open_n " fmp4 file \"%s\".", mp4->file.name.data);
			return NULL;
		}

		if (stu_hash_insert(&stu_rtmp_phase_fmp4_hash, &mp4->file.name, mp4) == STU_ERROR) {
			stu_log_error(0, "Failed to insert fmp4: %s.", tmp);
			return NULL;
		}
	}

	return mp4;
}
