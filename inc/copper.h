/**
 * A controller to meet performance targets by manipulating power caps.
 *
 * @author Connor Imes
 * @date 2016-05-19
 */
#ifndef _COPPER_H_
#define _COPPER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <inttypes.h>
#include <stdio.h>

// Represents state of kalman filter
typedef struct copper_filter_state {
  double x_hat_minus;
  double x_hat;
  double p_minus;
  double h;
  double k;
  double p;
  // constants
  double q;
  double r;
} copper_filter_state;

// Represents the controller, including old xup and error values
typedef struct copper_xup_state {
  double u;
  double uo;
  double uoo;
  double e;
  double eo;
  // constants
  double p1;
  double p2;
  double z1;
  double mu;
  double epc;
  double gl;
} copper_xup_state;

// Stores user-defined parameters
typedef struct copper_context {
  double constraint_target;
  double cost_min;
  double cost_max;
} copper_context;

// Log file fields
typedef struct copper_log_buffer {
  uint64_t id;
  uint64_t user_tag;
  double constraint_achieved;
  // Kalman filter values
  copper_filter_state fs;
  double workload;
  // controller xup and error
  double u;
  double e;
  double cu;
} copper_log_buffer;

// Maintains logging config and state
typedef struct copper_log_state {
  uint64_t id;
  uint32_t lb_length;
  copper_log_buffer* lb;
  FILE* lf;
} copper_log_state;

// The top-level context/state struct
typedef struct copper {
  copper_context ctx;
  copper_filter_state fs;
  copper_xup_state xs;
  copper_log_state ls;
} copper;

/**
 * Initialize a copper struct.
 * Constraints: performance_target > 0 and 0 < power_min <= power_start <= power_max
 *
 * @param cop
 *  an uninitialized copper struct, not NULL
 * @param performance_target
 *  the performance goal
 * @param power_min
 *  the minimum allowed power cap
 * @param power_max
 *  the maximum allowed power cap
 * @param power_start
 *  the starting power cap
 * @return 0 on success, -EINVAL if cop is NULL or params out of range (errno will be set)
 */
int copper_init(copper* cop, double performance_target, double power_min, double power_max, double power_start);

/**
 * Cleanup - flushes any cached log data.
 * User is responsible for managing all logging memory and open files.
 *
 * @param cop
 *  an initialized copper struct, not NULL
 */
void copper_destroy(copper* cop);

/**
 * Get the new power cap to apply.
 *
 * @param cop
 *  an initialized copper struct, not NULL
 * @param tag
 *  a user-specified identifier for this iteration (used only for logging)
 * @param performance
 *  the measured performance >= 0
 * @return the new power cap on success, -EINVAL if cop is NULL or performance < 0 (errno will be set)
 */
double copper_adapt(copper* cop, uint64_t tag, double performance);

/**
 * Enable/disable logging.
 * The user is responsible for managing the log buffer memory and file.
 *
 * @param cop
 *  an initialized copper struct, not NULL
 * @param lb
 *  the log buffer pointer (NULL to disable)
 * @param lb_length
 *  the log buffer length (0 to disable)
 * @param lf
 *  the log file to use (NULL to disable); requires non-NULL lb, lb_length > 0
 * @return 0 on success, -EINVAL if cop is NULL (errno will be set)
 */
int copper_set_logging(copper* cop, copper_log_buffer* lb, uint32_t lb_length, FILE* lf);

/**
 * Change the performance target.
 *
 * @param cop
 *  an initialized copper struct, not NULL
 * @param target
 *  the performance goal > 0
 * @return 0 on success, -EINVAL if cop is NULL or target <= 0 (errno will be set)
 */
int copper_set_performance_target(copper* cop, double target);

/**
 * Change the gain limit.
 *
 * @param cop
 *  an initialized copper struct, not NULL
 * @param gain
 *  0 <= gain < 1
 * @return 0 on success, -EINVAL if cop is NULL or gain is out of range (errno will be set)
 */
int copper_set_gain_limit(copper* cop, double gain);

#ifdef __cplusplus
}
#endif

#endif
