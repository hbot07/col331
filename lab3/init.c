// init: The initial user-level program

#include "types.h"
#include "user.h"
#include "fcntl.h"

int
main(void)
{
  int fd = open("console", O_RDWR);
  printf(fd, "Hello COL%d from init.c!\n", 331);
  
  int ret = get_sched_policy();
  printf(fd, "Set sched policy of process to : %d \n", ret);

  for (int j=0; j<20000; j++) {
    int i = 0;
    while(i < 100000000) {i++;}   
    printf(fd, "Task done by process with sched policy %d ... \n", ret);
  }
  if (ret == 0)
    printf(fd, "Scheduling policy retrieved is equal to 0.\n");
  else
    printf(fd, "Scheduling policy retrieved is equal to 1.\n");

  close(fd);

}
