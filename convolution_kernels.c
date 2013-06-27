/* convolution_kernels.c
 * Nicholas Donaldson
 * u5350448
 *
 * Functions to deal with
 * convolution kernels and images
 *
 */

#include "convolution_kernels.h"
#include "image_data_helper_functions.h"
#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>

// Calculates the gaussian function at distance with given standard_deviation,
// in this file for gaussian kernel use
double gaussian_function(double distance, double standard_deviation) {
    return (1/(sqrt(2.0*M_PI)*standard_deviation))*(pow(M_E,(-(pow(distance,2.0))/(2.0*(pow(standard_deviation,2.0))))));
}

// Computes the euclidean distance between two points,
// in this file for gaussian kernel use
double euclidean_distance(double x1, double y1, double x2, double y2) {
    return sqrt(pow(y2-y1,2.0) + pow(x2-x1,2.0));
}

void generate_gaussian_kernel(double matrix[5][5], double standard_deviation) {
    int x,y;
    for (y = -2; y < 3; y++) {
        for (x = -2; x < 3; x++) {
            matrix[y+2][x+2] = gaussian_function(euclidean_distance(x,y, 0.0, 0.0), standard_deviation);
        }
    }
    normalise_kernel(matrix);
}

// Divides each element in the kernel by the sum
// of everything in the kernel. Without this,
// filters based on convolution matrices
// would increase or decrease the brightness
// of the image
void normalise_kernel(double kernel[5][5]) {
    double kernel_sum = 0.0;
    int x,y;
    for (y = 0; y < 5; y++) {
        for (x = 0; x < 5; x++) {
            kernel_sum += kernel[y][x];
        }
    }
    if (kernel_sum == 0.0) kernel_sum =1.0;
    for (y = 0; y < 5; y++) {
        for (x = 0; x < 5; x++) {
            kernel[y][x] = kernel[y][x]/kernel_sum;
        }
    }
}

void apply_kernel_to_x_y(int x,int y, double kernel[5][5], struct image *img , struct pixel *pix) {
    double red_sum = 0.0;
    double green_sum = 0.0;
    double blue_sum = 0.0;

    struct pixel *img_pixel; 

    int x_dif;
    int y_dif;
    for (y_dif = -2; y_dif < 3; y_dif++) {
        for (x_dif = -2; x_dif < 3; x_dif++) {
            img_pixel = get_nearest_pixel(x + x_dif, y + y_dif, img);
            red_sum += img_pixel->Red*kernel[y_dif+2][x_dif+2];
            green_sum += img_pixel->Green*kernel[y_dif+2][x_dif+2];
            blue_sum += img_pixel->Blue*kernel[y_dif+2][x_dif+2];
        }
    }

    // Clamp the values
    red_sum = fmin(255.0, fmax(red_sum, 0.0));
    green_sum = fmin(255.0, fmax(green_sum, 0.0));
    blue_sum = fmin(255.0, fmax(blue_sum, 0.0));

    pix->Red = (int)red_sum;
    pix->Green = (int)green_sum;
    pix->Blue = (int)blue_sum;

}

void apply_kernel_to_struct_image(double kernel[5][5], struct image *img) {
    struct pixel *pix;
    struct image blurred_image;
    blurred_image.width = img->width;
    blurred_image.height = img->height;
    blurred_image.n_of_pixels = img->n_of_pixels;
    blurred_image.pixel_array = malloc(blurred_image.n_of_pixels*sizeof(struct pixel));

    if (blurred_image.pixel_array == NULL) {
        int errsv = errno;
        error(1, errsv, "Failed to allocate memory for the dummy image pixel array\n");
    }

    int x,y;
    
    for (y = 0; y < img->height; y++) {
        for (x = 0; x < img->width; x++) {
            pix = get_pixel_pointer_from_struct_image_x_y(x, y, &blurred_image);
            apply_kernel_to_x_y(x,y,kernel, img , pix);
        }
    }

    free(img->pixel_array);
    img->pixel_array = blurred_image.pixel_array;
}
