#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sched.h>
#include <thread>
#include <unistd.h>
#include "../cacheutils.h"

size_t array[5*1024];

volatile uint64_t timer = 0;

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

size_t onlyreload(size_t* addr)
{
  size_t time = timer;
  flush(addr);
  size_t delta = timer - time;
  maccess(addr);
  maccess(addr);
  return delta;
}

size_t flushandreload(size_t* addr)
{
  size_t time = timer;
  flush(addr);
  size_t delta = timer - time;
  flush(addr);
  return delta;
}

int main(int argc, char** argv)
{
  std::thread worker(timerthread);

  while (timer == 0) usleep(1);

  memset(array,-1,5*1024*sizeof(size_t));
  maccess(array + 2*1024);
  sched_yield();
  for (int i = 0; i < 1024; ++i)
  {
    size_t d = onlyreload(array+2*1024);
    if (d == 0) {
        i--; continue;
    }
    printf("H %zu\n", d);
    //hit_histogram[MIN(9999,d)]++;
    for (size_t i = 0; i < 30; ++i)
      sched_yield();
  }
  flush(array+1024);
  for (int i = 0; i < 1024; ++i)
  {
    size_t d = flushandreload(array+2*1024);
    if (d == 0) {
        i--; continue;
    }
    printf("M %zu\n", d);

    //miss_histogram[MIN(9999,d)]++;
    for (size_t i = 0; i < 30; ++i)
      sched_yield();
  }
  size_t hit_max = 0;
  size_t hit_max_i = 0;
  size_t miss_min_i = 0;
  for (size_t i = 0; i < 10000; ++i)
  {
    //printf("%3zu: %10zu %10zu\n",i,hit_histogram[i],miss_histogram[i]);
  }
  worker.detach();
  return 0;
}
