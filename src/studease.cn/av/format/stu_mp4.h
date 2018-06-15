/*
 * stu_mp4.h
 *
 *  Created on: 2018年5月29日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_FORMAT_STU_MP4_H_
#define STUDEASE_CN_FORMAT_STU_MP4_H_

#include "stu_format.h"

#define STU_MP4_MAX_TRACK_COUNT  8

typedef struct {
	stu_queue_t        items;
	stu_queue_t        queue;

	stu_uint32_t       size;
	u_char            *type;
	u_char            *data;
	stu_uint32_t       data_size;
} stu_mp4_box_t;

typedef struct {
	stu_int32_t        id;
	stu_int32_t        sequence_number;

	stu_av_type_e      type;
	stu_av_codec_id_e  codec_id;
	void              *ctx;
} stu_mp4_track_t;

typedef struct {
	stu_file_t         file;
	stu_mp4_track_t   *tracks[STU_MP4_MAX_TRACK_COUNT];
	stu_int32_t        track_id;

	stu_uint32_t       timestamp;
	stu_int32_t        width, height;
	stu_rational_t     framerate;
	stu_double_t       duration;
	stu_int64_t        filesize;

	stu_buf_t          buffer;
} stu_mp4_t;

stu_mp4_track_t *stu_mp4_create_track(stu_mp4_t *mp4, stu_av_type_e type, stu_av_codec_id_e codec_id);
stu_mp4_track_t *stu_mp4_get_track(stu_mp4_t *mp4, stu_av_type_e type);

stu_int32_t      stu_mp4_open(stu_mp4_t *mp4, u_long mode, u_long create, u_long access);
stu_mp4_box_t   *stu_mp4_box(const char *type, u_char *data, size_t size, ...);
void             stu_mp4_clear(stu_mp4_box_t *box);
stu_mp4_box_t   *stu_mp4_ftyp(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_moov(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_mvhd(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_trak(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_tkhd(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_mdia(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_mdhd(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_hdlr(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_minf(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_dinf(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_stbl(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_stsd(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_mp4a(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_esds(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_avc1(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_mvex(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_trex(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_moof(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_mfhd(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_traf(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_sdtp(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_trun(stu_mp4_track_t *track);
stu_mp4_box_t   *stu_mp4_mdat(stu_mp4_track_t *track);
stu_int32_t      stu_mp4_merge(stu_buf_t *dst, ...);
stu_int32_t      stu_mp4_write(stu_buf_t *dst, stu_mp4_box_t *box);
stu_int32_t      stu_mp4_flush(stu_mp4_t *mp4);
stu_int32_t      stu_mp4_close(stu_mp4_t *mp4);

#endif /* STUDEASE_CN_FORMAT_STU_MP4_H_ */
