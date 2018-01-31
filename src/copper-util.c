/**
 * CoPPer utility functions.
 *
 * @author Connor Imes
 * @date 2016-05-24
 */
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "copper-util.h"
#include "copper.h"

copper* copper_alloc_init(double performance_target, double power_min, double power_max, double power_start,
                          uint32_t lb_length, const char* log_filename) {
  copper* cop;
  copper_log_buffer* lb = NULL;
  FILE* lf = NULL;
  int err_save;

  // create the copper struct
  cop = malloc(sizeof(copper));
  if (cop == NULL) {
    return NULL;
  }

  // allocate log buffer
  if (lb_length > 0) {
    lb = malloc(lb_length * sizeof(copper_log_buffer));
    if (lb == NULL) {
      free(cop);
      return NULL;
    }
    // Open log file
    if (log_filename != NULL) {
      lf = fopen(log_filename, "w");
      if (lf == NULL) {
        free(lb);
        free(cop);
        return NULL;
      }
    }
  }

  // initialize and set logging
  if (copper_init(cop, performance_target, power_min, power_max, power_start) ||
      copper_set_logging(cop, lb, lb_length, lf)) {
    err_save = errno;
    if (lf != NULL) {
      fclose(lf);
    }
    free(lb);
    free(cop);
    errno = err_save;
    return NULL;
  }

  return cop;
}

int copper_destroy_free(copper* cop) {
  int ret = 0;
  if (cop != NULL) {
    copper_destroy(cop);
    if (cop->ls.lf != NULL && fclose(cop->ls.lf)) {
      ret = -errno;
    }
    free(cop->ls.lb);
    free(cop);
  }
  return ret;
}
