/*
 * stu_mp4.c
 *
 *  Created on: 2018年5月29日
 *      Author: Tony Lau
 */

#include "stu_format.h"

const char *STU_MP4_TYPE_AVC1 = "avc1";
const char *STU_MP4_TYPE_AVCC = "avcC";
const char *STU_MP4_TYPE_BTRT = "btrt";
const char *STU_MP4_TYPE_DINF = "dinf";
const char *STU_MP4_TYPE_DREF = "dref";
const char *STU_MP4_TYPE_ESDS = "esds";
const char *STU_MP4_TYPE_FTYP = "ftyp";
const char *STU_MP4_TYPE_HDLR = "hdlr";
const char *STU_MP4_TYPE_MDAT = "mdat";
const char *STU_MP4_TYPE_MDHD = "mdhd";
const char *STU_MP4_TYPE_MDIA = "mdia";
const char *STU_MP4_TYPE_MFHD = "mfhd";
const char *STU_MP4_TYPE_MINF = "minf";
const char *STU_MP4_TYPE_MOOF = "moof";
const char *STU_MP4_TYPE_MOOV = "moov";
const char *STU_MP4_TYPE_MP4A = "mp4a";
const char *STU_MP4_TYPE_MVEX = "mvex";
const char *STU_MP4_TYPE_MVHD = "mvhd";
const char *STU_MP4_TYPE_SDTP = "sdtp";
const char *STU_MP4_TYPE_STBL = "stbl";
const char *STU_MP4_TYPE_STCO = "stco";
const char *STU_MP4_TYPE_STSC = "stsc";
const char *STU_MP4_TYPE_STSD = "stsd";
const char *STU_MP4_TYPE_STSZ = "stsz";
const char *STU_MP4_TYPE_STTS = "stts";
const char *STU_MP4_TYPE_TFDT = "tfdt";
const char *STU_MP4_TYPE_TFHD = "tfhd";
const char *STU_MP4_TYPE_TRAF = "traf";
const char *STU_MP4_TYPE_TRAK = "trak";
const char *STU_MP4_TYPE_TRUN = "trun";
const char *STU_MP4_TYPE_TREX = "trex";
const char *STU_MP4_TYPE_TKHD = "tkhd";
const char *STU_MP4_TYPE_VMHD = "vmhd";
const char *STU_MP4_TYPE_SMHD = "smhd";

u_char  STU_MP4_DATA_AVC1[] = {
	0x00, 0x00, 0x00, 0x00, // reserved(4)
	0x00, 0x00, 0x00, 0x01, // reserved(2) + data_reference_index(2)
	0x00, 0x00, 0x00, 0x00, // pre_defined(2) + reserved(2)
	0x00, 0x00, 0x00, 0x00, // pre_defined: 3 * 4 bytes
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,             // width: 2 bytes
	0x00, 0x00,             // height: 2 bytes
	0x00, 0x48, 0x00, 0x00, // horizresolution: 4 bytes
	0x00, 0x48, 0x00, 0x00, // vertresolution: 4 bytes
	0x00, 0x00, 0x00, 0x00, // reserved: 4 bytes
	0x00, 0x01,             // frame_count
	0x0A,                   // strlen
	0x6B, 0x69, 0x77, 0x69, // compressorname: 32 bytes
	0x73, 0x74, 0x72, 0x6D,
	0x64, 0x2F, 0x74, 0x6F,
	0x6E, 0x79, 0x20, 0x6C,
	0x61, 0x75, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x18,                   // depth
	0xFF, 0xFF              // pre_defined = -1
};

u_char  STU_MP4_DATA_DREF[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) + flags
	0x00, 0x00, 0x00, 0x01, // entry_count
	0x00, 0x00, 0x00, 0x0C, // entry_size
	0x75, 0x72, 0x6C, 0x20, // type 'url '
	0x00, 0x00, 0x00, 0x01  // version(0) + flags
};

u_char  STU_MP4_DATA_ESDS[] = {
	0x00, 0x00, 0x00, 0x00, // version 0 + flags
	0x03,                   // descriptor_type
	0x00,                   // length3
	0x00, 0x01,             // es_id
	0x00,                   // stream_priority
	0x04,                   // descriptor_type
	0x00,                   // length
	0x40,                   // codec: mpeg4_audio
	0x15,                   // stream_type: Audio
	0x00, 0x00, 0x00,       // buffer_size
	0x00, 0x00, 0x00, 0x00, // maxBitrate
	0x00, 0x00, 0x00, 0x00, // avgBitrate
	0x05                    // descriptor_type
};

u_char  STU_MP4_DATA_FTYP[] = {
	0x69, 0x73, 0x6F, 0x6D, // major_brand: isom
	0x0,  0x0,  0x0,  0x1,  // minor_version: 0x01
	0x69, 0x73, 0x6F, 0x6D, // isom
	0x61, 0x76, 0x63, 0x31  // avc1
};

u_char  STU_MP4_DATA_HDLR_VIDEO[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) + flags
	0x00, 0x00, 0x00, 0x00, // pre_defined
	0x76, 0x69, 0x64, 0x65, // handler_type: 'vide'
	0x00, 0x00, 0x00, 0x00, // reserved: 3 * 4 bytes
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x56, 0x69, 0x64, 0x65,
	0x6F, 0x48, 0x61, 0x6E,
	0x64, 0x6C, 0x65, 0x72, 0x00 // name: VideoHandler
};

u_char  STU_MP4_DATA_HDLR_AUDIO[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) + flags
	0x00, 0x00, 0x00, 0x00, // pre_defined
	0x73, 0x6F, 0x75, 0x6E, // handler_type: 'soun'
	0x00, 0x00, 0x00, 0x00, // reserved: 3 * 4 bytes
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x53, 0x6F, 0x75, 0x6E,
	0x64, 0x48, 0x61, 0x6E,
	0x64, 0x6C, 0x65, 0x72, 0x00 // name: SoundHandler
};

u_char  STU_MP4_DATA_MDHD[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) + flags
	0x00, 0x00, 0x00, 0x00, // creation_time
	0x00, 0x00, 0x00, 0x00, // modification_time
	0x00, 0x00, 0x00, 0x00, // timescale: 4 bytes
	0x00, 0x00, 0x00, 0x00, // duration: 4 bytes
	0x55, 0xC4,             // language: und (undetermined)
	0x00, 0x00              // pre_defined = 0
};

u_char  STU_MP4_DATA_MFHD[] = {
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, // sequence_number: 4 bytes
};

u_char  STU_MP4_DATA_MP4A[] = {
	0x00, 0x00, 0x00, 0x00, // reserved(4)
	0x00, 0x00, 0x00, 0x01, // reserved(2) + data_reference_index(2)
	0x00, 0x00, 0x00, 0x00, // reserved: 2 * 4 bytes
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00,             // channel_configuration(2)
	0x00, 0x10,             // sample_size(2)
	0x00, 0x00, 0x00, 0x00, // reserved(4)
	0x00, 0x00,             // audio sample rate
	0x00, 0x00
};

u_char  STU_MP4_DATA_MVHD[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) + flags
	0x00, 0x00, 0x00, 0x00, // creation_time
	0x00, 0x00, 0x00, 0x00, // modification_time
	0x00, 0x00, 0x00, 0x00, // timescale: 4 bytes
	0x00, 0x00, 0x00, 0x00, // duration: 4 bytes
	0x00, 0x01, 0x00, 0x00, // Preferred rate: 1.0
	0x01, 0x00, 0x00, 0x00, // PreferredVolume(1.0, 2bytes) + reserved(2bytes)
	0x00, 0x00, 0x00, 0x00, // reserved: 4 + 4 bytes
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00, // ----begin composition matrix----
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x00, // ----end composition matrix----
	0x00, 0x00, 0x00, 0x00, // ----begin pre_defined 6 * 4 bytes----
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, // ----end pre_defined 6 * 4 bytes----
	0xFF, 0xFF, 0xFF, 0xFF  // next_track_ID
};

u_char  STU_MP4_DATA_SDTP[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) + flags
	0x00                    // is_leading            (2 bits)
	                        // sample_depends_on     (2 bits)
	                        // sample_is_depended_on (2 bits)
	                        // sample_has_redundancy (2 bits)
};

u_char  STU_MP4_DATA_STSD_PREFIX[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) + flags
	0x00, 0x00, 0x00, 0x01  // entry_count
};

u_char  STU_MP4_DATA_STTS[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) + flags
	0x00, 0x00, 0x00, 0x00  // entry_count
};

u_char *STU_MP4_DATA_STSC = STU_MP4_DATA_STTS;
u_char *STU_MP4_DATA_STCO = STU_MP4_DATA_STTS;

u_char  STU_MP4_DATA_STSZ[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) + flags
	0x00, 0x00, 0x00, 0x00, // sample_size
	0x00, 0x00, 0x00, 0x00  // sample_count
};

u_char  STU_MP4_DATA_TFDT[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) & flags
	0x00, 0x00, 0x00, 0x00, // baseMediaDecodeTime: 4 bytes
};

u_char  STU_MP4_DATA_TFHD[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) & flags
	0x00, 0x00, 0x00, 0x00, // track_ID
};

u_char  STU_MP4_DATA_TKHD[] = {
	0x00, 0x00, 0x00, 0x07, // version(0) + flags
	0x00, 0x00, 0x00, 0x00, // creation_time
	0x00, 0x00, 0x00, 0x00, // modification_time
	0x00, 0x00, 0x00, 0x00, // track_ID: 4 bytes
	0x00, 0x00, 0x00, 0x00, // reserved: 4 bytes
	0x00, 0x00, 0x00, 0x00, // duration: 4 bytes
	0x00, 0x00, 0x00, 0x00, // reserved: 2 * 4 bytes
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, // layer(2bytes) + alternate_group(2bytes)
	0x00, 0x00, 0x00, 0x00, // volume(2bytes) + reserved(2bytes)
	0x00, 0x01, 0x00, 0x00, // ----begin composition matrix----
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00,
	0x40, 0x00, 0x00, 0x00, // ----end composition matrix----
	0x00, 0x00,             // width
	0x00, 0x00,
	0x00, 0x00,             // height
	0x00, 0x00
};

u_char  STU_MP4_DATA_TREX[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) + flags
	0x00, 0x00, 0x00, 0x00, // track_ID
	0x00, 0x00, 0x00, 0x01, // default_sample_description_index
	0x00, 0x00, 0x00, 0x00, // default_sample_duration
	0x00, 0x00, 0x00, 0x00, // default_sample_size
	0x00, 0x01, 0x00, 0x01  // default_sample_flags
};

u_char  STU_MP4_DATA_TRUN[] = {
	0x00, 0x00, 0x0F, 0x01, // version(0) & flags
	0x00, 0x00, 0x00, 0x01, // sample_count
	0x00, 0x00, 0x00, 0x79, // data_offset

	0x00, 0x00, 0x00, 0x00, // sample_duration
	0x00, 0x00, 0x00, 0x00, // sample_size
	0x00, 0x00,             // sample_flags
	0x00, 0x00,             // sample_degradation_priority
	0x00, 0x00, 0x00, 0x00  // sample_composition_time_offset
};

// video media header
u_char  STU_MP4_DATA_VMHD[] = {
	0x00, 0x00, 0x00, 0x01, // version(0) + flags
	0x00, 0x00,             // graphicsmode: 2 bytes
	0x00, 0x00, 0x00, 0x00, // opcolor: 3 * 2 bytes
	0x00, 0x00
};

// Sound media header
u_char  STU_MP4_DATA_SMHD[] = {
	0x00, 0x00, 0x00, 0x00, // version(0) + flags
	0x00, 0x00, 0x00, 0x00  // balance(2) + reserved(2)
};


stu_mp4_track_t *
stu_mp4_create_track(stu_mp4_t *mp4, stu_av_type_e type, stu_av_codec_id_e codec_id) {
	stu_mp4_track_t        *track;
	stu_av_codec_context_t *ctx;
	stu_av_parse_pt         parse_pt;

	ctx = NULL;
	parse_pt = NULL;

	if (mp4->track_id >= STU_MP4_MAX_TRACK_COUNT) {
		stu_log_error(0, "Too many mp4 tracks.");
		return NULL;
	}

	track = stu_calloc(sizeof(stu_mp4_track_t));
	if (track == NULL) {
		stu_log_error(0, "Failed to calloc mp4 track.");
		return NULL;
	}

	switch (codec_id) {
	case AV_CODEC_ID_H264:
		ctx = stu_calloc(sizeof(stu_avc_context_t));
		parse_pt = (stu_av_parse_pt) stu_avc_parse_packet;
		break;

	case AV_CODEC_ID_AAC:
		ctx = stu_calloc(sizeof(stu_aac_context_t));
		parse_pt = (stu_av_parse_pt) stu_aac_parse_packet;
		break;

	default:
		stu_log_error(0, "Unknown codec: type=%d, id=%d.", type, codec_id);
		break;
	}

	if (ctx == NULL) {
		stu_free(track);
		return NULL;
	}

	ctx->parse_pt = parse_pt;
	ctx->timescale = 1000;
	ctx->duration = mp4->duration * 1000;

	if (type == STU_AV_TYPE_VIDEO) {
		ctx->framerate = mp4->framerate;
	}

	track->id = mp4->track_id + 1;
	track->type = type;
	track->codec_id = codec_id;
	track->ctx = ctx;

	mp4->tracks[mp4->track_id++] = track;

	return track;
}

stu_mp4_track_t *
stu_mp4_get_track(stu_mp4_t *mp4, stu_av_type_e type) {
	stu_mp4_track_t *track;
	stu_int32_t      i;

	for (i = 0; i < STU_MP4_MAX_TRACK_COUNT; i++) {
		track = mp4->tracks[i];
		if (track->type == type) {
			return track;
		}
	}

	return NULL;
}


stu_int32_t
stu_mp4_open(stu_mp4_t *mp4, u_long mode, u_long create, u_long access) {
	stu_file_t *file;

	file = &mp4->file;

	if (stu_create_path(file) == STU_ERROR) {
		stu_log_error(0, "Failed to create mp4 path.");
		return STU_ERROR;
	}

	file->fd = stu_file_open(file->name.data, mode, create, access);
	if (file->fd == STU_FILE_INVALID) {
		stu_log_error(stu_errno, "Failed to " stu_file_open_n " mp4 file \"%s\".", file->name.data);
		return STU_ERROR;
	}

	return STU_OK;
}

/*
 * Arguments should end of NULL.
 */
stu_mp4_box_t *
stu_mp4_box(const char *type, u_char *data, size_t size, ...) {
	stu_mp4_box_t *box, *sub;
	va_list        args;

	box = stu_calloc(sizeof(stu_mp4_box_t));
	if (box == NULL) {
		stu_log_error(stu_errno, "Failed to calloc mp4 box.");
		return NULL;
	}

	stu_queue_init(&box->items);
	stu_queue_init(&box->queue);
	box->type = type;
	box->data = data;
	box->data_size = box->size = size;

	va_start(args, size);
	for (sub = va_arg(args, stu_mp4_box_t *); sub; sub = va_arg(args, stu_mp4_box_t *)) {
		box->size += 8 + sub->size;
		stu_queue_insert_tail(&box->items, &sub->queue);
	}
	va_end(args);

	return box;
}

void
stu_mp4_clear(stu_mp4_box_t *box) {
	stu_mp4_box_t *sub;
	stu_queue_t   *q;

	if (box->data) {
		stu_free(box->data);
	}

	for (q = stu_queue_head(&box->items); q != stu_queue_sentinel(&box->items); /* void */) {
		sub = stu_queue_data(q, stu_mp4_box_t, queue);
		q = stu_queue_next(q);
		stu_mp4_clear(sub);
	}

	stu_free(box);
}

stu_mp4_box_t *
stu_mp4_ftyp(stu_mp4_track_t *track) {
	return stu_mp4_box(STU_MP4_TYPE_FTYP, STU_MP4_DATA_FTYP, sizeof(STU_MP4_DATA_FTYP), NULL);
}

stu_mp4_box_t *
stu_mp4_moov(stu_mp4_track_t *track) {
	stu_mp4_box_t *mvhd, *trak, *mvex;

	mvhd = stu_mp4_mvhd(track);
	trak = stu_mp4_trak(track);
	mvex = stu_mp4_mvex(track);

	return stu_mp4_box(STU_MP4_TYPE_MOOV, NULL, 0, mvhd, trak, mvex, NULL);
}

stu_mp4_box_t *
stu_mp4_mvhd(stu_mp4_track_t *track) {
	stu_av_codec_context_t *ctx;
	u_char                 *data;
	stu_uint32_t            size;

	ctx = track->ctx;
	size = sizeof(STU_MP4_DATA_MVHD);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_MVHD);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_MVHD, size);
	*(stu_uint32_t *) (data + 12) = stu_endian_32(ctx->timescale);
	*(stu_uint32_t *) (data + 16) = stu_endian_32(ctx->duration);

	return stu_mp4_box(STU_MP4_TYPE_MVHD, data, size, NULL);
}

stu_mp4_box_t *
stu_mp4_trak(stu_mp4_track_t *track) {
	stu_mp4_box_t *tkhd, *mdia;

	tkhd = stu_mp4_tkhd(track);
	mdia = stu_mp4_mdia(track);

	return stu_mp4_box(STU_MP4_TYPE_TRAK, NULL, 0, tkhd, mdia, NULL);
}

stu_mp4_box_t *
stu_mp4_tkhd(stu_mp4_track_t *track) {
	stu_av_codec_context_t *ctx;
	u_char                 *data;
	stu_uint32_t            size;

	ctx = track->ctx;
	size = sizeof(STU_MP4_DATA_TKHD);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_TKHD);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_TKHD, size);
	*(stu_uint32_t *) (data + 12) = stu_endian_32(track->id);
	*(stu_uint32_t *) (data + 20) = stu_endian_32(ctx->duration);
	*(stu_uint16_t *) (data + 76) = stu_endian_16(ctx->width);
	*(stu_uint16_t *) (data + 80) = stu_endian_16(ctx->height);

	return stu_mp4_box(STU_MP4_TYPE_TKHD, data, size, NULL);
}

stu_mp4_box_t *
stu_mp4_mdia(stu_mp4_track_t *track) {
	stu_mp4_box_t *mdhd, *hdlr, *minf;

	mdhd = stu_mp4_mdhd(track);
	hdlr = stu_mp4_hdlr(track);
	minf = stu_mp4_minf(track);

	return stu_mp4_box(STU_MP4_TYPE_MDIA, NULL, 0, mdhd, hdlr, minf, NULL);
}

stu_mp4_box_t *
stu_mp4_mdhd(stu_mp4_track_t *track) {
	stu_av_codec_context_t *ctx;
	u_char                 *data;
	stu_uint32_t            size;

	ctx = track->ctx;
	size = sizeof(STU_MP4_DATA_MDHD);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_MDHD);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_MDHD, size);
	*(stu_uint32_t *) (data + 12) = stu_endian_32(ctx->timescale);
	*(stu_uint32_t *) (data + 16) = stu_endian_32(ctx->duration);

	return stu_mp4_box(STU_MP4_TYPE_MDHD, data, size, NULL);
}

stu_mp4_box_t *
stu_mp4_hdlr(stu_mp4_track_t *track) {
	u_char       *data;
	stu_uint32_t  size;

	if (track->type == STU_AV_TYPE_AUDIO) {
		size = sizeof(STU_MP4_DATA_HDLR_AUDIO);
	} else {
		size = sizeof(STU_MP4_DATA_HDLR_VIDEO);
	}

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_HDLR);
		return NULL;
	}

	if (track->type == STU_AV_TYPE_AUDIO) {
		memcpy(data, STU_MP4_DATA_HDLR_AUDIO, size);
	} else {
		memcpy(data, STU_MP4_DATA_HDLR_VIDEO, size);
	}

	return stu_mp4_box(STU_MP4_TYPE_HDLR, data, size, NULL);
}

stu_mp4_box_t *
stu_mp4_minf(stu_mp4_track_t *track) {
	stu_mp4_box_t *xmhd, *dinf, *stbl;
	u_char        *data;
	stu_uint32_t   size;

	if (track->type == STU_AV_TYPE_AUDIO) {
		size = sizeof(STU_MP4_DATA_SMHD);
	} else {
		size = sizeof(STU_MP4_DATA_VMHD);
	}

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.",
				track->type == STU_AV_TYPE_AUDIO ? STU_MP4_TYPE_SMHD : STU_MP4_TYPE_VMHD);
		return NULL;
	}

	if (track->type == STU_AV_TYPE_AUDIO) {
		memcpy(data, STU_MP4_DATA_SMHD, size);
		xmhd = stu_mp4_box(STU_MP4_TYPE_SMHD, data, size, NULL);
	} else {
		memcpy(data, STU_MP4_DATA_VMHD, size);
		xmhd = stu_mp4_box(STU_MP4_TYPE_VMHD, data, size, NULL);
	}

	dinf = stu_mp4_dinf(track);
	stbl = stu_mp4_stbl(track);

	return stu_mp4_box(STU_MP4_TYPE_MINF, NULL, 0, xmhd, dinf, stbl, NULL);
}

stu_mp4_box_t *
stu_mp4_dinf(stu_mp4_track_t *track) {
	stu_mp4_box_t *dref;
	u_char        *data;
	stu_uint32_t   size;

	size = sizeof(STU_MP4_DATA_DREF);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_DREF);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_DREF, size);
	dref = stu_mp4_box(STU_MP4_TYPE_DREF, data, size, NULL);

	return stu_mp4_box(STU_MP4_TYPE_DINF, NULL, 0, dref, NULL);
}

stu_mp4_box_t *
stu_mp4_stbl(stu_mp4_track_t *track) {
	stu_mp4_box_t *stsd, *stts, *stsc, *stsz, *stco;
	u_char        *data;
	stu_uint32_t   size;

	stsd = stu_mp4_stsd(track);

	// stts
	size = sizeof(STU_MP4_DATA_STTS);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_STTS);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_STTS, size);
	stts = stu_mp4_box(STU_MP4_TYPE_STTS, data, size, NULL);

	// stsc
	size = sizeof(STU_MP4_DATA_STSC);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_STSC);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_STSC, size);
	stsc = stu_mp4_box(STU_MP4_TYPE_STSC, data, size, NULL);

	// stsz
	size = sizeof(STU_MP4_DATA_STSZ);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_STSZ);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_STSZ, size);
	stsz = stu_mp4_box(STU_MP4_TYPE_STSZ, data, size, NULL);

	// stco
	size = sizeof(STU_MP4_DATA_STCO);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_STCO);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_STCO, size);
	stco = stu_mp4_box(STU_MP4_TYPE_STCO, data, size, NULL);

	return stu_mp4_box(STU_MP4_TYPE_STBL, NULL, 0, stsd, stts, stsc, stsz, stco, NULL);
}

stu_mp4_box_t *
stu_mp4_stsd(stu_mp4_track_t *track) {
	stu_mp4_box_t *sub;
	u_char        *data;
	stu_uint32_t   size;

	size = sizeof(STU_MP4_DATA_STSD_PREFIX);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_STSD);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_STSD_PREFIX, size);

	if (track->type == STU_AV_TYPE_AUDIO) {
		sub = stu_mp4_mp4a(track);
	} else {
		sub = stu_mp4_avc1(track);
	}

	return stu_mp4_box(STU_MP4_TYPE_STSD, data, size, sub, NULL);
}

stu_mp4_box_t *
stu_mp4_mp4a(stu_mp4_track_t *track) {
	stu_aac_context_t *ctx;
	stu_mp4_box_t     *esds;
	u_char            *data;
	stu_uint32_t       size;

	ctx = track->ctx;
	size = sizeof(STU_MP4_DATA_MP4A);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_MP4A);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_MP4A, size);
	*(stu_uint16_t *) (data + 16) = stu_endian_16(ctx->channel_configuration);
	*(stu_uint16_t *) (data + 24) = stu_endian_16(ctx->sampling_frequency);

	esds = stu_mp4_esds(track);

	return stu_mp4_box(STU_MP4_TYPE_MP4A, data, size, esds, NULL);
}

stu_mp4_box_t *
stu_mp4_esds(stu_mp4_track_t *track) {
	stu_aac_context_t *ctx;
	u_char            *data, *pos;
	stu_uint32_t       size;

	ctx = track->ctx;
	size = sizeof(STU_MP4_DATA_ESDS) + 4 + ctx->conf_size;

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_ESDS);
		return NULL;
	}

	pos = stu_memcpy(data, STU_MP4_DATA_ESDS, sizeof(STU_MP4_DATA_ESDS));
	*(u_char *) (data + 5) = 0x17 + ctx->conf_size;
	*(u_char *) (data + 10) = 0x0F + ctx->conf_size;

	*pos++ = ctx->conf_size;
	pos = stu_memcpy(pos, ctx->conf, ctx->conf_size);
	pos = stu_memcpy(pos, "\x6\x1\x2", 3); // GASpecificConfig

	return stu_mp4_box(STU_MP4_TYPE_ESDS, data, size, NULL);
}

stu_mp4_box_t *
stu_mp4_avc1(stu_mp4_track_t *track) {
	stu_avc_context_t *ctx;
	stu_mp4_box_t     *avcc;
	u_char            *data;
	stu_uint32_t       size;

	ctx = track->ctx;
	size = sizeof(STU_MP4_DATA_AVC1);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_AVC1);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_AVC1, size);
	*(stu_uint16_t *) (data + 24) = stu_endian_16(ctx->avctx.coded_width);
	*(stu_uint16_t *) (data + 26) = stu_endian_16(ctx->avctx.coded_height);

	avcc = stu_mp4_box(STU_MP4_TYPE_AVCC, ctx->avcc, ctx->avcc_size, NULL);

	return stu_mp4_box(STU_MP4_TYPE_AVC1, data, size, avcc, NULL);
}

stu_mp4_box_t *
stu_mp4_mvex(stu_mp4_track_t *track) {
	stu_mp4_box_t *trex;

	trex = stu_mp4_trex(track);

	return stu_mp4_box(STU_MP4_TYPE_MVEX, NULL, 0, trex, NULL);
}

stu_mp4_box_t *
stu_mp4_trex(stu_mp4_track_t *track) {
	u_char       *data;
	stu_uint32_t  size;

	size = sizeof(STU_MP4_DATA_TREX);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_TREX);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_TREX, size);
	*(stu_uint32_t *) (data + 4) = stu_endian_32(track->id);

	return stu_mp4_box(STU_MP4_TYPE_TREX, data, size, NULL);
}

stu_mp4_box_t *
stu_mp4_moof(stu_mp4_track_t *track) {
	stu_mp4_box_t *mfhd, *traf;

	mfhd = stu_mp4_mfhd(track);
	traf = stu_mp4_traf(track);

	return stu_mp4_box(STU_MP4_TYPE_MOOF, NULL, 0, mfhd, traf, NULL);
}

stu_mp4_box_t *
stu_mp4_mfhd(stu_mp4_track_t *track) {
	u_char            *data;
	stu_uint32_t       size;

	size = sizeof(STU_MP4_DATA_MFHD);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_MFHD);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_MFHD, size);
	*(stu_uint32_t *) (data + 4) = stu_endian_32(track->sequence_number);

	return stu_mp4_box(STU_MP4_TYPE_MFHD, data, size, NULL);
}

stu_mp4_box_t *
stu_mp4_traf(stu_mp4_track_t *track) {
	stu_av_codec_context_t *ctx;
	stu_mp4_box_t          *tfhd, *tfdt, *trun, *sdtp;
	u_char                 *data;
	stu_uint32_t            size;

	ctx = track->ctx;

	// tfhd
	size = sizeof(STU_MP4_DATA_TFHD);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_TFHD);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_TFHD, size);
	*(stu_uint32_t *) (data + 4) = stu_endian_32(track->id);

	tfhd = stu_mp4_box(STU_MP4_TYPE_TFHD, data, size, NULL);

	// tfdt
	size = sizeof(STU_MP4_DATA_TFDT);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_TFDT);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_TFDT, size);
	*(stu_uint32_t *) (data + 4) = stu_endian_32(ctx->dts);

	tfdt = stu_mp4_box(STU_MP4_TYPE_TFDT, data, size, NULL);

	// trun
	trun = stu_mp4_trun(track);

	// sdtp
	sdtp = stu_mp4_sdtp(track);

	return stu_mp4_box(STU_MP4_TYPE_TRAF, NULL, 0, tfhd, tfdt, trun, sdtp, NULL);
}

stu_mp4_box_t *
stu_mp4_trun(stu_mp4_track_t *track) {
	stu_av_codec_context_t *ctx;
	u_char                 *data;
	stu_uint32_t            size;

	ctx = track->ctx;
	size = sizeof(STU_MP4_DATA_TRUN);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_TRUN);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_TRUN, size);
	*(stu_uint32_t *) (data + 12) = stu_endian_32(ctx->duration);
	*(stu_uint32_t *) (data + 16) = stu_endian_32(ctx->data_size);
	*(u_char *) (data + 20) = ctx->flags.is_leading << 2 | ctx->flags.sample_depends_on;
	*(u_char *) (data + 21) = ctx->flags.sample_is_depended_on << 6 | ctx->flags.sample_has_redundancy << 4 | ctx->flags.is_non_sync;
	*(stu_uint32_t *) (data + 24) = stu_endian_32(ctx->cts);

	return stu_mp4_box(STU_MP4_TYPE_TRUN, data, size, NULL);
}

stu_mp4_box_t *
stu_mp4_sdtp(stu_mp4_track_t *track) {
	stu_av_codec_context_t *ctx;
	u_char                 *data;
	stu_uint32_t            size;

	ctx = track->ctx;
	size = sizeof(STU_MP4_DATA_SDTP);

	data = stu_calloc(size);
	if (data == NULL) {
		stu_log_error(0, "Failed to calloc mp4 box data: type=%s.", STU_MP4_TYPE_SDTP);
		return NULL;
	}

	memcpy(data, STU_MP4_DATA_SDTP, size);
	*(u_char *) (data + 4) = ctx->flags.is_leading << 6
			| ctx->flags.sample_depends_on << 4
			| ctx->flags.sample_is_depended_on << 2
			| ctx->flags.sample_has_redundancy;

	return stu_mp4_box(STU_MP4_TYPE_SDTP, data, size, NULL);
}

stu_mp4_box_t *
stu_mp4_mdat(stu_mp4_track_t *track) {
	stu_av_codec_context_t *ctx;

	ctx = track->ctx;

	return stu_mp4_box(STU_MP4_TYPE_MDAT, ctx->data, ctx->data_size, NULL);
}


/*
 * Arguments should end of NULL.
 */
stu_int32_t
stu_mp4_merge(stu_buf_t *dst, ...) {
	stu_mp4_box_t *sub;
	size_t         size;
	va_list        args;

	size = 0;

	va_start(args, dst);
	for (sub = va_arg(args, stu_mp4_box_t *); sub; sub = va_arg(args, stu_mp4_box_t *)) {
		size += 8 + sub->size;
	}
	va_end(args);

	if (size == 0) {
		goto done;
	}

	dst->start = stu_calloc(size);
	if (dst->start == NULL) {
		stu_log_error(0, "Failed to calloc mp4 segment data.");
		return STU_ERROR;
	}

	dst->pos = dst->last = dst->start;
	dst->end = dst->start + size;
	dst->size = size;

	va_start(args, dst);
	for (sub = va_arg(args, stu_mp4_box_t *); sub; sub = va_arg(args, stu_mp4_box_t *)) {
		stu_mp4_write(dst, sub);
	}
	va_end(args);

done:

	return STU_OK;
}

stu_int32_t
stu_mp4_write(stu_buf_t *dst, stu_mp4_box_t *box) {
	stu_mp4_box_t *sub;
	stu_queue_t   *q;

	*(stu_uint32_t *) dst->last = stu_endian_32(8 + box->size);
	dst->last += 4;

	dst->last = stu_memcpy(dst->last, box->type, 4);

	if (box->data) {
		dst->last = stu_memcpy(dst->last, box->data, box->data_size);
		stu_free(box->data);
	}

	for (q = stu_queue_head(&box->items); q != stu_queue_sentinel(&box->items); /* void */) {
		sub = stu_queue_data(q, stu_mp4_box_t, queue);
		q = stu_queue_next(q);
		stu_mp4_write(dst, sub);
	}

	stu_free(box);

	return STU_OK;
}

stu_int32_t
stu_mp4_flush(stu_mp4_t *mp4) {
	stu_file_t *file;
	stu_buf_t  *buf;

	file = &mp4->file;
	buf = &mp4->buffer;

	if (buf->size == 0) {
		return STU_OK;
	}

	if (stu_file_write(file, buf->start, buf->size, file->offset) == STU_ERROR) {
		stu_log_error(0, "Failed to write mp4 buffer: file=%s.", file->name.data);
		return STU_ERROR;
	}

	stu_free(buf->start);

	buf->start = buf->end = NULL;
	buf->pos = buf->last = NULL;
	buf->size = 0;

	return STU_OK;
}

stu_int32_t
stu_mp4_close(stu_mp4_t *mp4) {
	stu_file_t *file;

	file = &mp4->file;

	return stu_file_close(file->fd);
}
