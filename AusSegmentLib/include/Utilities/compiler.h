#pragma once

#define RIGHT_EYE 0
#define LEFT_EYE 1

#define BASE_SCALE_6
//#define BASE_SCALE_10

#ifdef BASE_SCALE_6
#define ENCODE_BASE_SCALE 6
#define MATCH_FILTERSELECT 0xff
#endif

#ifdef BASE_SCALE_10
#define ENCODE_BASE_SCALE 10
#define MATCH_FILTERSELECT 0x0f
#endif

#define MATCH_MIN_COMMON_BITS 1000
#define TEMPLATE_MIN_VALID_BITS 1500

#ifdef ALGO_SEG_SCORE_TEST  // for testing score thresholds
#define EYEFIND_MIN_SCORE 6000
#define IRISFIND_SPEC_MIN_SCORE 0
#define IRISFIND_DARK_MIN_SCORE 0
#define IRISFIND_PUPIL_MIN_SCORE 0
#define IRISFIND_IRIS_MIN_SCORE 0
#else  // normal operation
#define EYEFIND_MIN_SCORE \
  6000  // note: failure does not prevent iris segmentation
#define IRISFIND_SPEC_MIN_SCORE 1000  //
#define IRISFIND_DARK_MIN_SCORE \
  3200  // dark 3200 // sony 3200 // steve 3200 // 4000 10% reject // 3400 5%
        // reject // 4000
#define IRISFIND_PUPIL_MIN_SCORE \
  500  // dark 500 // sony 2000 // steve 3200 // 4000 10% reject // 2000 5%
       // reject // 3000
#define IRISFIND_IRIS_MIN_SCORE \
  1400  // dark 1400 // sony 2800 // steve 3200 // 3200 10% reject // 2800 5%
        // reject // 3000
#endif

// determined by combination of lens + sensor
#define PIXELS_PER_CM_AT_50CM 95

// determined by avg human iris diameter in mm
#define IRIS_TYPICAL_DIAMETER_IN_MM 11.8f

// change of 2 cm distance = ~4 pixels difference in iris diameter
#define IRISFIND_MAX_IRIS_DISTANCE_DIFF_CM 2.0f

// radius of gaze = spec location - iris location
#define GAZE_RADIUS_THRESH 10.0f

#if defined(COMPILER_QT)

#include <QtGlobal>  // for Qt data types

#define ASSERT(expr)

#define int8 qint8
#define int16 qint16
#define int32 qint32
#define int64 qint64
#define uint8 quint8
#define uint16 quint16
#define uint32 quint32
#define uint64 quint64
#define float32 float

#elif defined(COMPILER_XILINX_ARM)

#include "xil_types.h"

#define ASSERT(expr)

#define int8 s8
#define int16 s16
#define int32 s32
#define int64 s64
#define uint8 u8
#define uint16 u16
#define uint32 u32
#define uint64 u64
#define float32 float
#define bool uint8

#define true 1
#define false 0

#else  // standard libraries - Microsoft or GNU

#include <stddef.h>  // for NULL
#include <stdint.h>  // for *_t types below

// FIXME #include "elutil.h" here failed to define ASSERT, find out why
#define ASSERT assert
#include <assert.h>

#define int8 int8_t
#define int16 int16_t
#define int32 int32_t
#define int64 int64_t
#define uint8 uint8_t
#define uint16 uint16_t
#define uint32 uint32_t
#define uint64 uint64_t
#define float32 float

#endif

typedef uint8 *PLINE;
typedef uint8 TEMPLATE[1280];
