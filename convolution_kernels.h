/* convolution_kernels.h
 * Nicholas Donaldson
 * u5350448
 *
 * Declaration of functions to deal with
 * convolution kernels and images
 *
 */

#ifndef CONVOLUTION_KERNELS_H
#define CONVOLUTION_KERNELS_H

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E  2.71828182845904523536
#endif

#include "image_data_types.h"

double gaussian_function(double distance, double standard_deviation);
double euclidean_distance(double x1, double y1, double x2, double y2);
void generate_gaussian_kernel(double matrix[5][5], double standard_deviation);

void normalise_kernel(double kernel[5][5]);

void apply_kernel_to_x_y(int x,int y, double kernel[5][5], struct image *img , struct pixel *pix);
void apply_kernel_to_struct_image(double kernel[5][5], struct image *img);

#endif
