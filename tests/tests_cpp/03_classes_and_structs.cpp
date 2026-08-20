#include <stdio.h>

class Point {
public:
    int x;
    int y;

    int get_x() {
        return x;
    }

    int get_y() {
        return y;
    }

    void set(int new_x, int new_y) {
        x = new_x;
        y = new_y;
    }

private:
    int secret;
};

struct Vector3D {
    float x;
    float y;
    float z;

    float length_sq() {
        return x * x + y * y + z * z;
    }
};

int main() {
    Point p;
    p.x = 10;
    p.y = 20;
    printf("Point: x=%d, y=%d\n", p.x, p.y);

    Vector3D v;
    v.x = 1.0f;
    v.y = 2.0f;
    v.z = 2.0f;
    printf("Vector3D: %f, %f, %f, sizeof=%d\n", v.x, v.y, v.z, (int)sizeof(Vector3D));
    return 0;
}
