#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

static char *
state_name(int state)
{
  switch(state){
  case 0:
    return "unused";
  case 1:
    return "used";
  case 2:
    return "sleep";
  case 3:
    return "runble";
  case 4:
    return "run";
  case 5:
    return "zombie";
  default:
    return "?";
  }
}

int
main(int argc, char *argv[])
{
  struct procinfo procs[NPROC];
  int n;

  if(argc != 1){
    printf("usage: plist\n");
    exit(1);
  }

  n = listprocs(procs, NPROC);
  if(n < 0){
    printf("plist: listprocs failed\n");
    exit(1);
  }

  printf("pid\tname\tstate\tenergy\n");
  for(int i = 0; i < n; i++){
    printf("%d\t%s\t%s\t%d\n",
           procs[i].pid,
           procs[i].name,
           state_name(procs[i].state),
           procs[i].energy
        );
  }

  exit(0);
}
