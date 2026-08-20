#include <stdio.h>

int multiply(int a, int b = 10, int c = 2) {
    return a * b * c;
}

void print_coords(int x, int y = 5, int z = 10) {
    printf("coords: %d %d %d\n", x, y, z);
}

int main() {
    printf("multiply(3): %d\n", multiply(3));
    printf("multiply(3, 4): %d\n", multiply(3, 4));
    printf("multiply(3, 4, 5): %d\n", multiply(3, 4, 5));

    print_coords(1);
    print_coords(1, 2);
    print_coords(1, 2, 3);

    return 0;
}
