#include <stdio.h>
#include <stdlib.h>

#define NUM_SHAPES 500000

class Shape {
public:
    int type; // 1: Circle, 2: Rectangle, 3: Triangle
    double x;
    double y;
};

class Circle : public Shape {
public:
    double radius;
};

class Rectangle : public Shape {
public:
    double width;
    double height;
};

class Triangle : public Shape {
public:
    double base;
    double height;
};

double circle_area(const Circle &c) {
    return 3.141592653589793 * c.radius * c.radius;
}

double rect_area(const Rectangle &r) {
    return r.width * r.height;
}

double triangle_area(const Triangle &tr) {
    return 0.5 * tr.base * tr.height;
}

static Shape shapes[NUM_SHAPES];

int main() {
    double total_area = 0.0;
    unsigned int seed = 987654321;

    for (int i = 0; i < NUM_SHAPES; i++) {
        seed = seed * 1103515245 + 12345;
        int t = static_cast<int>(seed % 3) + 1;
        double a = static_cast<double>((seed >> 8) % 100) + 1.0;
        double b = static_cast<double>((seed >> 16) % 100) + 1.0;

        shapes[i].type = t;
        shapes[i].x = static_cast<double>(i);
        shapes[i].y = static_cast<double>(i);

        if (t == 1) {
            Circle *c = (Circle *)&shapes[i];
            c->radius = a;
        } else if (t == 2) {
            Rectangle *r = (Rectangle *)&shapes[i];
            r->width = a;
            r->height = b;
        } else {
            Triangle *tr = (Triangle *)&shapes[i];
            tr->base = a;
            tr->height = b;
        }
    }

    for (int i = 0; i < NUM_SHAPES; i++) {
        if (shapes[i].type == 1) {
            Circle *c = (Circle *)&shapes[i];
            total_area += circle_area(*c);
        } else if (shapes[i].type == 2) {
            Rectangle *r = (Rectangle *)&shapes[i];
            total_area += rect_area(*r);
        } else {
            Triangle *tr = (Triangle *)&shapes[i];
            total_area += triangle_area(*tr);
        }
    }

    printf("CppOOPShapes: Count=%d TotalArea=%.2f\n", NUM_SHAPES, total_area);
    return 0;
}
