#include<unistd.h>
#include<signal.h>
#include<stdio.h>
#include<sys/time.h>
int setitimer(int ITIMER_REAL,const struct itimerval *value,struct itimerval *ovalue);
struct itimerval{
  struct timeval it_interval;
  struct timeval it_value;
}

void sig_handler(int signum){
  printf("hello");
  
}

int main()
{
  signal{14,sig_handler);
  while(1){
    printf("z\n");
    sleep(1);
  }
}
