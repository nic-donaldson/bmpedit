/* filters.c
 * Nicholas Donaldson
 * u5350448
 *
 * Standard filter functions
 * implemented on the image structures
 * defined in image_data_types.h
 *
 */

#include "filters.h"
#include "image_data_helper_functions.h"
#include "convolution_kernels.h"
#include <error.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

void threshold_pixel(double threshold_value, struct pixel *ptr_pixel) {
    // Get pixel average (grey value)
    double pixel_average = (double)(ptr_pixel->Blue + ptr_pixel->Green + ptr_pixel->Red) / 3.0;

    // Bring it between 0.0 and 1.0
    double pixel_grey_percent = pixel_average / 255.0;

    // Check average against threshold value
    if (pixel_grey_percent > threshold_value) {
        ptr_pixel-> Red = 0xFF;
        ptr_pixel-> Blue = 0xFF;
        ptr_pixel-> Green = 0xFF;
    } else {
        ptr_pixel-> Red = 0x0;
        ptr_pixel-> Blue = 0x0;
        ptr_pixel-> Green = 0x0;
    }
}

void threshold_image(double threshold_value, struct image *img) {
    int pixel_index;
    for (pixel_index = 0; pixel_index < img->n_of_pixels; pixel_index++) {
        threshold_pixel(threshold_value, &img->pixel_array[pixel_index]);
    }
}

void invert_pixel(struct pixel *ptr_pixel) {
    ptr_pixel->Red = 255 - ptr_pixel->Red;
    ptr_pixel->Green = 255 - ptr_pixel->Green;
    ptr_pixel->Blue = 255 - ptr_pixel->Blue;
}

void invert_image(struct image *img) {
    int pixel_index;
    for (pixel_index = 0; pixel_index < img->n_of_pixels; pixel_index++) {
        invert_pixel(&img->pixel_array[pixel_index]);
    }
}

// Blends two pixels together, the result is stored in pixel 1
// Blend coefficient is from pixel 1 to pixel 2, so
// a coefficient of 0 is all pixel 1, 0.5 is half/half,
// and 1 is all pixel 2
void blend_two_pixels(double blend_coefficient, struct pixel *pixel_1, struct pixel *pixel_2) {
    pixel_1->Red   = (int)((1.0-blend_coefficient)*pixel_1->Red + blend_coefficient*pixel_2->Red);
    pixel_1->Green = (int)((1.0-blend_coefficient)*pixel_1->Green + blend_coefficient*pixel_2->Green);
    pixel_1->Blue  = (int)((1.0-blend_coefficient)*pixel_1->Blue + blend_coefficient*pixel_2->Blue);
}

// Blends two images together with the blend_two_pixels function,
// stores the result in img_1
// Images must be the same dimensions and size
// Returns 0 on success, -1 on failure
int blend_two_images(double blend_coefficient, struct image *img_1, struct image *img_2) {
    // Check images can be blended
    if (img_1->width != img_2->width || img_1->height != img_2->height) {
        return -1;
    } else if (img_1->n_of_pixels != img_2->n_of_pixels) {
        return -1;
    }

    int pixel_index;
    for (pixel_index = 0; pixel_index < img_1->n_of_pixels; pixel_index++) {
        blend_two_pixels(blend_coefficient, &img_1->pixel_array[pixel_index], &img_2->pixel_array[pixel_index]);
    }
    return 0;
}


// Crops an image to contain all pixels between (x1,y1) inclusive and (x2,y2) exclusive
void crop_image (int x1, int y1, int x2, int y2, struct image *img) {
    int new_width = x2 - x1;
    int new_height = y2 - y1;

    if (new_width <= 0 || new_height <= 0 || new_width > img->width || new_height > img->height) {
        error(1, 0, "Crop needs sensible dimensions");
    }

    struct image new_img;
    new_img.width = new_width;
    new_img.height = new_height;
    new_img.n_of_pixels = new_width*new_height;
    new_img.pixel_array_byte_size = new_img.n_of_pixels*3;
    new_img.pixel_array = malloc(new_img.n_of_pixels*sizeof(struct pixel));

    if (new_img.pixel_array == NULL) {
        error(1, 0, "Couldn't allocate memory for new pixel array");
    }

    int x,y;
    struct pixel *pix_ptr;
    for (y = y1; y < y2; y++) {
        for (x = x1; x < x2; x++) {
            pix_ptr = get_pixel_pointer_from_struct_image_x_y(x, y, img);
            set_pixel_in_struct_image_x_y(x-x1, y-y1, &new_img, pix_ptr);
        }
    }
    
    // Free the old array
    free(img->pixel_array);       

    // Point the old pointer at the new array
    img->pixel_array = new_img.pixel_array;

    // Set the new width/height/byte size
    img->width = new_img.width;
    img->height = new_img.height;
    img->n_of_pixels = new_img.n_of_pixels;
    img->pixel_array_byte_size = new_img.pixel_array_byte_size;

}

void parse_crop_arg(int *x1, int *y1, int *x2, int *y2,char *crop_arg) {
    // Get values out of string with strtok
    *x1 = atoi(strtok(crop_arg, ","));
    *y1 = atoi(strtok(NULL, ","));
    *x2 = atoi(strtok(NULL, ","));
    *y2 = atoi(strtok(NULL, ","));
}

void set_brightness_pixel(double brightness_percentage_change, struct pixel *pix) {
    int old_red = pix->Red;
    int old_green = pix->Green;
    int old_blue = pix->Blue;

    // Necessary because pixel colours are uint8_t
    int new_red = pix->Red;
    int new_green = pix->Green;
    int new_blue = pix->Blue;

    double brightness = (old_red + old_green + old_blue)/3.0;
    double new_brightness = brightness_percentage_change*brightness + brightness;

    new_red = (int)(3*new_brightness - old_green - old_blue);
    new_green = (int)(3*new_brightness - old_red - old_blue);
    new_blue = (int)(3*new_brightness - old_red- old_green);

    if (new_red > 255) {
        pix->Red = 255;
    } else if (new_red < 0) {
        pix->Red = 0;
    } else {
        pix->Red = new_red;
    }

    if (new_green> 255) {
        pix->Green = 255;
    } else if (new_green < 0) {
        pix->Green = 0;
    } else {
        pix->Green = new_green;
    }

    if (new_blue> 255) {
        pix->Blue = 255;
    } else if (new_blue < 0) {
        pix->Blue = 0;
    } else {
        pix->Blue = new_blue;
    }
}

void brightness_image(double brightness_percentage_change, struct image *img) {
    int pixel_index;
    for (pixel_index = 0; pixel_index < img->n_of_pixels; pixel_index++) {
        set_brightness_pixel(brightness_percentage_change, &img->pixel_array[pixel_index]);
    }
}


void greyscale_pixel(struct pixel *pix) {
    double grey_value = (int) ((pix->Red + pix->Green + pix->Blue)/3.0);
    pix->Red = grey_value;
    pix->Green = grey_value;
    pix->Blue = grey_value;
}

void greyscale_image(struct image *img) {
    int pixel_index;
// Blends two images together with the blend_two_pixels function,
// stores the result in img_1
// Images must be the same dimensions and size
// Returns 0 on success, -1 on failure
int blend_two_images(double blend_coefficient, struct image *img_1, struct image *img_2) {
    // Check images can be blended
    if (img_1->width != img_2->width || img_1->height != img_2->height) {
        return -1;
    } else if (img_1->n_of_pixels != img_2->n_of_pixels) {
        return -1;
    }

    int pixel_index;
    for (pixel_index = 0; pixel_index < img_1->n_of_pixels; pixel_index++) {
        blend_two_pixels(blend_coefficient, &img_1->pixel_array[pixel_index], &img_2->pixel_array[pixel_index]);
    }
    return 0;
}
    for (pixel_index = 0; pixel_index < img->n_of_pixels; pixel_index++) {
        greyscale_pixel(&img->pixel_array[pixel_index]);
    }
}

// Emboss image
// Creates an embossed effect,
// conv matrix from
// http://docs.gimp.org/en/plug-in-convmatrix.html
void emboss_image (struct image *img) {
    double conv_matrix[5][5] = {{0.0, 0.0, 0.0, 0.0, 0.0},
                          {0.0, -2.0, -1.0, 0.0, 0.0},
                          {0.0, -1.0, 1.0, 1.0, 0.0},
                          {0.0, 0.0, 1.0, 2.0, 0.0},
                          {0.0, 0.0, 0.0, 0.0, 0.0}};
    normalise_kernel(conv_matrix);
    apply_kernel_to_struct_image(conv_matrix, img);
}

// Sharpen image
// based off of various sharpen conv matrices that I have seen
// around, varies the middle value for difference in effect
// Here is one example: http://www.nist.gov/lispix/imlab/filter/sharpen.html
void sharpen_image(double sharpen_value, struct image *img) {
    double conv_matrix[5][5] = {{0.0,  0.0,  0.0,  0.0, 0.0},
                                {0.0, -1.0, -1.0, -1.0, 0.0},
                                {0.0, -1.0,  0.0, -1.0, 0.0},
                                {0.0, -1.0, -1.0, -1.0, 0.0},
                                {0.0,  0.0,  0.0,  0.0, 0.0}};
    conv_matrix[2][2] = sharpen_value;
    normalise_kernel(conv_matrix);
    apply_kernel_to_struct_image(conv_matrix, img);
}      

// Sobel edge detector
// Detects horizontal and vertical edges
// Matrices are from
// http://homepages.inf.ed.ac.uk/rbf/HIPR2/sobel.htm
// Expensive implementation to take advantage of conv matrix functions
void sobel_edge_detect_image(struct image *img) {
    // Duplicate the original image then add the two images together
    struct image dup_image;
    dup_image.width = img->width;
    dup_image.height = img->height;
    dup_image.n_of_pixels = img->n_of_pixels;
    dup_image.pixel_array = malloc(dup_image.n_of_pixels*sizeof(struct pixel));

    if (dup_image.pixel_array == NULL) {
        int errsv = errno;
        error(1, errsv, "Failed to allocate memory for the duplicate image");
    }

    memcpy(dup_image.pixel_array, img->pixel_array, dup_image.n_of_pixels*sizeof(struct pixel));

    double conv_matrix[5][5] = {{0.0,  0.0,  0.0,  0.0, 0.0},
                                {0.0, 1.0, 0.0, -1.0, 0.0},
                                {0.0, 2.0,  0.0, -2.0, 0.0},
                                {0.0, 1.0, 0.0, -1.0, 0.0},
                                {0.0,  0.0,  0.0,  0.0, 0.0}};
    normalise_kernel(conv_matrix);
    apply_kernel_to_struct_image(conv_matrix, img);

    
    double conv_2_matrix[5][5] = {{0.0,  0.0,  0.0,  0.0, 0.0},
                                  {0.0, 1.0, 2.0, 1.0, 0.0},
                                  {0.0, 0.0,  0.0, 0.0, 0.0},
                                  {0.0, -1.0, -2.0, -1.0, 0.0},
                                  {0.0,  0.0,  0.0,  0.0, 0.0}};
    normalise_kernel(conv_2_matrix);
    apply_kernel_to_struct_image(conv_2_matrix, &dup_image);

    add_two_images(img, &dup_image);
    free(dup_image.pixel_array);
}  


// Gaussian blur
// Generates a gaussian convolution matrix
// then applies it to the image
// This implementation is slower than necessary
// the application of the convolution matrix
// could be split up and applied horizontally
// and vertically, but I want to use the conv
// matrix functions
void gaussian_blur(int repeat, double standard_deviation, struct image *img) {
    double conv_matrix[5][5];
    generate_gaussian_kernel(conv_matrix, standard_deviation);

    int i;
    for (i = 0; i < repeat; i++) {
        apply_kernel_to_struct_image(conv_matrix, img);
    }
}

void parse_gaussian_arg(int *repeat, double *standard_deviation, char *gaussian_arg) {
    // Get values out of string with strtok
    *repeat = atoi(strtok(gaussian_arg, ","));
    *standard_deviation = atof(strtok(NULL, ","));
}
