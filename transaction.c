#include <stdio.h>
#define N 10000

void *func(void *arg) {

  
}

pthread_t thread_id;

int main(){
  FILE *istream;
  istream = fopen(trans.csv,"r");
  int from[N],to[N],amount[N],account[100];
  
  char useless[N];

  for (i = 0;i < 100; i++){
    account[i] = 10000;
  }
  
  if (istream == NULL){
    printf("ファイルが開けません");
    return 1;
  }
  
  while (1){
    val = fscanf(istream,"%c,%d,%d,%d\n",useless[count],from[count],to[count],amount[count]);
    count += 1;
    if (val == NULL) break;

  }
  fclose(istream);
}
