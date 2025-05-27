// SPDX-License-Identifier: GPL-2.0-only
/*
 * Digital Voice Modem - Modem Firmware
 * GPLv2 Open Source. Use is subject to license terms.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 *  Copyright (C) 2018 by Adrian Musceac YO8RZZ
 *
 */
#if !defined(__ARM_MATH_H__)
#define __ARM_MATH_H__

#include <cstddef>

// ---------------------------------------------------------------------------
//  Types
// ---------------------------------------------------------------------------

#ifndef _INT8_T_DECLARED
#ifndef __INT8_TYPE__
typedef signed char         int8_t;
#endif // __INT8_TYPE__
#endif // _INT8_T_DECLARED
#ifndef _INT16_T_DECLARED
#ifndef __INT16_TYPE__
typedef short               int16_t;
#endif // __INT16_TYPE__
#endif // _INT16_T_DECLARED
#ifndef _INT32_T_DECLARED
#ifndef __INT32_TYPE__
typedef int                 int32_t;
#endif // __INT32_TYPE__
#endif // _INT32_T_DECLARED
#ifndef _INT64_T_DECLARED
#ifndef __INT64_TYPE__
typedef long long           int64_t;
#endif // __INT64_TYPE__
#endif // _INT64_T_DECLARED
#ifndef _UINT8_T_DECLARED
#ifndef __UINT8_TYPE__
typedef unsigned char       uint8_t;
#endif // __UINT8_TYPE__
#endif // _UINT8_T_DECLARED
#ifndef _UINT16_T_DECLARED
#ifndef __UINT16_TYPE__
typedef unsigned short      uint16_t;
#endif // __UINT16_TYPE__
#endif // _UINT16_T_DECLARED
#ifndef _UINT32_T_DECLARED
#ifndef __UINT32_TYPE__
typedef unsigned int        uint32_t;
#endif // __UINT32_TYPE__
#endif // _UINT32_T_DECLARED
#ifndef _UINT64_T_DECLARED
#ifndef __UINT64_TYPE__
typedef unsigned long long  uint64_t;
#endif // __UINT64_TYPE__
#endif // _UINT64_T_DECLARED

#ifndef __LONG64_TYPE__
typedef long long           long64_t;
#endif // __LONG64_TYPE__
#ifndef __ULONG64_TYPE__
typedef unsigned long long  ulong64_t;
#endif // __ULONG64_TYPE__

typedef int16_t             q15_t;
typedef int32_t             q31_t;
typedef int64_t             q63_t;

// ---------------------------------------------------------------------------
//  Structures
// ---------------------------------------------------------------------------

typedef struct {
    uint16_t numTaps;               /**< number of filter coefficients in the filter. */
    q15_t* pState;                  /**< points to the state variable array. The array is of length numTaps+blockSize-1. */
    q15_t* pCoeffs;                 /**< points to the coefficient array. The array is of length numTaps.*/
} arm_fir_instance_q15;

typedef struct {
    uint8_t L;                      /**< upsample factor. */
    uint16_t phaseLength;           /**< length of each polyphase filter component. */
    q15_t* pCoeffs;                 /**< points to the coefficient array. The array is of length L*phaseLength. */
    q15_t* pState;                  /**< points to the state variable array. The array is of length blockSize+phaseLength-1. */
} arm_fir_interpolate_instance_q15;

typedef struct {
    uint32_t numStages;             /**< number of 2nd order stages in the filter.  Overall order is 2*numStages. */
    q31_t* pState;                  /**< Points to the array of state coefficients.  The array is of length 4*numStages. */
    q31_t* pCoeffs;                 /**< Points to the array of coefficients.  The array is of length 5*numStages. */
    uint8_t postShift;              /**< Additional shift, in bits, applied to each output sample. */
} arm_biquad_casd_df1_inst_q31;

// ---------------------------------------------------------------------------
//  Macros
// ---------------------------------------------------------------------------

#define __SSAT(x, y)  ((x>32767)  ? 32767 : ((x < -32768) ? -32768 : x))

// ---------------------------------------------------------------------------
//  Global Functions
// ---------------------------------------------------------------------------

/**
 * @brief Processing function for the Q15 FIR interpolator.
 * @param S An instance of the Q15 FIR interpolator structure.
 * @param pSrc Block of input data.
 * @param pDst Block of output data.
 * @param blockSize Number of input samples to process per call.
 */
void arm_fir_interpolate_q15(const arm_fir_interpolate_instance_q15* S, q15_t* pSrc, q15_t* pDst, uint32_t blockSize);

/**
 * @brief Processing function for the fast Q15 FIR filter for Cortex-M3 and Cortex-M4.
 * @param S An instance of the Q15 FIR interpolator structure.
 * @param pSrc Block of input data.
 * @param pDst Block of output data.
 * @param blockSize Number of input samples to process per call.
 */
void arm_fir_fast_q15(const arm_fir_instance_q15* S, q15_t* pSrc, q15_t* pDst, uint32_t blockSize);

/**
 * @brief Processing function for the Q31 Biquad cascade filter
 * @param S An instance of the Q15 FIR interpolator structure.
 * @param pSrc Block of input data.
 * @param pDst Block of output data.
 * @param blockSize Number of input samples to process per call.
 */
void arm_biquad_cascade_df1_q31(const arm_biquad_casd_df1_inst_q31* S, q31_t* pSrc, q31_t* pDst, uint32_t blockSize);

/**
 * @brief Converts the elements of the Q15 vector to Q31 vector.
 * 
 * @param pSrc Input pointer.
 * @param pDst Output pointer.
 * @param blockSize Number of input samples to process.
 */
void arm_q15_to_q31(q15_t* pSrc, q31_t* pDst, uint32_t blockSize);

#endif // __ARM_MATH_H__
