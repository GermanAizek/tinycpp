#include <stdio.h>

struct Item {
    int id;
    int value;
};

int main() {
    int *p = new int;
    *p = 12345;
    printf("new int: %d\n", *p);
    delete p;

    Item *item = new Item;
    item->id = 1;
    item->value = 999;
    printf("new Item: id=%d, value=%d\n", item->id, item->value);
    delete item;

    int *arr = new int[5];
    for (int i = 0; i < 5; i++) {
        arr[i] = (i + 1) * 10;
    }
    printf("new int[]: %d %d %d %d %d\n", arr[0], arr[1], arr[2], arr[3], arr[4]);
    delete[] arr;

    return 0;
}
