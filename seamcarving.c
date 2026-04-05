#include "seamcarving.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

void calc_energy(struct rgb_img *im, struct rgb_img **grad) {
    int rx, ry, gx, gy, bx, by;
    int left, right, up, down;

    create_img(grad, im->height, im->width);

    for (int y = 0; y < im->height; y++) {
        for (int x = 0; x < im->width; x++) {

            if (x == 0) {
                left = im->width - 1;
            } else {
                left = x - 1;
            }

            if (x == im->width - 1) {
                right = 0;
            } else {
                right = x + 1;
            }

            if (y == 0) {
                up = im->height - 1;
            } else {
                up = y - 1;
            }

            if (y == im->height - 1) {
                down = 0;
            } else {
                down = y + 1;
            }

            rx = get_pixel(im, y, right, 0) - get_pixel(im, y, left, 0);
            ry = get_pixel(im, down, x, 0) - get_pixel(im, up, x, 0);

            gx = get_pixel(im, y, right, 1) - get_pixel(im, y, left, 1);
            gy = get_pixel(im, down, x, 1) - get_pixel(im, up, x, 1);

            bx = get_pixel(im, y, right, 2) - get_pixel(im, y, left, 2);
            by = get_pixel(im, down, x, 2) - get_pixel(im, up, x, 2);

            uint8_t energy = (uint8_t)(sqrt(
                rx * rx + ry * ry +
                gx * gx + gy * gy +
                bx * bx + by * by
            ) / 10.0);

            set_pixel(*grad, y, x, energy, energy, energy);
        }
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr) {
    *best_arr = (double *)malloc(sizeof(double) * grad->height * grad->width);

    for (int y = 0; y < grad->height; y++) {
        for (int x = 0; x < grad->width; x++) {

            double energy = get_pixel(grad, y, x, 0);

            if (y == 0) {
                (*best_arr)[y * grad->width + x] = energy;
            } else {
                int left, right;

                if (x == 0) {
                    left = 0;
                } else {
                    left = x - 1;
                }

                if (x == grad->width - 1) {
                    right = grad->width - 1;
                } else {
                    right = x + 1;
                }

                double min_prev = (*best_arr)[(y - 1) * grad->width + left];

                for (int k = left; k <= right; k++) {
                    double val = (*best_arr)[(y - 1) * grad->width + k];
                    if (val < min_prev) {
                        min_prev = val;
                    }
                }

                (*best_arr)[y * grad->width + x] = energy + min_prev;
            }
        }
    }
}


void recover_path(double *best, int height, int width, int **path) {

    *path = (int *)malloc(sizeof(int) * height);

    int min_col = 0;
    double min_val = best[(height - 1) * width];

    for (int x = 1; x < width; x++) {
        if (best[(height - 1) * width + x] < min_val) {
            min_val = best[(height - 1) * width + x];
            min_col = x;
        }
    }

    (*path)[height - 1] = min_col;

    for (int y = height - 2; y >= 0; y--) {

        int cur_x = (*path)[y + 1];

        int left, right;

        if (cur_x == 0) {
            left = 0;
        } else {
            left = cur_x - 1;
        }

        if (cur_x == width - 1) {
            right = width - 1;
        } else {
            right = cur_x + 1;
        }

        int best_x = left;
        double best_val = best[y * width + left];

        if (cur_x >= left && cur_x <= right) {
            double val = best[y * width + cur_x];
            if (val < best_val) {
                best_val = val;
                best_x = cur_x;
            }
        }

        if (right != cur_x) {
            double val = best[y * width + right];
            if (val < best_val) {
                best_val = val;
                best_x = right;
            }
        }

        (*path)[y] = best_x;
    }
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path) {

    create_img(dest, src->height, src->width - 1);

    for (int y = 0; y < src->height; y++) {
        int new_x = 0;

        for (int x = 0; x < src->width; x++) {
            if (x != path[y]) {
                set_pixel(*dest, y, new_x,
                          get_pixel(src, y, x, 0),
                          get_pixel(src, y, x, 1),
                          get_pixel(src, y, x, 2));
                new_x++;
            }
        }
    }
}