#include <stdio.h>
#include <stdlib.h>

#define SIZE 1500000

static int arr[SIZE];

void swap_int(int &a, int &b) {
    int t = a;
    a = b;
    b = t;
}

void quicksort_cpp(int *a, int low, int high) {
    if (low < high) {
        int pivot = a[high];
        int i = low - 1;
        for (int j = low; j < high; j++) {
            if (a[j] <= pivot) {
                i++;
                swap_int(a[i], a[j]);
            }
        }
        swap_int(a[i + 1], a[high]);
        int p = i + 1;
        quicksort_cpp(a, low, p - 1);
        quicksort_cpp(a, p + 1, high);
    }
}

int main() {
    unsigned int seed = 123456789;

    for (int i = 0; i < SIZE; i++) {
        seed = seed * 1103515245 + 12345;
        arr[i] = static_cast<int>(seed % 10000000);
    }

    quicksort_cpp(arr, 0, SIZE - 1);

    long long sum = 0;
    for (int i = 0; i < SIZE; i++) {
        if (i > 0 && arr[i] < arr[i - 1]) {
            printf("CppSort FAILED at %d!\n", i);
            return 1;
        }
        sum += (arr[i] % 100);
    }

    printf("CppQuickSort: SIZE=%d Checksum=%lld\n", SIZE, sum);
    return 0;
}
