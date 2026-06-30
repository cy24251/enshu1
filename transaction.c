#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 10000

int from[N], to[N], amount[N], account[100];
char useless[N];

// 全ての口座を1つの鍵で守る（一番安全な方法）
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvar = PTHREAD_COND_INITIALIZER;

void *func(void *arg) {
    int thn = (int)(long)arg; 
    int start = thn * 1000;
    int end = start + 1000;

    for (int i = start; i < end; i++) {
        int f = from[i];
        int t = to[i];
        int am = amount[i];

        pthread_mutex_lock(&mutex); // 全体のロック開始

        // 残高不足なら誰かが振り込むまで待機
        while (account[f] < am) {
            pthread_cond_wait(&cvar, &mutex);
        }

        account[f] -= am;
        account[t] += am;

        // 誰かの残高が増えたので、待っているスレッド全員に通知
        pthread_cond_broadcast(&cvar); 

        pthread_mutex_unlock(&mutex); // 全体のロック解除
    }
    return NULL;
}

int main() {
    FILE *istream = fopen("trans.csv", "r"); 
    int count = 0;
    
    if (istream == NULL) {
        printf("ファイルが開けません\n");
        return 1;
    }

    for (int i = 0; i < 100; i++) account[i] = 10000;
    
    while (1) {
        int val = fscanf(istream, "%c,%d,%d,%d\n", &useless[count], &from[count], &to[count], &amount[count]);
        if (val == EOF) break;
        count++;
    }
    fclose(istream);

    pthread_t threads[10];
    for (int i = 0; i < 10; i++) {
        pthread_create(&threads[i], NULL, func, (void *)(long)i);
    }
    for (int i = 0; i < 10; i++) {
        pthread_join(threads[i], NULL);
    }

    int sumAmount = 0;
    for (int i = 0; i < 100; i++) sumAmount += account[i];

    printf("最終的な全口座の合計残高: %d 円\n", sumAmount);
    return 0;
}
