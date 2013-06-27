/* bmp_struct_image.h 
 * Nicholas Donaldson
 * u5350448
 *
 * Declarations of functions for dealing
 * with 24bpp bitmap files and structures
 * defined in image_data_types.h
 */

#ifndef BMP_STRUCT_IMAGE_H
#define BMP_STRUCT_IMAGE_H

#include "image_data_types.h"

void bmp_to_struct_image(int input_fildes, struct image *img);
void struct_image_to_bmp(int output_fildes, struct image *img);

void get_dimensions_from_bmp(int *width, int *height, int input_fildes);
void get_pixel_array_from_bmp_malloc(struct image *raw_image, int input_fildes);

void write_bmp_header_to_file(int fildes, struct image *img);
void write_pixel_array_to_bmp(int fildes, struct image *img);

#endif
