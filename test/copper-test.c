/**
 * Some very basic unit tests.
 * These do not do complete code coverage (or even close to it).
 */
// force assertions
#undef NDEBUG
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include "copper.h"
#include "copper-util.h"

static const double PERFORMANCE_TARGET = 1.0;
static const double POWER_MIN = 0.01;
static const double POWER_MAX = 100.0;
static const double POWER_START = 50.0;

static const uint32_t LB_LENGTH = 1;
static const char* LOG_FILENAME = NULL;

static void test_standard_use_case(void) {
  copper cop;
  // init
  int ret = copper_init(&cop, PERFORMANCE_TARGET,
                        POWER_MIN, POWER_MAX, POWER_START);
  assert(ret == 0);
  // normal adapt
  double pwr = copper_adapt(&cop, 0, PERFORMANCE_TARGET);
  assert (pwr > 0);
  // gain limit (bad and good values)
  ret = copper_set_gain_limit(&cop, -1);
  assert (ret == -EINVAL);
  ret = copper_set_gain_limit(&cop, 1);
  assert (ret == -EINVAL);
  ret = copper_set_gain_limit(&cop, 0.5);
  assert(ret == 0);
  // destroy
  copper_destroy(&cop);
}

static void test_util_alloc_init_free(void) {
  copper* cop = copper_alloc_init(PERFORMANCE_TARGET, POWER_MIN, POWER_MAX, POWER_START,
                                  LB_LENGTH, LOG_FILENAME);
  assert(cop != NULL);
  int ret = copper_destroy_free(cop);
  assert(ret == 0);
}

int main(void) {
  test_standard_use_case();
  test_util_alloc_init_free();
}
