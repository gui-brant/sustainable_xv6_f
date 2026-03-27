#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pid = getpid();
  int loops = 200000000;
  volatile int i;

  if(argc > 1)
    loops = atoi(argv[1]);

  printf("starting pid=%d energy=%d\n", pid, getenergy(pid));

  for(i = 0; i < loops; i++){
    ;
  }

  printf("ending pid=%d energy=%d\n", pid, getenergy(pid));
  exit(0);
}