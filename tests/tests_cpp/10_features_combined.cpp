#include <stdio.h>

extern "C" {
    int native_sum(int a, int b) {
        return a + b;
    }
}

namespace Geometry {
    class Shape {
    public:
        int width;
        int height;
    };

    class ColoredRectangle : public Shape {
    public:
        int color_code;
    };
}

void describe_rect(const Geometry::ColoredRectangle &rect) {
    int area = rect.width * rect.height;
    printf("Rectangle: %dx%d, area=%d, color=0x%x\n", rect.width, rect.height, area, rect.color_code);
}

void scale_shape(Geometry::Shape &s, int factor = 2) {
    s.width = s.width * factor;
    s.height = s.height * factor;
}

int main() {
    bool is_active = true;
    if (is_active) {
        printf("Active status: %d\n", is_active);
    }

    void *ptr = nullptr;
    if (ptr == nullptr) {
        printf("Pointer is nullptr\n");
    }

    Geometry::ColoredRectangle *rect = new Geometry::ColoredRectangle;
    rect->width = 10;
    rect->height = 20;
    rect->color_code = 0xFF00AA;
    describe_rect(*rect);

    scale_shape(*rect);
    describe_rect(*rect);

    scale_shape(*rect, 3);
    describe_rect(*rect);

    delete rect;

    double fval = 42.99;
    int ival = static_cast<int>(fval);
    printf("Cast result: %d\n", ival);

    int sum = native_sum(100, 250);
    printf("Native sum: %d\n", sum);

    return 0;
}
