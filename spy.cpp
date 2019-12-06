#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include "cacheutils.h"
#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include <list>
#include "config.h"

// more trials show features more clearly
#ifndef NUMBER_OF_TRIALS
#define NUMBER_OF_TRIALS (16*1024*1024)
#endif

#ifdef USE_RDTSC
#define TIMED(x) \
  size_t time = rdtsc();\
  x;\
  int64_t delta = rdtsc() - time
#else
#define TIMED(x) \
  size_t time = timer;\
  x;\
  int64_t delta = timer - time
#endif

size_t data[5*1024];

std::map<size_t *, size_t> timings;
#ifdef HISTOGRAM
std::map<size_t *, std::vector<int64_t>> histo;
#endif

int rnd;

volatile uint64_t timer = 0;

void access_array(bool key) {
    if (key) {
        maccess(data+2*1024);
#ifndef NO_DOUBLE_ACCESS
        maccess(data+2*1024);
#endif
    }
    else {
        maccess(data);
#ifndef NO_DOUBLE_ACCESS
        maccess(data);
#endif
    }
}

void timerthread() {
    asm volatile ("xor %%rax, %%rax\n"
         "lea %[timer], %%rbx\n"
         "1: inc %%rax\n"
         "movq %%rax, (%%rbx)\n"
         "jmp 1b"
         : /* no output registers */
         : [timer] "m" (timer)
         : "%rax", "%rbx"
        );

    // while (true) timer++;
}

void guess() {
  // Clear timing map
  timings.clear();
  uint8_t rb;
  bool key;
  int n = read(rnd, &rb, sizeof(rb));
  if (!n) return;
  key = rb < 128 ? 0 : 1;
  size_t *probe = data;
  for (size_t i = 0; i < NUMBER_OF_TRIALS; ++i) {
#ifndef NOYIELD
    sched_yield();
#endif
    flush(probe);
#ifndef NOYIELD
    sched_yield();
#endif
    access_array(key);
    TIMED(flush(probe));
    if (delta < 0) { i-- ; continue; }
    timings[probe] += delta;
#ifdef HISTOGRAM
    histo[probe].push_back(delta);
#endif
  }
  probe = data+2*1024;
  for (size_t i = 0; i < NUMBER_OF_TRIALS; ++i) {
#ifndef NOYIELD
    sched_yield();
#endif
    flush(probe);
#ifndef NOYIELD
    sched_yield();
#endif
    access_array(key);
    TIMED(flush(probe));
    if (delta < 0) { i-- ; continue; }
    timings[probe] += delta;
#ifdef HISTOGRAM
    histo[probe].push_back(delta);
#endif
  }
  
  printf("Key: %d, data[0] = %p, data+2*1024 = %p\n", !!key, data, data+2*1024);
  printf("Mean:\n");
  for (auto ait : timings) {
    float avg = ait.second/(float)NUMBER_OF_TRIALS;
    printf("%p : %f\n", ait.first, avg);
  }
#ifdef HISTOGRAM
  // Sort so we can get median easily
  for (auto ait: histo) {
      std::sort(ait.second.begin(), ait.second.end());
  }
  printf("Median:\n");
  for (auto ait : histo) {
    printf("%p : %zu\n", ait.first, ait.second[ait.second.size()/2]);
  }

  printf("Nonzero entries:\n");
  for (auto ait : histo) {
    int nz = 0;
    for (auto rit = ait.second.rbegin(); rit != ait.second.rend(); ++rit) {
      //printf("%zu\n", *rit);
      if (*rit != 0) nz++;
    }
    printf("%p : %d\n", ait.first, nz);
  }
#endif

  // If timings[data] < timings[data+2*1024] then it took less time
  // to flush data[0], meaning it was not in the cache. So the key
  // must be 1
  bool key_guess = (timings[data] < timings[data+2*1024] ? 1 : 0);
  printf("Key %d guess %d ", key, key_guess);
  if (key_guess == key) printf("CORRECT\n");
  else printf("WRONG\n");
}

int main(int argc, char **argv)
{
  // Get randomness from /dev/urandom. This is a much better RNG
  // than trying to use rand()
  rnd = open("/dev/urandom", O_RDONLY);
  int n = read(rnd, data, sizeof(data));
  if (!n) return 1;
#ifndef USE_RDTSC
  std::thread worker(timerthread);
#ifdef PIN_THREAD
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(0, &cpuset);
  pthread_setaffinity_np(worker.native_handle(), sizeof(cpu_set_t), &cpuset); 
#endif

  while (timer == 0) usleep(1);
#endif

  guess();

  // Clean up
#ifndef USE_RDTSC
  worker.detach();
#endif
  close(rnd);
  fflush(stdout);
  return 0;
}
