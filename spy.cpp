#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <fcntl.h>
#include <sched.h>
#include <sys/mman.h>
#include "cacheutils.h"
#include <map>
#include <vector>
#include <algorithm>
#include <thread>
#include <condition_variable>
#include <mutex>

// more trials show features more clearly
#define NUMBER_OF_TRIALS (1024*1024)

size_t data[5*1024];

std::map<size_t *, size_t> timings;

int rnd;

volatile uint64_t timer = 0;

void access_array(bool key) {
    if (key) {
        maccess(data+2*1024);
        maccess(data+2*1024);
    }
    else {
        maccess(data);
        maccess(data);
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
  read(rnd, &rb, sizeof(rb));
  key = rb < 128 ? 0 : 1;
  size_t *probe = data;
  for (size_t i = 0; i < NUMBER_OF_TRIALS; ++i) {
    sched_yield();
    flush(probe);
    access_array(key);
    size_t time = timer;
    flush(probe);
    int64_t delta = timer - time;
    if (delta < 0) { i-- ; continue; }
    timings[probe] += delta;
    //for (int j = 0; j < 30; j++) sched_yield();
  }
  probe = data+2*1024;
  for (size_t i = 0; i < NUMBER_OF_TRIALS; ++i) {
    sched_yield();
    flush(probe);
    sched_yield();
    access_array(key);
    size_t time = timer;
    flush(probe);
    int64_t delta = timer - time;
    if (delta < 0) { i-- ; continue; }
    timings[probe] += delta;
    //for (int j = 0; j < 30; j++) sched_yield();
  }
  
  printf("Key: %d, data[0] = %p, data+2*1024 = %p\n", !!key, data, data+2*1024);
  for (auto ait : timings) {
    float avg = ait.second/(float)NUMBER_OF_TRIALS;
    printf("%p : %f\n", ait.first, avg);
  }
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
  read(rnd, data, sizeof(data));
  std::thread worker(timerthread);
#ifdef PIN_THREAD
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);
  CPU_SET(0, &cpuset);
  pthread_setaffinity_np(worker.native_handle(), sizeof(cpu_set_t), &cpuset); 
#endif

  while (timer == 0) usleep(1);

  printf("Timer value 1: %zu\n", timer);
  printf("Timer value 2: %zu\n", timer);
  guess();

  // Clean up
  worker.detach();
  close(rnd);
  fflush(stdout);
  return 0;
}
