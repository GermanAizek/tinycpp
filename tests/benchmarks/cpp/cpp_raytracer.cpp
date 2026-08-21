#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef struct { double x, y, z; } Vec3;

static Vec3 vec_add(Vec3 a, Vec3 b) { return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z}; }
static Vec3 vec_sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }
static Vec3 vec_scale(Vec3 a, double s) { return (Vec3){a.x * s, a.y * s, a.z * s}; }
static double vec_dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
static Vec3 vec_norm(Vec3 a) {
    double len = sqrt(vec_dot(a, a));
    return (Vec3){a.x / len, a.y / len, a.z / len};
}

typedef struct {
    Vec3 center;
    double radius;
    Vec3 color;
    double reflection;
} Sphere;

typedef struct {
    Vec3 orig;
    Vec3 dir;
} Ray;

static int ray_sphere_intersect(Ray r, Sphere s, double *t) {
    Vec3 oc = vec_sub(r.orig, s.center);
    double b = 2.0 * vec_dot(oc, r.dir);
    double c = vec_dot(oc, oc) - s.radius * s.radius;
    double disc = b * b - 4.0 * c;
    if (disc < 0.0) return 0;
    double t0 = (-b - sqrt(disc)) * 0.5;
    double t1 = (-b + sqrt(disc)) * 0.5;
    if (t0 > 0.001) { *t = t0; return 1; }
    if (t1 > 0.001) { *t = t1; return 1; }
    return 0;
}

static Vec3 trace_ray(Ray r, Sphere *spheres, int n_spheres, Vec3 light_pos, int depth) {
    if (depth > 3) return (Vec3){0.05, 0.05, 0.1}; // Background color

    double nearest_t = 1e9;
    int hit_idx = -1;
    for (int i = 0; i < n_spheres; i++) {
        double t;
        if (ray_sphere_intersect(r, spheres[i], &t) && t < nearest_t) {
            nearest_t = t;
            hit_idx = i;
        }
    }

    if (hit_idx == -1) return (Vec3){0.05, 0.05, 0.1};

    Sphere hit_sp = spheres[hit_idx];
    Vec3 hit_pt = vec_add(r.orig, vec_scale(r.dir, nearest_t));
    Vec3 normal = vec_norm(vec_sub(hit_pt, hit_sp.center));
    Vec3 light_dir = vec_norm(vec_sub(light_pos, hit_pt));

    // Shadow ray
    Ray shadow_ray = { vec_add(hit_pt, vec_scale(normal, 0.001)), light_dir };
    int in_shadow = 0;
    for (int i = 0; i < n_spheres; i++) {
        double t;
        if (ray_sphere_intersect(shadow_ray, spheres[i], &t)) {
            in_shadow = 1;
            break;
        }
    }

    double diffuse = 0.1; // Ambient
    if (!in_shadow) {
        double d = vec_dot(normal, light_dir);
        if (d > 0.0) diffuse += d * 0.9;
    }

    Vec3 direct_color = vec_scale(hit_sp.color, diffuse);

    if (hit_sp.reflection > 0.0) {
        Vec3 refl_dir = vec_sub(r.dir, vec_scale(normal, 2.0 * vec_dot(r.dir, normal)));
        Ray refl_ray = { vec_add(hit_pt, vec_scale(normal, 0.001)), refl_dir };
        Vec3 refl_color = trace_ray(refl_ray, spheres, n_spheres, light_pos, depth + 1);
        return vec_add(vec_scale(direct_color, 1.0 - hit_sp.reflection), vec_scale(refl_color, hit_sp.reflection));
    }

    return direct_color;
}

#define W 200
#define H 200

int main(void) {
    Sphere spheres[4] = {
        { { 0.0, -1.0, 3.0 }, 1.0, { 1.0, 0.2, 0.2 }, 0.2 },
        { { 2.0, 0.0, 4.0 },  1.0, { 0.2, 0.8, 0.2 }, 0.4 },
        { { -2.0, 0.0, 4.0 }, 1.0, { 0.2, 0.2, 1.0 }, 0.4 },
        { { 0.0, -5001.0, 0.0 }, 5000.0, { 0.8, 0.8, 0.8 }, 0.1 } // Floor
    };
    Vec3 light_pos = { 0.0, 5.0, -1.0 };
    Vec3 camera = { 0.0, 0.0, -2.0 };

    double r_sum = 0.0, g_sum = 0.0, b_sum = 0.0;

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
            double u = (x - W * 0.5) / (double)W;
            double v = -(y - H * 0.5) / (double)H;
            Vec3 dir = vec_norm((Vec3){ u, v, 1.0 });
            Ray r = { camera, dir };
            Vec3 c = trace_ray(r, spheres, 4, light_pos, 0);
            r_sum += c.x;
            g_sum += c.y;
            b_sum += c.z;
        }
    }

    printf("Raytracer: %dx%d Checksum=%.3f\n", W, H, r_sum + g_sum + b_sum);
    return 0;
}
