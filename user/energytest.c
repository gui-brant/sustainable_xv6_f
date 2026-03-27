#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int pid = getpid();
  int loops = 200000000;
  int start_energy;
  int end_energy;
  volatile int i;

  if(argc > 1)
    loops = atoi(argv[1]);

  printf("starting energy test\n");
  start_energy = getenergy(pid);
  printf("[energytest] pid=%d start_energy=%d loops=%d\n", pid, start_energy, loops);
  for(i = 0; i < loops; i++){
    ;
  }

  end_energy = getenergy(pid);
  printf("[energytest] pid=%d end_energy=%d delta=%d\n", pid, end_energy, end_energy - start_energy);
  exit(0);
}