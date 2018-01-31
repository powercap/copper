/**
 * A basic usage example.
 */
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "copper.h"
#include "copper-util.h"

static const uint64_t ITERATIONS = 10;
static const uint64_t WINDOW_SIZE = 2;
static const uint32_t LB_LENGTH = 1;
// the application measures its own performance
static const double PERFORMANCE_TARGET = 100.0;
// power can be in any units, e.g. watts or microwatts
static const double POWER_MIN = 10.0;
static const double POWER_MAX = 100.0;
static const double POWER_START = 60.0;

// Some function to apply new power setting...
static void apply_powercap(double powercap) {
  (void) powercap;
}

static void application_do_work(void) {
  // business logic goes here...
}

static void application_loop(copper* cop) {
  uint64_t i;
  double powercap;
  double performance = 0.0;
  // top-level loop
  for (i = 0; i < ITERATIONS; i++) {
    // only change power every WINDOW_SIZE iterations
    if (i != 0 && i % WINDOW_SIZE == 0) {
      // would use a real performance measurement here...
      performance = 200.0;
      // adapt power to meet performance target (first call will do nothing since performance is 0)
      powercap = copper_adapt(cop, i, performance);
      if (powercap <= 0) {
        perror("copper_adapt");
        exit(1);
      }
      apply_powercap(powercap);
    }
    // perform business logic
    application_do_work();
  }
}

static void basic_example(void) {
  copper cop;

  // initialize the controller
  if (copper_init(&cop, PERFORMANCE_TARGET, POWER_MIN, POWER_MAX, POWER_START)) {
    perror("copper_init");
    exit(1);
  }

  // run application
  application_loop(&cop);

  // cleanup
  copper_destroy(&cop);
}

static void util_example(const char* logfile) {
  // initialize the controller using utility function
  // if logfile not NULL, will also enable logging
  copper* cop = copper_alloc_init(PERFORMANCE_TARGET, POWER_MIN, POWER_MAX, POWER_START, LB_LENGTH, logfile);
  if (cop == NULL) {
    perror("copper_alloc_init");
    exit(1);
  }

  // run application
  application_loop(cop);

  // cleanup using utility function
  if (copper_destroy_free(cop)) {
    perror("copper_destroy_free");
    exit(1);
  }
}

// first param is a logfile
int main(int argc, char** argv) {
  const char* logfile = NULL;
  if (argc > 1) {
    logfile = argv[1];
  }
  if (logfile == NULL) {
    basic_example();
  } else {
    util_example(logfile);
  }
  return 0;
}
