#include "seamcarving.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "HJoceanSmall.bin");

    int seams_to_remove = im->width / 2;

    for (int i = 0; i < seams_to_remove; i++) {
        printf("i = %d\n", i);
        calc_energy(im, &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);

        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);

        im = cur_im;
    }

    write_img(im, "halfwidth.bin");
    destroy_image(im);

    return 0;
}