/**
 * A controller to meet performance targets by manipulating power caps.
 *
 * @author Connor Imes
 * @date 2016-05-19
 */
#include <assert.h>
#include <errno.h>
#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "copper.h"
#include "copper-constants.h"

/*
 * Estimates the base workload of the application, e.g. the amount of time (in seconds) between measurements.
 * Uses a Kalman Filter.
 */
static double estimate_base_workload(copper_filter_state* fs, double current_workload, double last_xup) {
  assert(fs != NULL);
  double w;
  fs->x_hat_minus = fs->x_hat;
  fs->p_minus = fs->p + fs->q;
  fs->h = last_xup;
  fs->k = (fs->p_minus * fs->h) / ((fs->h * fs->p_minus * fs->h) + fs->r);
  fs->x_hat = fs->x_hat_minus + (fs->k * (current_workload - (fs->h * fs->x_hat_minus)));
  fs->p = (1.0 - (fs->k * fs->h)) * fs->p_minus;
  w = 1.0 / fs->x_hat;
  return w;
}

/**
 * Minimum number of control steps before we can expect the controller to settle within epsilon percent of the goal.
 */
static double get_confidence_zone(double pole, double epsilon) {
  assert(pole >= 0.0 && pole < 1.0);
  assert(epsilon > 0.0 && epsilon < 1.0);
  // expect instantaneous settling if pole is 0
  return pole < DBL_EPSILON ? 0.0 : (log(epsilon) / log(pole));
}

static double clamp(double val, double min, double max) {
  return val < min ? min : (val > max ? max : val);
}

/*
 * Calculates the xup necessary to achieve the target constraint, e.g. speedup.
 */
static void calculate_xup(copper_xup_state* xs, double target, double achieved, double w, uint64_t id, double umax) {
  assert(xs != NULL);
  const double P1 = xs->p1;
  const double P2 = xs->p2;
  const double Z1 = xs->z1;
  const double MU = xs->mu;

  const double A = -(-(P1 * Z1) - (P2 * Z1) + (MU * P1 * P2) - (MU * P2) + P2 - (MU * P1) + P1 + MU);
  const double B = -(-(MU * P1 * P2 * Z1) + (P1 * P2 * Z1) + (MU * P2 * Z1) + (MU * P1 * Z1) - (MU * Z1) - (P1 * P2));
  const double C = (((MU - (MU * P1)) * P2) + (MU * P1) - MU) * w;
  const double D = ((((MU * P1) - MU) * P2) - (MU * P1) + MU) * w * Z1;
  const double F = 1.0 / (Z1 - 1.0);

  // compute error
  xs->e = target - achieved;

  // Calculate xup
  xs->u = F * ((A * xs->uo) + (B * xs->uoo) + (C * xs->e) + (D * xs->eo));
  // must clamp xup before applying gain so that large error is still handled
  xs->u = clamp(xs->u, 1.0, umax);
  if (id > ceil(get_confidence_zone(P1, xs->epc))) {
    // absolute normalized errors
    const double en = fabs(xs->e) / target;
    const double eno = fabs(xs->eo) / target;
     // absolute normalized change in errors
    const double den = fabs(eno - en);
    // if error is already low, we do not want to make big changes since the controller is already settled
    // normalized error scalar value: 0 <= 1 - 1/(x+1) < 1
    const double ens = 1.0 - (1.0 / (en + 1.0));
    // if difference in error is low, we want to make changes to maximize cost adjustment (to reduce it)
    // error delta scalar value: 0 < 1/(x+1) <= 1
    const double dens = 1.0 / (den + 1.0);
    // compute the gain
    const double gain = 1.0 - (xs->gl * dens * ens);
    // scale xup based on gain and re-clamp
    xs->u = clamp(gain * xs->u, 1.0, umax);
  }

  // Save old values
  xs->uoo = xs->uo;
  xs->uo = xs->u;
  xs->eo = xs->e;
}

/**
 * If log file and buffer exist, writes "count" entries to the log file from the start of the log buffer.
 */
static void flush_log_file(FILE* lf, const copper_log_buffer* lb, uint32_t count) {
  uint32_t i;
  for (i = 0; lb != NULL && lf != NULL && i < count; i++) {
    fprintf(lf,
            "%16"PRIu64" %16"PRIu64" %16f "
            "%16f %16f %16f %16f %16f %16f "
            "%16f %16f %16f %16f\n",
            lb[i].id, lb[i].user_tag, lb[i].constraint_achieved,
            lb[i].fs.x_hat_minus, lb[i].fs.x_hat, lb[i].fs.p_minus, lb[i].fs.h, lb[i].fs.k, lb[i].fs.p,
            lb[i].workload, lb[i].u, lb[i].e, lb[i].cu);
  }
}

/**
 * If the circular log buffer exists, record an entry in it.
 * If the log file exists and the buffer becomes full, the buffer will flush its contents to the file.
 */
static void copper_log(copper* cop, uint32_t tag, double constraint_achieved, double workload, double cost) {
  assert(cop != NULL);
  uint32_t i;
  const copper_filter_state* fs = &cop->fs;
  const copper_xup_state* xs = &cop->xs;
  copper_log_state* ls = &cop->ls;
  copper_log_buffer* lb = ls->lb;

  if (ls->lb_length > 0 && lb != NULL) {
    i = ls->id % ls->lb_length;
    lb[i].id = ls->id;
    lb[i].user_tag = tag;
    lb[i].constraint_achieved = constraint_achieved;
    memcpy(&lb[i].fs, fs, sizeof(copper_filter_state));
    lb[i].workload = workload;
    lb[i].u = xs->u;
    lb[i].e = xs->e;
    lb[i].cu = cost;

    if (i == ls->lb_length - 1) {
      // flush buffer to log file (if used)
      flush_log_file(ls->lf, lb, ls->lb_length);
    }
    ls->id++;
  }
}

int copper_init(copper* cop, double performance_target, double power_min, double power_max, double power_start) {
  if (cop == NULL || performance_target <= 0 ||
      power_min <= 0 || power_max < power_min || power_start < power_min || power_start > power_max) {
    errno = EINVAL;
    return -errno;
  }

  // set the constraint and power bounds
  cop->ctx.constraint_target = performance_target;
  cop->ctx.cost_min = power_min;
  cop->ctx.cost_max = power_max;

  // initialize variables used in performance filter
  cop->fs.x_hat_minus = X_HAT_MINUS_START;
  cop->fs.x_hat = X_HAT_START;
  cop->fs.p_minus = P_MINUS_START;
  cop->fs.h = H_START;
  cop->fs.k = K_START;
  cop->fs.p = P_START;
  cop->fs.q = Q_DEFAULT;
  cop->fs.r = R_DEFAULT;

  // initialize variables used for calculating speedup
  // estimate xup corresponding with this power_start
  cop->xs.u = power_start / power_min;
  cop->xs.uo = cop->xs.u;
  cop->xs.uoo = cop->xs.u;
  cop->xs.e = E_START;
  cop->xs.eo = EO_START;
  cop->xs.p1 = P1_DEFAULT;
  cop->xs.p2 = P2_DEFAULT;
  cop->xs.z1 = Z1_DEFAULT;
  cop->xs.mu = MU_DEFAULT;
  cop->xs.epc = EPC_DEFAULT;
  cop->xs.gl = GAIN_LIMIT_DEFAULT;

  // no logging by default
  cop->ls.id = 0;
  cop->ls.lb_length = 0;
  cop->ls.lb = NULL;
  cop->ls.lf = NULL;

  return 0;
}

void copper_destroy(copper* cop) {
  // flush log buffer (if used)
  if (cop != NULL && cop->ls.lb_length > 0) {
    flush_log_file(cop->ls.lf, cop->ls.lb, cop->ls.id % cop->ls.lb_length);
  }
}

double copper_adapt(copper* cop, uint64_t tag, double performance) {
  if (cop == NULL || performance < 0) {
    errno = EINVAL;
    return -errno;
  }

  const copper_context* ctx = &cop->ctx;
  // Estimate the performance workload, i.e. time between measurements given minimum power
  const double workload = estimate_base_workload(&cop->fs, performance, cop->xs.u);
  // Get a new xup
  calculate_xup(&cop->xs, ctx->constraint_target, performance, workload, cop->ls.id, ctx->cost_max / ctx->cost_min);
  // Get new cost
  const double cost = cop->xs.u * ctx->cost_min;
  assert(cost >= ctx->cost_min);
  assert(cost <= ctx->cost_max);
  // internal logging
  copper_log(cop, tag, performance, workload, cost);
  return cost;
}

int copper_set_logging(copper* cop, copper_log_buffer* lb, uint32_t lb_length, FILE* lf) {
  if (cop == NULL) {
    errno = EINVAL;
    return -errno;
  }
  if (lf != NULL) {
    // write header to log file
    errno = 0;
    if (fprintf(lf,
                "%16s %16s %16s "
                "%16s %16s %16s %16s %16s %16s "
                "%16s %16s %16s %16s\n",
                "ID", "USER_TAG", "CONSTRAINT",
                "X_HAT_MINUS", "X_HAT", "P_MINUS", "H", "K", "P",
                "WORKLOAD", "XUP", "ERROR", "COST") < 0) {
      if (!errno) {
        errno = EIO;
      }
      return -errno;
    }
  }
  // reset id (prevents writing garbage to files when logging is unset/set)
  cop->ls.id = 0;
  cop->ls.lb_length = lb_length;
  cop->ls.lb = lb;
  cop->ls.lf = lf;
  return 0;
}

int copper_set_performance_target(copper* cop, double target) {
  if (cop == NULL || target <= 0) {
    errno = EINVAL;
    return -errno;
  }
  cop->ctx.constraint_target = target;
  return 0;
}

int copper_set_gain_limit(copper* cop, double gain) {
  if (cop == NULL || gain < 0 || gain >= 1) {
    errno = EINVAL;
    return -errno;
  }
  cop->xs.gl = gain;
  return 0;
}
