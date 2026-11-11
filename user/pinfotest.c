
#include "kernel/types.h"
#include "kernel/pinfo.h"
#include "user/user.h"

int
main(void)
{
  struct pinfo info[64];
  int count;
  int pid = getpid();
  int initial_ticks = -1;
  int initial_sched = -1;

  count = getpinfo(info, 64);

  if (count < 0) {
    printf("FAIL: getpinfo returned -1\n");
    exit(1);
  }

  printf("Found %d processes\n", count);

  for (int i = 0; i < count; i++) {
    printf("PID: %d Name: %s State: %d Level: %d Runticks: %d Nsched: %d Ctime: %d\n",
           info[i].pid,
           info[i].name,
           info[i].state,
           info[i].level,
           info[i].runticks,
           info[i].nsched,
           info[i].ctime);

    if (info[i].pid == pid) {
      initial_ticks = info[i].runticks;
      initial_sched = info[i].nsched;
    }
  }

  if (initial_ticks < 0) {
    printf("FAIL: Current process not found\n");
    exit(1);
  }

  printf("Running CPU test...\n");

  volatile unsigned long x = 0;

  for (unsigned long i = 0; i < 100000000UL; i++)
    x += i;

  count = getpinfo(info, 64);

  if (count < 0) {
    printf("FAIL: Second getpinfo call failed\n");
    exit(1);
  }

  int final_ticks = -1;
  int final_sched = -1;

  for (int i = 0; i < count; i++) {
    if (info[i].pid == pid) {
      final_ticks = info[i].runticks;
      final_sched = info[i].nsched;
      break;
    }
  }

  printf("Initial runticks: %d\n", initial_ticks);
  printf("Final runticks: %d\n", final_ticks);
  printf("Initial nsched: %d\n", initial_sched);
  printf("Final nsched: %d\n", final_sched);

  if (final_ticks > initial_ticks)
    printf("PASS: runticks increased\n");
  else
    printf("FAIL: runticks did not increase\n");

  if (final_sched > initial_sched)
    printf("PASS: nsched increased\n");
  else
    printf("FAIL: nsched did not increase\n");

  if (getpinfo(info, -1) == -1)
    printf("PASS: Negative max rejected\n");
  else
    printf("FAIL: Negative max accepted\n");

  if (getpinfo(info, 0) == 0)
    printf("PASS: Zero max handled\n");
  else
    printf("FAIL: Zero max handled incorrectly\n");

  if (getpinfo((struct pinfo *)0, 64) == -1)
    printf("PASS: Invalid buffer rejected\n");
  else
    printf("FAIL: Invalid buffer accepted\n");

  exit(0);
}
