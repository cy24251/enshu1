#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define N 10000

int from[N], to[N], amount[N], account[100];
char useless[N];

pthread_mutex_t acc_mutex[100];
pthread_cond_t acc_cond[100];

void *func(void *arg) {
    int thn = (int)(long)arg; 
    int start = thn * 1000;
    int end = start + 1000;

    for (int i = start; i < end; i++) {
        int f = from[i];   
        int t = to[i];     
        int am = amount[i]; 

        if (f == t) continue;

        while (1) {
            pthread_mutex_lock(&acc_mutex[f]);

            // 残高不足時はロックを保持したまま待機
            if (account[f] < am) {
                pthread_cond_wait(&acc_cond[f], &acc_mutex[f]);
                pthread_mutex_unlock(&acc_mutex[f]);
                continue; 
            }

            // デッドロック回避のため、送金先はtrylockで確保
            if (pthread_mutex_trylock(&acc_mutex[t]) == 0) {
                break; 
            } else {
                pthread_mutex_unlock(&acc_mutex[f]); // 失敗時は解放してやり直し
                usleep(100); 
            }
        }

        account[f] -= am;
        account[t] += am;

        // 送金先口座に通知を送る
        pthread_cond_signal(&acc_cond[t]); 

        pthread_mutex_unlock(&acc_mutex[t]);
        pthread_mutex_unlock(&acc_mutex[f]);
    }
    return NULL;
}

int main() {
    FILE *istream = fopen("trans.csv", "r"); 
    int count = 0;
    
    if (istream == NULL) return 1;

    for (int i = 0; i < 100; i++) {
        account[i] = 10000;
        pthread_mutex_init(&acc_mutex[i], NULL);
        pthread_cond_init(&acc_cond[i], NULL);
    }
    
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
    for (int i = 0; i < 100; i++) {
        sumAmount += account[i];
        pthread_mutex_destroy(&acc_mutex[i]);
        pthread_cond_destroy(&acc_cond[i]);
    }

    printf("最終的な全口座の合計残高: %d 円\n", sumAmount);
    return 0;
}
