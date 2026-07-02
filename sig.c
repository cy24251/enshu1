#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

int main(int argc,char *argv[]){
  if(argc != 2){
    printf("プロセスIDを指定してください");
    return 1;
  }
  
  int pid = atoi(argv[1]);
  kill(pid,SIGUSR1);
  return 0;
}
  
