#include <stdio.h>

int main(){
  long long a = 0;
  for (int i = 0;i <= 1000000000;i++){
    a = a + i;
  }
  printf("%lld",a);
  return 0;
}
