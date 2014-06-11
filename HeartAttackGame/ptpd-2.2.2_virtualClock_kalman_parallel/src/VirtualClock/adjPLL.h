//
//  adjPLL.h
//  
//
//  Created by 李勁璋 on 2014/1/27.
//
//

#ifndef _adjPLL_h
#define _adjPLL_h

#include "virservo.h"

/**
 * When set to a non-zero value, this variable determines the
 * proportional constant for the PI controller.
 */
extern double configured_adjPll_kp;

/**
 * When set to a non-zero value, this variable determines the
 * integral constant for the PI controller.
 */
extern double configured_adjPll_ki;

/**
 * When set to a non-zero value, this variable determines the scale in the
 * formula used to set the proportional constant of the PI controller from the
 * sync interval.
 * kp = min(kp_scale * sync^kp_exponent, kp_norm_max / sync)
 */
extern double configured_adjPll_kp_scale;

/**
 * This variable determines the exponent in the formula used to set the
 * proportional constant of the PI controller from the sync interval.
 * kp = min(kp_scale * sync^kp_exponent, kp_norm_max / sync)
 */
extern double configured_adjPll_kp_exponent;

/**
 * This variable determines the normalized maximum in the formula used to set
 * the proportional constant of the PI controller from the sync interval.
 * kp = min(kp_scale * sync^kp_exponent, kp_norm_max / sync)
 */
extern double configured_adjPll_kp_norm_max;

/**
 * When set to a non-zero value, this variable determines the scale in the
 * formula used to set the integral constant of the PI controller from the
 * sync interval.
 * ki = min(ki_scale * sync^ki_exponent, ki_norm_max / sync)
 */
extern double configured_adjPll_ki_scale;

/**
 * This variable determines the exponent in the formula used to set the
 * integral constant of the PI controller from the sync interval.
 * ki = min(ki_scale * sync^ki_exponent, ki_norm_max / sync)
 */
extern double configured_adjPll_ki_exponent;

/**
 * This variable determines the normalized maximum in the formula used to set
 * the integral constant of the PI controller from the sync interval.
 * ki = min(ki_scale * sync^ki_exponent, ki_norm_max / sync)
 */
extern double configured_adjPll_ki_norm_max;

/**
 * When set to a non-zero value, this variable controls the maximum allowed
 * offset before a clock jump occurs instead of the default clock-slewing
 * mechanism.
 *
 * Note that this variable is measured in seconds, and allows fractional values.
 */
extern double configured_adjPll_offset;

/**
 * When set to zero, the clock is not stepped on start. When set to a non-zero
 * value, the value bahaves as a threshold and the clock is stepped on start if
 * the offset is bigger than the threshold.
 *
 * Note that this variable is measured in seconds, and allows fractional values.
 */
extern double configured_adjPll_f_offset;

/**
 * When set to a non-zero value, this variable sets an additional limit for
 * the frequency adjustment of the clock. It's in ppb.
 */
extern int configured_adjPll_max_freq;

extern double configured_adjPll_frequencyWeight_exponent;
extern double configured_adjPll_phaseWeight_exponent;
extern double configured_adjPll_complianceWeight_exponent;
extern double configured_adjPll_compliancemax_exponent;
extern double configured_adjPll_compliancemultiplier_exponent;

struct servo *adjPll_servo_create(int fadj, int max_ppb, int sw_ts);


#endif
