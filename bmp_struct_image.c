/* bmp_struct_image.c 
 * Nicholas Donaldson
 * u5350448
 *
 * Functions for dealing
 * with 24bpp bitmap files and structures
 * defined in image_data_types.h
 */

#include "bmp_struct_image.h"
#include <unistd.h>
#include <inttypes.h>
#include <stdlib.h>
#include <error.h>
#include <errno.h>
#include <math.h>
#include <string.h>

void bmp_to_struct_image(int input_fildes, struct image *img) {
    // Check the first two characters are "BM"
    char buf[3];
    if (pread(input_fildes, buf, 2, 0x00) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed read");
    } 
    buf[2] = '\0';

    if (strcmp(buf, "BM") != 0) {
        error(1, 0, "Input file is not a supported bitmap");
    }

    get_dimensions_from_bmp(&img->width, &img->height, input_fildes);
    get_pixel_array_from_bmp_malloc(img, input_fildes);
}

void struct_image_to_bmp(int output_fildes, struct image *img) {
    write_bmp_header_to_file(output_fildes, img);
    write_pixel_array_to_bmp(output_fildes, img);
}

void get_dimensions_from_bmp(int *width, int *height, int input_fildes) {
    if (pread(input_fildes, width, 4, 0x12) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed read");
    }
    if (pread(input_fildes, height, 4, 0x16) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed read");
    }
}

void get_pixel_array_from_bmp_malloc(struct image *raw_image, int input_fildes) {
    // Get pixel array offset in bmp
    int pixel_array_offset;
    if (pread(input_fildes, &pixel_array_offset, 4, 0xA) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed read");
    }

    // Get pixel data size
    uint32_t pixel_array_size;
    if (pread(input_fildes, &pixel_array_size, 4, 0x22) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed read");
    }

    int n_of_pixels = raw_image->width*raw_image->height;

    struct pixel *pixel_array = malloc(n_of_pixels * sizeof(struct pixel));
    if (pixel_array == NULL) {
        int errsv = errno;
        error(1, errsv, "Couldn't allocate memory for pixel array\n");
    }

    // Read in bitmap data
    uint8_t *img_buf = malloc(pixel_array_size);
    if (img_buf == NULL) {
        int errsv = errno;
        error(1, errsv, "Couldn't allocate memory for image buffer\n");
    }

    if (pread(input_fildes, img_buf, pixel_array_size, pixel_array_offset) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed read");
    }

    // Calculate row width
    // http://en.wikipedia.org/wiki/BMP_file_format
    
    int row_width = (int)(floor((24.0*((double)raw_image->width) + 31.0)/32.0)*4.0);
    int bytes_to_skip = row_width - raw_image->width*3;
    int row_index;
    int col_index;

    // Get row by row backwards
    
    uint8_t *img_buf_backwards_pointer = img_buf + pixel_array_size - 1;
    for (row_index = 0; row_index < raw_image->height; row_index++) {
        img_buf_backwards_pointer -= bytes_to_skip;
        for (col_index = 0; col_index < raw_image->width; ++col_index, img_buf_backwards_pointer-=3) {
            pixel_array[row_index*raw_image->width + col_index].Red = *img_buf_backwards_pointer;
            pixel_array[row_index*raw_image->width + col_index].Green = *(img_buf_backwards_pointer-1);
            pixel_array[row_index*raw_image->width + col_index].Blue = *(img_buf_backwards_pointer-2);
        }
    }

    // Set the image
    raw_image->pixel_array = pixel_array;
    raw_image->pixel_array_byte_size = pixel_array_size;
    raw_image->n_of_pixels = n_of_pixels;

    // Free the buffer
    free(img_buf);
}

void write_bmp_header_to_file(int fildes, struct image *img) {
    // Write BM
    if (write(fildes, "BM", 2) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // Write file size (unsigned)
    uint32_t file_size = 0x36 + img->pixel_array_byte_size;
    if (write(fildes, (char*)&file_size, 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // Write signature
    if (write(fildes, "NICD", 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // Write pixel array offset
    uint32_t pixel_array_offset = 0x36;
    if (write(fildes, (char*)&pixel_array_offset, 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // Write header size
    uint32_t header_size = 40;
    if (write(fildes, (char*)&header_size, 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // Write dimensions
    if (write(fildes, (char*)&img->width, 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }
    if (write(fildes, (char*)&img->height, 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // Number of colour planes
    uint16_t n_of_colour_planes = 1;
    if (write(fildes, (char*)&n_of_colour_planes, 2) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // bpp
    uint16_t bpp = 24;
    if (write(fildes, (char*)&bpp, 2) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // compression method
    if (write(fildes, "\0\0\0\0", 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // raw bitmap data size
    if (write(fildes, (char*)&img->pixel_array_byte_size, 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // image resolution (signed)
    int32_t img_res = 2880;
    if (write(fildes, (char*)&img_res, 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }
    if (write(fildes, (char*)&img_res, 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // colours in palette
    if (write(fildes, "\0\0\0\0", 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // import colours in palette
    if (write(fildes, "\0\0\0\0", 4) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }
}

void write_pixel_array_to_bmp(int fildes, struct image *img)  {

    // Write row by row
    int row_width = (int)(floor((24.0*((double)img->width) + 31.0)/32.0)*4.0);
    int bytes_to_pad = row_width - img->width*3;
    int row_index;
    int col_index;
    int offset = 0x0;
    int pad_index;
    int byte_index;
    int pixel_index;

    uint8_t *img_buf = malloc(img->pixel_array_byte_size + img->height*bytes_to_pad);

    if (img_buf == NULL) {
        int errsv = errno;
        error(1, errsv, "Failed to allocate memory for the image");
    }

    // Write each row starting at the back of the pixel array but the front of img_buf
    for (row_index = 0; row_index < img->height; row_index++) {
        for (col_index = 0; col_index < img->width; col_index++) {
            // Write blue then green then red
            byte_index = row_index*img->width*3 + col_index*3 + offset;
            pixel_index = img->n_of_pixels - (row_index*img->width + col_index) - 1;

            img_buf[byte_index] = img->pixel_array[pixel_index].Blue;
            img_buf[byte_index + 1] = img->pixel_array[pixel_index].Green;
            img_buf[byte_index + 2] = img->pixel_array[pixel_index].Red;
        }

        // Add padding
        for (pad_index = 0; pad_index < bytes_to_pad; pad_index++) {
            img_buf[row_index*img->width*3 + col_index*3 + offset + pad_index] = 0;
        }

        // Add offset
        offset += bytes_to_pad;
    }

    // Write the buffer to the file
    if (write(fildes, img_buf, img->pixel_array_byte_size) == -1) {
        int errsv = errno;
        error(1, errsv, "Failed write");
    }

    // Free the memory
    if (img_buf) free(img_buf);
}
