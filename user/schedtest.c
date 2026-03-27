#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

struct child_result {
  int pid;
  int mode;
  int loops;
  int start_energy;
  int end_energy;
  int dticks;
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

static int
readn(int fd, void *buf, int n)
{
  int rtot = 0;
  char *p = (char *)buf;

  while(rtot < n){
    int m = read(fd, p + rtot, n - rtot);
    if(m <= 0)
      return -1;
    rtot += m;
  }
  return 0;
}

static void
run_workload(int mode, int loops)
{
  volatile int sink = 0;
  int chunk = loops / 10;
  if(chunk < 1)
    chunk = 1;

  for(int i = 0; i < loops; i++){
    sink += (i ^ mode) & 0x7;

    if(mode == 1 && (i % chunk) == 0){
      pause(1);
    }

    if(mode == 2 && (i % (chunk / 2 + 1)) == 0){
      pause(2);
    }
  }

  if(sink == 123456789)
    printf("%d\n", sink);
}

int
main(int argc, char *argv[])
{
  int base_loops = 20000000;
  int children = 10;
  int pfd[2];
  int spawned = 0;

  if(argc > 1)
    base_loops = atoi(argv[1]);
  if(argc > 2)
    children = atoi(argv[2]);

  if(children < 1)
    children = 1;
  if(children > 8)
    children = 8;

  if(pipe(pfd) < 0){
    printf("schedtest: pipe failed\n");
    exit(1);
  }

  printf("schedtest: parent=%d base_loops=%d children=%d\n",
         getpid(), base_loops, children);

  for(int idx = 0; idx < children; idx++){
    int pid = fork();
    if(pid < 0){
      printf("schedtest: fork failed at idx=%d\n", idx);
      break;
    }

    if(pid == 0){
      close(pfd[0]);

      int mypid = getpid();
      int mode = idx % 3;
      int loops = base_loops * (idx + 1);
      int start = uptime();
      int start_energy = getenergy(mypid);
      struct child_result res;

      run_workload(mode, loops);

      res.pid = mypid;
      res.mode = mode;
      res.loops = loops;
      res.start_energy = start_energy;
      res.end_energy = getenergy(mypid);
      res.dticks = uptime() - start;

      writen(pfd[1], &res, sizeof(res));
      close(pfd[1]);
      exit(0);
    }

    spawned++;
  }

  close(pfd[1]);

  for(int i = 0; i < spawned; i++){
    struct child_result res;
    if(readn(pfd[0], &res, sizeof(res)) < 0)
      break;

    printf("child done  pid=%d mode=%d loops=%d energy=%d->%d dticks=%d\n",
           res.pid,
           res.mode,
           res.loops,
           res.start_energy,
           res.end_energy,
           res.dticks);
  }

  close(pfd[0]);

  for(int i = 0; i < spawned; i++){
    int st = 0;
    int done = wait(&st);
    if(done >= 0)
      printf("parent: reaped pid=%d status=%d\n", done, st);
  }

  exit(0);
}
