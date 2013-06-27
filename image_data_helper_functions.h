/* image_data_helper_functions.h 
 * Nicholas Donaldson
 * u5350448
 *
 * Declaration of functions to help deal
 * with types defined in image_data_types.h
 *
 */


#ifndef IMAGE_DATA_HELPER_H
#define IMAGE_DATA_HELPER_H

#include "image_data_types.h"

int min(int x, int y);

struct pixel *get_pixel_pointer_from_struct_image_x_y(int x, int y, struct image *img);

void set_pixel_in_struct_image_x_y(int x, int y, struct image *img, struct pixel *pix);

void print_pixel(struct pixel *ptr_pixel);

struct pixel *get_nearest_pixel(int x, int y, struct image *img);

void add_two_pixels(struct pixel *pix1, struct pixel *pix2);

void add_two_images(struct image *img_1, struct image *img_2);

#endif
