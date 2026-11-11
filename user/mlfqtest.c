
#include "kernel/types.h"
#include "kernel/pinfo.h"
#include "user/user.h"

int
main(void)
{
  struct pinfo info[16];
  int child;
  int lastlevel = -1;
  int sawdemotion = 0;
  int sawboost = 0;
  int sawlevel3 = 0;

  child = fork();

  if (child < 0) {
    printf("FAIL: fork failed\n");
    exit(1);
  }

  if (child == 0) {
    volatile unsigned long work = 0;

    while (1) {
      for (unsigned long i = 0; i < 1000000UL; i++)
        work += i;
    }
  }

  printf("Monitoring CPU-intensive child PID %d\n", child);

  for (int sample = 0; sample < 2000; sample++) {
    volatile unsigned long delay = 0;

    for (unsigned long j = 0; j < 1000000UL; j++)
    delay += j;

    int count = getpinfo(info, 16);

    if (count < 0) {
      printf("FAIL: getpinfo failed\n");
      kill(child);
      wait(0);
      exit(1);
    }

    for (int i = 0; i < count; i++) {
      if (info[i].pid == child) {
        int level = info[i].level;

        if (lastlevel >= 0) {
          if (level > lastlevel)
            sawdemotion = 1;

          if (level < lastlevel)
            sawboost = 1;
        }

        if (level == 3)
          sawlevel3 = 1;

        if (level != lastlevel) {
          printf("Sample %d: Level %d Runticks %d Nsched %d\n",
                 sample, level, info[i].runticks, info[i].nsched);
        }

        lastlevel = level;
        break;
      }
    }
  }

  kill(child);
  wait(0);

  if (sawdemotion)
    printf("PASS: Priority demotion observed\n");
  else
    printf("INFO: Priority demotion not observed\n");

  if (sawlevel3)
    printf("PASS: Child reached level 3\n");
  else
    printf("INFO: Child did not reach level 3\n");

  if (sawboost)
    printf("PASS: Priority boost observed\n");
  else
    printf("INFO: Priority boost not observed\n");

  exit(0);
}
