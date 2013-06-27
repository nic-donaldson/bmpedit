/* image_data_types.h 
 * Nicholas Donaldson
 * u5350448
 *
 * Definition of struct types that are
 * central to bmpedit
 * 
 */

#ifndef IMG_DATA_TYPES_H
#define IMG_DATA_TYPES_H

#include <inttypes.h>

struct pixel {
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;
};

// Generic image structure
// 24bpp

struct image {
    int width;
    int height;
    int n_of_pixels;
    uint32_t pixel_array_byte_size;
    struct pixel *pixel_array;
};
#endif
