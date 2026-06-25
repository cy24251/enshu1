#include <time.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
    int num_txn, num_account, max_amount, i;
    if (argc!=4) {
            printf("usage: txgen num_txn num_account max_amount\n");
            exit(1);
    }
    num_txn = atoi(argv[1]);
    num_account = atoi(argv[2]);
    max_amount = atoi(argv[3]);

    srand(time(NULL));

//    printf("num_txn=%d, num_account=%d, max_amount=%d\n", num_txn, num_account, max_amount);
    for (i = 0; i<num_txn; i++) {
            int toVal, amount;
            int fromVal = rand() % num_account;
            while (1) {
                    toVal = rand() % num_account;
                    if (fromVal != toVal) break;
            }
            amount = (rand() % max_amount) + 1;
            printf("t,%d,%d,%d\n", fromVal, toVal, amount);
    }
    return 0;
}
