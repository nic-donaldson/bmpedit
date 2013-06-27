/* filters.h
 * Nicholas Donaldson
 * u5350448
 *
 * Declaration of standard filter functions
 * implemented on the image structures
 * defined in image_data_types.h
 *
 */

#ifndef FILTERS_H
#define FILTERS_H

#include "image_data_types.h"

void threshold_pixel(double threshold_value, struct pixel *ptr_pixel);
void threshold_image(double threshold_value, struct image *img);

void invert_pixel(struct pixel *ptr_pixel);
void invert_image(struct image *img);

void blend_two_pixels(double blend_coefficient, struct pixel *pixel_1, struct pixel *pixel_2);
int blend_two_images(double blend_coefficient, struct image *img_1, struct image *img_2);

void crop_image (int x1, int y1, int x2, int y2, struct image *img);
void parse_crop_arg(int *x1, int *y1, int *x2, int *y2,char *crop_arg);

void set_brightness_pixel(double brightness_percentage_increase, struct pixel *pix);
void brightness_image(double brightness_percentage_change, struct image *img);

void emboss_image (struct image *img);

void sharpen_image(double sharpen_value, struct image *img);

void sobel_edge_detect_image(struct image *img);

void greyscale_pixel(struct pixel *pix);
void greyscale_image(struct image *img);

void sharpen_image(double sharpen_value, struct image *img);

void gaussian_blur(int repeat, double standard_deviation, struct image *img);
void parse_gaussian_arg(int *repeat, double *standard_deviation, char *gaussian_arg);

#endif
