/**
 * CoPPer utility functions.
 *
 * @author Connor Imes
 * @date 2016-05-24
 */
#ifndef _COPPER_UTIL_H_
#define _COPPER_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include "copper.h"

/**
 * Handle resource allocations and initialize.
 * Do not call copper_init separately.
 * Constraints: performance_target > 0
 *              0 < power_min <= power_start <= power_max
 *
 * @param performance_target
 *  the performance goal
 * @param power_min
 *  the minimum allowed power cap
 * @param power_max
 *  the maximum allowed power cap
 * @param power_start
 *  the starting power cap
 * @param lb_length
 *  the log buffer length (0 to disable)
 * @param log_filename
 *  the log file name (NULL to disable); requires lb_length > 0
 * @return initialized copper instance, NULL on failure (errno will be set)
 */
copper* copper_alloc_init(double performance_target, double power_min, double power_max, double power_start,
                          uint32_t lb_length, const char* log_filename);

/**
 * Destroy and free resources allocated by copper_alloc_init.
 * Only call if copper_alloc_init was used - do not call copper_destroy separately.
 *
 * @param cop
 *  an initialized copper struct, not NULL
 * @return 0 on success, negative error code on failure (errno will be set)
 */
int copper_destroy_free(copper* cop);

#ifdef __cplusplus
}
#endif

#endif
