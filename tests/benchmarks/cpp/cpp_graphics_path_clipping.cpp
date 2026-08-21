#include <stdio.h>
#include <math.h>

typedef struct { float x, y; } Point;

static int clip_polygon(const Point *in, int n_in, Point *out, float x_min, float x_max, float y_min, float y_max) {
    Point temp[64];
    int n_temp = 0;
    // Left edge
    for (int i = 0; i < n_in; i++) {
        Point p1 = in[i], p2 = in[(i + 1) % n_in];
        if (p1.x >= x_min && p2.x >= x_min) temp[n_temp++] = p2;
        else if (p1.x < x_min && p2.x >= x_min) {
            temp[n_temp++] = (Point){x_min, p1.y + (p2.y - p1.y)*(x_min - p1.x)/(p2.x - p1.x)};
            temp[n_temp++] = p2;
        } else if (p1.x >= x_min && p2.x < x_min) {
            temp[n_temp++] = (Point){x_min, p1.y + (p2.y - p1.y)*(x_min - p1.x)/(p2.x - p1.x)};
        }
    }
    for (int i = 0; i < n_temp; i++) out[i] = temp[i];
    return n_temp;
}

int main(void) {
    Point poly[4] = {{-10, -10}, {50, -5}, {40, 60}, {-5, 40}};
    Point clipped[64];
    float total_area = 0.0f;
    for (int i = 0; i < 20000; i++) {
        poly[0].x = (float)(i % 20 - 10);
        int n_out = clip_polygon(poly, 4, clipped, 0.0f, 30.0f, 0.0f, 30.0f);
        float area = 0.0f;
        for (int j = 0; j < n_out; j++) {
            int k = (j + 1) % n_out;
            area += clipped[j].x * clipped[k].y - clipped[k].x * clipped[j].y;
        }
        total_area += fabsf(area * 0.5f);
    }
    printf("GraphicsPathClipping: TotalArea=%.2f\n", total_area);
    return 0;
}