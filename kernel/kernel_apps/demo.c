#include "demo.h"

//DEFINITIONS

uint32_t WIDTH  = 0;
uint32_t HEIGHT = 0;

// DATA
/*
int verticies[8][3] = {
    {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
    {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
};

int edges[12][2] = {
    {0, 1}, {1, 2}, {2, 3}, {3, 0},
    {4, 5}, {5, 6}, {6, 7}, {7, 4},
    {0, 4}, {1, 5}, {2, 6}, {3, 7}
};

*/
float verticies[20][3] = {
    // x y z
    {0, 1, -1}, // 0
    {0.5, 0, -1}, // 1
    {-0.5, 0, -1}, // 2
    {-2, -2, -1}, // 3
    {0, 3, -1}, // 4
    {2, -2, -1}, // 5
    {0.5, -1, -1}, // 6
    {-0.5, -1, -1}, // 7
    {-1, -2, -1}, // 8
    {1, -2, -1}, // 9
    {0, 1, 1}, // 10
    {0.5, 0, 1}, // 11
    {-0.5, 0, 1}, // 12
    {-2, -2, 1}, // 13
    {0, 3, 1}, // 14
    {2, -2, 1}, // 15
    {0.5, -1, 1}, // 16
    {-0.5, -1, 1}, // 17
    {-1, -2, 1}, // 18
    {1, -2, 1}, // 19
};

float edges[30][2] = {
    {2, 1},
    {2, 0},
    {1, 0},
    {4, 3},
    {4, 5},
    {8, 3},
    {9, 5},
    {9, 6},
    {7, 6},
    {7, 8},
    {12, 11},
    {12, 10},
    {11, 10},
    {14, 13},
    {14, 15},
    {18, 13},
    {19, 15},
    {19, 16},
    {17, 16},
    {17, 18},
    {0, 10},
    {1, 11},
    {2, 12},
    {3, 13},
    {4, 14},
    {5, 15},
    {6, 16},
    {7, 17},
    {8, 18},
    {9, 19},
};

// TRANSFORMATORS

static inline void combine_rotations(double rot_x[3][3], double rot_y[3][3], double rot_z[3][3], double out[3][3]) {
    double temp[3][3];
    
    // rot_z * rot_y
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            temp[i][j] = 0;
            for (int k = 0; k < 3; k++) {
                temp[i][j] += rot_z[i][k] * rot_y[k][j];
            }
        }
    }
    
    // (rot_z * rot_y) * rot_x
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            out[i][j] = 0;
            for (int k = 0; k < 3; k++) {
                out[i][j] += temp[i][k] * rot_x[k][j];
            }
        }
    }
}

static inline void transform_vertex(double vertex[3], double matrix[3][3], double out[3]) {
    out[0] = matrix[0][0] * vertex[0] + matrix[0][1] * vertex[1] + matrix[0][2] * vertex[2];
    out[1] = matrix[1][0] * vertex[0] + matrix[1][1] * vertex[1] + matrix[1][2] * vertex[2];
    out[2] = matrix[2][0] * vertex[0] + matrix[2][1] * vertex[1] + matrix[2][2] * vertex[2];
}

static inline void rotation_x(double angle, double out[3][3]) {
    double c = cos(angle);
    double s = sin(angle);
    out[0][0] = 1; out[0][1] = 0;  out[0][2] = 0;
    out[1][0] = 0; out[1][1] = c;  out[1][2] = -s;
    out[2][0] = 0; out[2][1] = s;  out[2][2] = c;
}

static inline void rotation_y(double angle, double out[3][3]) {
    double c = cos(angle);
    double s = sin(angle);
    out[0][0] = c;  out[0][1] = 0; out[0][2] = s;
    out[1][0] = 0;  out[1][1] = 1; out[1][2] = 0;
    out[2][0] = -s; out[2][1] = 0; out[2][2] = c;
}

static inline void rotation_z(double angle, double out[3][3]) {
    double c = cos(angle);
    double s = sin(angle);
    out[0][0] = c;  out[0][1] = -s; out[0][2] = 0;
    out[1][0] = s;  out[1][1] = c;  out[1][2] = 0;
    out[2][0] = 0;  out[2][1] = 0;  out[2][2] = 1;
}

// Display Function
static inline int display(double vertex[], double distance, int *x_out, int *y_out) {
    double z = vertex[2] + distance;
    if (z <= 0)
        return 0;

    int scale = 300 / z;
    *x_out = (int)(vertex[0] * scale + WIDTH / 2);
    *y_out = (int)(vertex[1] * scale + HEIGHT / 2);
    return 1;
}

// MAIN function
void start_demo_show(uint64_t fb_size) {
    double angle_x = 0;
    double angle_y = 0;
    double angle_z = 0;

    WIDTH = framebuffer_pb->width;
    HEIGHT = framebuffer_pb->height;

    // backbuffer
    uint64_t framebuffer_size_b = fb_size; // framebuffer_pb->pitch*framebuffer_pb->width*4
    uint32_t* backbuffer_addr = kmalloc(framebuffer_size_b);
    uint32_t* background_addr = kmalloc(framebuffer_size_b);

    if (backbuffer_addr == 0) {
        return;
    }

    if (background_addr == 0) {
        return;
    }

    framebuffer_t backbuffer = {
        .width = framebuffer_pb->width,
        .height = framebuffer_pb->height,
        .pitch = framebuffer_pb->pitch,
        .address = backbuffer_addr,
    };

    print("Backbuffer size     : %lx \n", framebuffer_size_b);
    print("Backbuffer addr base: %lx \n", backbuffer_addr);
    print("Backbuffer addr end : %lx \n", (uint64_t)backbuffer_addr + framebuffer_size_b);

    memcpy(
        background_addr,
        framebuffer_pb->address,
        framebuffer_size_b
    );

    memcpy(
        framebuffer_pb->address,
        backbuffer_addr,
        framebuffer_size_b
    );
    uint64_t iterations = 0;
    for (;;) {
        print("Iteration: %n \r", iterations);
        angle_x += 0.01;
        angle_y += 0.015;
        angle_z += 0.005;
        
        //fill_rect_q(&backbuffer, 1,1, backbuffer.width,backbuffer.height, 0x00ffffff);

        memcpy(backbuffer_addr, background_addr, fb_size);

        double rot_x[3][3], rot_y[3][3], rot_z[3][3];
        rotation_x(angle_x, rot_x);
        rotation_y(angle_y, rot_y);
        rotation_z(angle_z, rot_z);

        double combined[3][3];
        combine_rotations(rot_x, rot_y, rot_z, combined);

        double transformed[sizeof(verticies) / sizeof(verticies[0])][3];
        for (int i = 0; i < sizeof(verticies) / sizeof(verticies[0]); i++) {
            double vertex[3] = {
                (double)verticies[i][0],
                (double)verticies[i][1],
                (double)verticies[i][2]
            };
            transform_vertex(vertex, combined, transformed[i]);
        }

        for (int i = 0; i < sizeof(edges) / sizeof(edges[0]); i++) {
            int v1 = edges[i][0];
            int v2 = edges[i][1];

            int x1, y1, x2, y2;
            if (display(transformed[v1], 5.0, &x1, &y1) &&
                display(transformed[v2], 5.0, &x2, &y2)) {
                draw_line_q(&backbuffer, x1,y1, x2,y2, 0x00DDDDDD); // 
            }
        }

        // int memcmp(const void *a, const void *b, size_t n)
        
        memcpy(framebuffer_pb->address, backbuffer_addr, framebuffer_size_b);
        // memcpy to framebuffer
        ++iterations;
    }

    kfree(backbuffer_addr);
}
