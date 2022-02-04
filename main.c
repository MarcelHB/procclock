// SPDX-License-Identifier: Unlicense
//
// procclock, sending SIGSTOP and SIGCONT to some process to simulate
// a limited platform.

#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

const int TRAIN_ITERATIONS = 1000;
const int BILLION = 1000000000;
const int MILLION = 1000000;

struct context {
  int pid;
  int running;
} context;

/* Don't forget to continue the process if we are forced to leave. */
void sig_handler (int num) {
  if (!context.running) {
    kill(context.pid, SIGCONT);
  }
  exit(0);
}

int main (int argc, char **argv) {
  context.running = 0;
  signal(SIGINT, sig_handler);

  if (argc < 4) {
    fprintf(stderr, "Usage: PID us Hz\n");
    return 1;
  }

  context.pid = atoi(argv[1]);
  const int time = atoi(argv[2]);
  const int freq = atoi(argv[3]);

  if (time <= 0 || freq <= 0) {
    fprintf(stderr, "Time and frequency need to be positive, non-zero.\n");
    return 1;
  }

  if (freq >= MILLION / 2) {
    fprintf(stderr, "Frequency must be less than half a million.\n");
    return 1;
  }

  if (kill(context.pid, 0) != 0) {
    fprintf(stderr, "Process ID not found.\n");
    return 1;
  }

  const struct timespec train_clock = { .tv_sec = 0, .tv_nsec = 1000 };
  struct timespec t1 = {0};
  struct timespec t2 = {0};

  /**
   * Trying to get what 1us actually turns out to be, and stick to this
   * for the rest of the program. It's primitive but works well for simulating
   * a busy, slow execution.
   */
  uint64_t diff_ns = 0;
  for (int i = 0; i < TRAIN_ITERATIONS; ++i) {
    clock_gettime(CLOCK_MONOTONIC, &t1);
    nanosleep(&train_clock, NULL);
    clock_gettime(CLOCK_MONOTONIC, &t2);

    if (t2.tv_sec > t1.tv_sec) {
      diff_ns += (t2.tv_sec - 1 - t1.tv_sec) * BILLION + t2.tv_nsec + (BILLION - t1.tv_nsec);
    } else {
      diff_ns += t2.tv_nsec - t1.tv_nsec;
    }
  }

  const uint64_t real_wait_time_ns = diff_ns / TRAIN_ITERATIONS;
  const uint64_t real_wait_time_us = real_wait_time_ns / 1000;
  const struct timespec clock = { .tv_sec = 0, .tv_nsec = real_wait_time_ns };

  if (real_wait_time_us == 0) {
    fprintf(stderr, "Interval time calculated below 1 us, that's not working.\n");
    return 1;
  }

  if (MILLION / freq < real_wait_time_us || time < real_wait_time_us) {
    printf("Warning: requirements too high for system, ticking approx. every %lu us instead.\n", real_wait_time_us);
  }

  context.running = 1;
  uint64_t runtime_us = 0;
  uint64_t last_go_us = 0;
  uint64_t us_since_start = 0;
  const int start_interval = MILLION / freq;

  while (1) {
    if (!context.running && (us_since_start - last_go_us >= start_interval)) {
      if (kill(context.pid, SIGCONT) != 0) {
        printf("Lost process.\n");
        break;
      }

      last_go_us = us_since_start;
      runtime_us = 0;
      context.running = 1;
    } else if (context.running && runtime_us >= time) {
      if (kill(context.pid, SIGSTOP) != 0) {
        printf("Lost process.\n");
        break;
      }

      context.running = 0;
    } else if (context.running == 1) {
      runtime_us += real_wait_time_us;
    }

    nanosleep(&clock, NULL);
    us_since_start += real_wait_time_us;
  }

  return 0;
}
