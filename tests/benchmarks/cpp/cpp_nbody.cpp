#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.141592653589793
#define SOLAR_MASS (4 * PI * PI)
#define DAYS_PER_YEAR 365.24

struct Body {
    double x, y, z;
    double vx, vy, vz;
    double mass;
};

static void advance(Body *bodies, int n, double dt) {
    for (int i = 0; i < n; i++) {
        Body &b1 = bodies[i];
        for (int j = i + 1; j < n; j++) {
            Body &b2 = bodies[j];
            double dx = b1.x - b2.x;
            double dy = b1.y - b2.y;
            double dz = b1.z - b2.z;

            double d2 = dx * dx + dy * dy + dz * dz;
            double dist = sqrt(d2);
            double mag = dt / (d2 * dist);

            b1.vx -= dx * b2.mass * mag;
            b1.vy -= dy * b2.mass * mag;
            b1.vz -= dz * b2.mass * mag;

            b2.vx += dx * b1.mass * mag;
            b2.vy += dy * b1.mass * mag;
            b2.vz += dz * b1.mass * mag;
        }
    }

    for (int i = 0; i < n; i++) {
        Body &b = bodies[i];
        b.x += dt * b.vx;
        b.y += dt * b.vy;
        b.z += dt * b.vz;
    }
}

static double compute_energy(const Body *bodies, int n) {
    double e = 0.0;
    for (int i = 0; i < n; i++) {
        const Body &b1 = bodies[i];
        e += 0.5 * b1.mass * (b1.vx * b1.vx + b1.vy * b1.vy + b1.vz * b1.vz);
        for (int j = i + 1; j < n; j++) {
            const Body &b2 = bodies[j];
            double dx = b1.x - b2.x;
            double dy = b1.y - b2.y;
            double dz = b1.z - b2.z;
            double dist = sqrt(dx * dx + dy * dy + dz * dz);
            e -= (b1.mass * b2.mass) / dist;
        }
    }
    return e;
}

int main(void) {
    Body bodies[5] = {
        { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, SOLAR_MASS },
        { 4.84143144246472090e+00, -1.16032004402742839e+00, -1.03622044471123109e-01,
          1.66007664274403694e-03 * DAYS_PER_YEAR, 7.69901118419740425e-03 * DAYS_PER_YEAR, -6.90460016972063023e-05 * DAYS_PER_YEAR,
          9.54791938424326609e-04 * SOLAR_MASS },
        { 8.34336671824457987e+00, 4.12479856412430479e+00, -4.03523417114321381e-01,
          -2.76742510726862411e-03 * DAYS_PER_YEAR, 4.99852801234917238e-03 * DAYS_PER_YEAR, 2.30417297573763929e-05 * DAYS_PER_YEAR,
          2.85885670658354910e-04 * SOLAR_MASS },
        { 1.28943695621391310e+01, -1.51111514016986312e+01, -2.23307578892655734e-01,
          2.96460137564761618e-03 * DAYS_PER_YEAR, 2.37847173976563638e-03 * DAYS_PER_YEAR, -2.96589568540237556e-05 * DAYS_PER_YEAR,
          4.36624404335156298e-05 * SOLAR_MASS },
        { 1.53796971148509165e+01, -2.59193146099879641e+01, 1.79258772950371181e-01,
          2.68067772490389322e-03 * DAYS_PER_YEAR, 1.62824170038242295e-03 * DAYS_PER_YEAR, -9.51592254519715870e-05 * DAYS_PER_YEAR,
          5.15138902046611451e-05 * SOLAR_MASS }
    };

    double px = 0.0, py = 0.0, pz = 0.0;
    for (int i = 0; i < 5; i++) {
        px += bodies[i].vx * bodies[i].mass;
        py += bodies[i].vy * bodies[i].mass;
        pz += bodies[i].vz * bodies[i].mass;
    }
    bodies[0].vx = -px / SOLAR_MASS;
    bodies[0].vy = -py / SOLAR_MASS;
    bodies[0].vz = -pz / SOLAR_MASS;

    double e_init = compute_energy(bodies, 5);

    int steps = 1500000;
    for (int step = 0; step < steps; step++) {
        advance(bodies, 5, 0.01);
    }

    double e_final = compute_energy(bodies, 5);

    printf("CPP_NBody: Init=%.6f Final=%.6f Delta=%.6f\n", e_init, e_final, fabs(e_final - e_init));
    return 0;
}
