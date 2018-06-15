/*
 * stu_av.h
 *
 *  Created on: 2018年5月31日
 *      Author: Tony Lau
 */

#ifndef STUDEASE_CN_AV_STU_AV_H_
#define STUDEASE_CN_AV_STU_AV_H_

#include "../stu_config.h"
#include "../core/stu_core.h"

#define STU_AV_ARRAY_LEN(a) (sizeof(a) / sizeof((a)[0]))

/*
 * Chromaticity coordinates of the source primaries.
 */
typedef enum {
	STU_AV_COL_PRI_RESERVED0   = 0,
	STU_AV_COL_PRI_BT709       = 1,  ///< also ITU-R BT1361 / IEC 61966-2-4 / SMPTE RP177 Annex B
	STU_AV_COL_PRI_UNSPECIFIED = 2,
	STU_AV_COL_PRI_RESERVED    = 3,
	STU_AV_COL_PRI_BT470M      = 4,  ///< also FCC Title 47 Code of Federal Regulations 73.682 (a)(20)

	STU_AV_COL_PRI_BT470BG     = 5,  ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM
	STU_AV_COL_PRI_SMPTE170M   = 6,  ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
	STU_AV_COL_PRI_SMPTE240M   = 7,  ///< functionally identical to above
	STU_AV_COL_PRI_FILM        = 8,  ///< colour filters using Illuminant C
	STU_AV_COL_PRI_BT2020      = 9,  ///< ITU-R BT2020
	STU_AV_COL_PRI_SMPTEST428_1= 10, ///< SMPTE ST 428-1 (CIE 1931 XYZ)
	STU_AV_COL_PRI_NB,               ///< Not part of ABI
} stu_av_col_pri_e;

/*
 * Color Transfer Characteristic.
 */
typedef enum {
	STU_AV_COL_TRC_RESERVED0    = 0,
	STU_AV_COL_TRC_BT709        = 1,  ///< also ITU-R BT1361
	STU_AV_COL_TRC_UNSPECIFIED  = 2,
	STU_AV_COL_TRC_RESERVED     = 3,
	STU_AV_COL_TRC_GAMMA22      = 4,  ///< also ITU-R BT470M / ITU-R BT1700 625 PAL & SECAM
	STU_AV_COL_TRC_GAMMA28      = 5,  ///< also ITU-R BT470BG
	STU_AV_COL_TRC_SMPTE170M    = 6,  ///< also ITU-R BT601-6 525 or 625 / ITU-R BT1358 525 or 625 / ITU-R BT1700 NTSC
	STU_AV_COL_TRC_SMPTE240M    = 7,
	STU_AV_COL_TRC_LINEAR       = 8,  ///< "Linear transfer characteristics"
	STU_AV_COL_TRC_LOG          = 9,  ///< "Logarithmic transfer characteristic (100:1 range)"
	STU_AV_COL_TRC_LOG_SQRT     = 10, ///< "Logarithmic transfer characteristic (100 * Sqrt(10) : 1 range)"
	STU_AV_COL_TRC_IEC61966_2_4 = 11, ///< IEC 61966-2-4
	STU_AV_COL_TRC_BT1361_ECG   = 12, ///< ITU-R BT1361 Extended Colour Gamut
	STU_AV_COL_TRC_IEC61966_2_1 = 13, ///< IEC 61966-2-1 (sRGB or sYCC)
	STU_AV_COL_TRC_BT2020_10    = 14, ///< ITU-R BT2020 for 10-bit system
	STU_AV_COL_TRC_BT2020_12    = 15, ///< ITU-R BT2020 for 12-bit system
	STU_AV_COL_TRC_SMPTEST2084  = 16, ///< SMPTE ST 2084 for 10-, 12-, 14- and 16-bit systems
	STU_AV_COL_TRC_SMPTEST428_1 = 17, ///< SMPTE ST 428-1
	STU_AV_COL_TRC_ARIB_STD_B67 = 18, ///< ARIB STD-B67, known as "Hybrid log-gamma"
	STU_AV_COL_TRC_NB,                ///< Not part of ABI
} stu_av_col_trc_e;

/*
 * YUV colorspace type.
 */
typedef enum {
	STU_AV_COL_SPC_RGB         = 0,  ///< order of coefficients is actually GBR, also IEC 61966-2-1 (sRGB)
	STU_AV_COL_SPC_BT709       = 1,  ///< also ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / SMPTE RP177 Annex B
	STU_AV_COL_SPC_UNSPECIFIED = 2,
	STU_AV_COL_SPC_RESERVED    = 3,
	STU_AV_COL_SPC_FCC         = 4,  ///< FCC Title 47 Code of Federal Regulations 73.682 (a)(20)
	STU_AV_COL_SPC_BT470BG     = 5,  ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
	STU_AV_COL_SPC_SMPTE170M   = 6,  ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC
	STU_AV_COL_SPC_SMPTE240M   = 7,  ///< functionally identical to above
	STU_AV_COL_SPC_YCOCG       = 8,  ///< Used by Dirac / VC-2 and H.264 FRext, see ITU-T SG16
	STU_AV_COL_SPC_BT2020_NCL  = 9,  ///< ITU-R BT2020 non-constant luminance system
	STU_AV_COL_SPC_BT2020_CL   = 10, ///< ITU-R BT2020 constant luminance system
	STU_AV_COL_SPC_NB,               ///< Not part of ABI
} stu_av_col_spc_e;

#define STU_AV_COL_SPC_YCGCO  STU_AV_COL_SPC_YCOCG


/*
 * MPEG vs JPEG YUV range.
 */
typedef enum {
	STU_AV_COL_RANGE_UNSPECIFIED = 0,
	STU_AV_COL_RANGE_MPEG        = 1, ///< the normal 219*2^(n-8) "MPEG" YUV ranges
	STU_AV_COL_RANGE_JPEG        = 2, ///< the normal     2^n-1   "JPEG" YUV ranges
	STU_AV_COL_RANGE_NB,              ///< Not part of ABI
} stu_av_col_range_e;

/*
 * Location of chroma samples.
 *
 * Illustration showing the location of the first (top left) chroma sample of the
 * image, the left shows only luma, the right
 * shows the location of the chroma sample, the 2 could be imagined to overlay
 * each other but are drawn separately due to limitations of ASCII
 *
 *                1st 2nd       1st 2nd horizontal luma sample positions
 *                 v   v         v   v
 *                 ______        ______
 *1st luma line > |X   X ...    |3 4 X ...     X are luma samples,
 *                |             |1 2           1-6 are possible chroma positions
 *2nd luma line > |X   X ...    |5 6 X ...     0 is undefined/unknown position
 */
typedef enum {
	STU_AV_CHROMA_LOC_UNSPECIFIED = 0,
	STU_AV_CHROMA_LOC_LEFT        = 1, ///< MPEG-2/4 4:2:0, H.264 default for 4:2:0
	STU_AV_CHROMA_LOC_CENTER      = 2, ///< MPEG-1 4:2:0, JPEG 4:2:0, H.263 4:2:0
	STU_AV_CHROMA_LOC_TOPLEFT     = 3, ///< ITU-R 601, SMPTE 274M 296M S314M(DV 4:1:1), mpeg2 4:2:2
	STU_AV_CHROMA_LOC_TOP         = 4,
	STU_AV_CHROMA_LOC_BOTTOMLEFT  = 5,
	STU_AV_CHROMA_LOC_BOTTOM      = 6,
	STU_AV_CHROMA_LOC_NB,              ///< Not part of ABI
} stu_av_chroma_location_e;

typedef enum {
	STU_AV_TYPE_DATA  = 0x12,
	STU_AV_TYPE_VIDEO = 0x09,
	STU_AV_TYPE_AUDIO = 0x08
} stu_av_type_e;

typedef enum {
	AV_CODEC_ID_NONE = 0,

	/* video codecs */
	AV_CODEC_ID_H264 = 0x07,

	/* audio codecs */
	AV_CODEC_ID_AAC  = 0x0A
} stu_av_codec_id_e;

static stu_inline stu_int32_t
stu_av_clip(stu_int32_t a, stu_int32_t amin, stu_int32_t amax) {
	if (amin > amax)   abort();

	if      (a < amin) return amin;
	else if (a > amax) return amax;
	else               return a;
}

extern const stu_uint8_t  stu_av_zigzag_direct[64];
extern const stu_uint8_t  stu_av_zigzag_scan[16+1];

#include "stu_bitstream.h"
#include "stu_golomb.h"
#include "stu_rational.h"
#include "codec/stu_codec.h"
#include "format/stu_format.h"

#endif /* STUDEASE_CN_AV_STU_AV_H_ */
