#include "demo.h"

#include <stddef.h>
#include <stdint.h>

uint32_t WIDTH  = 0;
uint32_t HEIGHT = 0;

#define VERTEX_COUNT 20
#define EDGE_COUNT   30
/*
static const float vertices[VERTEX_COUNT][3] = {
    { 0.0f,  1.0f, -1.0f},
    { 0.5f,  0.0f, -1.0f},
    {-0.5f,  0.0f, -1.0f},
    {-2.0f, -2.0f, -1.0f},
    { 0.0f,  3.0f, -1.0f},
    { 2.0f, -2.0f, -1.0f},
    { 0.5f, -1.0f, -1.0f},
    {-0.5f, -1.0f, -1.0f}, 
    {-1.0f, -2.0f, -1.0f}, 
    { 1.0f, -2.0f, -1.0f},

    { 0.0f,  1.0f,  1.0f}, 
    { 0.5f,  0.0f,  1.0f}, 
    {-0.5f,  0.0f,  1.0f},
    {-2.0f, -2.0f,  1.0f},
    { 0.0f,  3.0f,  1.0f},
    { 2.0f, -2.0f,  1.0f},
    { 0.5f, -1.0f,  1.0f},
    {-0.5f, -1.0f,  1.0f},
    {-1.0f, -2.0f,  1.0f},
    { 1.0f, -2.0f,  1.0f},
};
*/

static const float vertices[VERTEX_COUNT][3] = {
    /* x,    y,    z */
    { 0.0f, -1.0f, -1.0f}, /*  0 */
    { 0.5f,  0.0f, -1.0f}, /*  1 */
    {-0.5f,  0.0f, -1.0f}, /*  2 */
    {-2.0f,  2.0f, -1.0f}, /*  3 */
    { 0.0f, -3.0f, -1.0f}, /*  4 */
    { 2.0f,  2.0f, -1.0f}, /*  5 */
    { 0.5f,  1.0f, -1.0f}, /*  6 */
    {-0.5f,  1.0f, -1.0f}, /*  7 */
    {-1.0f,  2.0f, -1.0f}, /*  8 */
    { 1.0f,  2.0f, -1.0f}, /*  9 */

    { 0.0f, -1.0f,  1.0f}, /* 10 */
    { 0.5f,  0.0f,  1.0f}, /* 11 */
    {-0.5f,  0.0f,  1.0f}, /* 12 */
    {-2.0f,  2.0f,  1.0f}, /* 13 */
    { 0.0f, -3.0f,  1.0f}, /* 14 */
    { 2.0f,  2.0f,  1.0f}, /* 15 */
    { 0.5f,  1.0f,  1.0f}, /* 16 */
    {-0.5f,  1.0f,  1.0f}, /* 17 */
    {-1.0f,  2.0f,  1.0f}, /* 18 */
    { 1.0f,  2.0f,  1.0f}, /* 19 */
};


/* Use integer vertex indices rather than storing indices as floats. */
static const uint8_t edges[EDGE_COUNT][2] = {
    { 2,  1}, { 2,  0}, { 1,  0},
    { 4,  3}, { 4,  5}, { 8,  3}, { 9,  5},
    { 9,  6}, { 7,  6}, { 7,  8},

    {12, 11}, {12, 10}, {11, 10},
    {14, 13}, {14, 15}, {18, 13}, {19, 15},
    {19, 16}, {17, 16}, {17, 18},

    { 0, 10}, { 1, 11}, { 2, 12}, { 3, 13},
    { 4, 14}, { 5, 15}, { 6, 16}, { 7, 17},
    { 8, 18}, { 9, 19},
};

/*
 * Builds Rz * Ry * Rx directly, avoiding two general 3x3 matrix
 * multiplications per frame.
 */
static inline void make_rotation(double ax, double ay, double az,
                                 double out[3][3])
{
    const double sx = sin(ax);
    const double cx = cos(ax);
    const double sy = sin(ay);
    const double cy = cos(ay);
    const double sz = sin(az);
    const double cz = cos(az);

    out[0][0] = cz * cy;
    out[0][1] = cz * sy * sx - sz * cx;
    out[0][2] = cz * sy * cx + sz * sx;

    out[1][0] = sz * cy;
    out[1][1] = sz * sy * sx + cz * cx;
    out[1][2] = sz * sy * cx - cz * sx;

    out[2][0] = -sy;
    out[2][1] = cy * sx;
    out[2][2] = cy * cx;
}

static inline void transform_vertex(const float vertex[3],
                                    const double matrix[3][3],
                                    double out[3])
{
    out[0] = matrix[0][0] * vertex[0]
           + matrix[0][1] * vertex[1]
           + matrix[0][2] * vertex[2];

    out[1] = matrix[1][0] * vertex[0]
           + matrix[1][1] * vertex[1]
           + matrix[1][2] * vertex[2];

    out[2] = matrix[2][0] * vertex[0]
           + matrix[2][1] * vertex[1]
           + matrix[2][2] * vertex[2];
}

/*
 * Projects one transformed vertex. The scale calculation deliberately
 * uses floating-point division rather than integer division.
 */
static inline int project_vertex(const double vertex[3],
                                 double distance,
                                 int *x_out,
                                 int *y_out)
{
    const double z = vertex[2] + distance;

    if (z <= 0.0)
        return 0;

    const double scale = 300.0 / z;

    *x_out = (int)(vertex[0] * scale + (double)WIDTH  * 0.5);
    *y_out = (int)(vertex[1] * scale + (double)HEIGHT * 0.5);

    return 1;
}

void start_demo_show(uint64_t fb_size)
{
    double angle_x = 0.0;
    double angle_y = 0.0;
    double angle_z = 0.0;

    WIDTH = framebuffer_pb->width;
    HEIGHT = framebuffer_pb->height;

    if (fb_size == 0)
        return;

    uint32_t *backbuffer_addr = kmalloc(fb_size);
    if (backbuffer_addr == NULL)
        return;

    uint32_t *background_addr = kmalloc(fb_size);
    if (background_addr == NULL) {
        kfree(backbuffer_addr);
        return;
    }

    framebuffer_t backbuffer = {
        .width   = framebuffer_pb->width,
        .height  = framebuffer_pb->height,
        .pitch   = framebuffer_pb->pitch,
        .address = backbuffer_addr,
    };

    /*
     * Save the original screen and initialize the backbuffer.
     * The original version copied the uninitialized backbuffer to
     * the screen here.
     */
    memcpy(background_addr, framebuffer_pb->address, fb_size);
    memcpy(backbuffer_addr, background_addr, fb_size);

    uint64_t report_start = get_milliseconds();
    uint64_t frames_since_report = 0;

    for (;;) {
        angle_x += 0.010;
        angle_y += 0.015;
        angle_z += 0.010;

        /* Keep angles bounded to avoid losing precision over long runs. */
        const double two_pi = 6.28318530717958647692;
        if (angle_x >= two_pi) angle_x -= two_pi;
        if (angle_y >= two_pi) angle_y -= two_pi;
        if (angle_z >= two_pi) angle_z -= two_pi;

        /*
         * Restore the background before drawing this frame, so lines
         * from the previous frame do not remain visible.
         */
        memcpy(backbuffer_addr, background_addr, fb_size);

        double rotation[3][3];
        make_rotation(angle_x, angle_y, angle_z, rotation);

        double transformed[VERTEX_COUNT][3];
        int screen_x[VERTEX_COUNT];
        int screen_y[VERTEX_COUNT];
        uint8_t visible[VERTEX_COUNT];

        for (size_t i = 0; i < VERTEX_COUNT; ++i) {
            transform_vertex(vertices[i], rotation, transformed[i]);
            visible[i] = (uint8_t)project_vertex(
                transformed[i], 5.0, &screen_x[i], &screen_y[i]
            );
        }

        for (size_t i = 0; i < EDGE_COUNT; ++i) {
            const uint8_t v1 = edges[i][0];
            const uint8_t v2 = edges[i][1];

            if (visible[v1] && visible[v2]) {
                draw_line_q(&backbuffer,
                            screen_x[v1], screen_y[v1],
                            screen_x[v2], screen_y[v2],
                            0x00DDDDDD);
            }
        }

        /* Present the completed frame. */
        memcpy(framebuffer_pb->address, backbuffer_addr, fb_size);

        ++frames_since_report;
        const uint64_t now = get_milliseconds();
        const uint64_t elapsed = now - report_start;

        /* Console output every second instead of every frame. */
        //const uint64_t fps = frames_since_report * 1000 / elapsed;
        print("FT: %u\r", (unsigned)elapsed);
        frames_since_report = 0;
        report_start = now;
    }

    /* The loop currently runs forever. */
    kfree(background_addr);
    kfree(backbuffer_addr);
}
