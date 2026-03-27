#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

struct child_report {
  int pid;
  int budget;
  int start_energy;
  int end_energy;
};

static int
writen(int fd, void *buf, int n)
{
  int written = 0;
  char *p = (char *)buf;

  while(written < n){
    int m = write(fd, p + written, n - written);
    if(m <= 0)
      return -1;
    written += m;
  }
  return 0;
}

static void
busy_work(int loops)
{
  volatile int sink = 0;
  for(int i = 0; i < loops; i++){
    sink += (i & 7);
  }

  if(sink == 123456789)
    printf("%d\n", sink);
}

int
main(int argc, char *argv[])
{
  int children = 3;
  int loops = 120000000;
  int pfd[2];
  int spawned = 0;
  int child_pid[8];
  int child_budget[8];
  int reported[8];

  int budgets[3] = {20, 800, 2500};

  if(argc > 1)
    loops = atoi(argv[1]);
  if(argc > 2)
    children = atoi(argv[2]);

  if(children < 1)
    children = 1;
  if(children > 8)
    children = 8;

  if(pipe(pfd) < 0){
    printf("[budgettest] error: pipe failed\n");
    exit(1);
  }

  printf("starting budget test\n");

  for(int i = 0; i < children; i++){
    int mybudget;

    if(i < 3)
      mybudget = budgets[i];
    else
      mybudget = 500 + i * 400;

    int pid = fork();
    if(pid < 0){
      printf("[budgettest] error: fork failed idx=%d\n", i);
      break;
    }

    if(pid == 0){
      struct child_report rep;
      int me = getpid();
      close(pfd[0]);

      if(setenergybudget(me, mybudget) < 0){
        close(pfd[1]);
        exit(1);
      }

      rep.pid = me;
      rep.budget = mybudget;
      rep.start_energy = getenergy(me);

      busy_work(loops * (i + 1));

      rep.end_energy = getenergy(me);
      writen(pfd[1], &rep, sizeof(rep));
      close(pfd[1]);
      exit(0);
    }

    child_pid[spawned] = pid;
    child_budget[spawned] = mybudget;
    reported[spawned] = 0;
    spawned++;
  }

  close(pfd[1]);

  for(;;){
    struct child_report rep;
    int n = read(pfd[0], &rep, sizeof(rep));

    if(n == 0)
      break;
    if(n != sizeof(rep))
      break;

    for(int k = 0; k < spawned; k++){
      if(child_pid[k] == rep.pid){
        reported[k] = 1;
        break;
      }
    }

    printf("[budgettest] child pid=%d budget=%d energy=%d->%d\n",
           rep.pid,
           rep.budget,
           rep.start_energy,
           rep.end_energy);
  }

  close(pfd[0]);

  for(int i = 0; i < spawned; i++){
    int st = 0;
    int done = wait(&st);
    int budget = -1;
    int had_report = 0;

    if(done < 0)
      continue;

    for(int k = 0; k < spawned; k++){
      if(child_pid[k] == done){
        budget = child_budget[k];
        had_report = reported[k];
        break;
      }
    }

    if(st != 0)
      printf("[budgettest] reap pid=%d status=%d budget=%d (killed: went over budget)\n", done, st, budget);
    else if(!had_report)
      printf("[budgettest] reap pid=%d status=%d budget=%d (finished, no final report)\n", done, st, budget);
    else
      printf("[budgettest] reap pid=%d status=%d budget=%d (finished within budget)\n", done, st, budget);
  }

  exit(0);
}
