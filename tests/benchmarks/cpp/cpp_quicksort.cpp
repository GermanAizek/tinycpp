#include <stdio.h>
#include <stdlib.h>

#define SIZE 1500000

static int arr[SIZE];

static void quicksort(int *a, int low, int high) {
    if (low < high) {
        int pivot = a[high];
        int i = low - 1;
        int j, t;
        for (j = low; j < high; j++) {
            if (a[j] <= pivot) {
                i++;
                t = a[i]; a[i] = a[j]; a[j] = t;
            }
        }
        t = a[i + 1]; a[i + 1] = a[high]; a[high] = t;
        int p = i + 1;
        quicksort(a, low, p - 1);
        quicksort(a, p + 1, high);
    }
}

int main(void) {
    int i;
    unsigned int seed = 123456789;

    for (i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr[i] = (int)(seed % 10000000);
    }

    quicksort(arr, 0, SIZE - 1);

    long long sum = 0;
    for (i = 0; i < SIZE; i++) {
        if (i > 0 && arr[i] < arr[i - 1]) {
            printf("Sort FAILED at %d!\n", i);
            return 1;
        }
        sum += (arr[i] % 100);
    }

    printf("QuickSort: SIZE=%d Checksum=%lld\n", SIZE, sum);
    return 0;
}
