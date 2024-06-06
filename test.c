#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum snr_type {
    SNR_TYPE_TH = 0,
    SNR_TYPE_WL,
    SNR_TYPE_SM,
    SNR_TYPE_HM,
} snr_type_e;

typedef struct Node {
    int thr_lvl;
    int pin_num;
    snr_type_e snr_type;
} node_t;

// This function prints contents of linked list starting
// from the given node
void printList(node_t *n, int *snr_data)
{
    int idx;
    bool flag = false;
    for (idx = 0; idx < 3; idx++) {
        if (n[idx].thr_lvl < snr_data[idx]) {

            switch (n[idx].snr_type) {
            case SNR_TYPE_TH:
                printf("Tmp critica\n");
                break;

            case SNR_TYPE_WL:
                printf("H2o critica\n");
                break;

            case SNR_TYPE_SM:
                printf("Moi critica\n");
                break;

            case SNR_TYPE_HM:
                printf("Hum critica\n");
                break;
            }
            flag = false;
            idx = 4;
        } else {
            printf("thr_lvl: %d | pin_num: %d\n", n->thr_lvl, n->pin_num);
            flag = true;
        }
    }

    if (flag)
        printf("Sistema en marcha");
    else
        printf("Sistema no está listo");
}

// Driver's code
int main()
{
    node_t head[] = {
        {25, 15, SNR_TYPE_HM},
        {30, 16, SNR_TYPE_WL},
        {10, 17, SNR_TYPE_TH},
    };

    int snr_data[] = {50, 15, 30};
    // Function call
    printList(head, snr_data);

    return 0;
}