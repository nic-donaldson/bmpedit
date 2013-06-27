/* image_data_helper_functions.c
 * Nicholas Donaldson
 * u5350448
 *
 * Functions to help deal
 * with types defined in image_data_types.h
 *
 */

#include "image_data_helper_functions.h"
#include "error.h"
#include <stdio.h>

int min(int x, int y) { return x < y ? x : y; }
int max(int x, int y) { return x > y ? x : y; }

// Returns a pointer to the pixel at the given coordinates in the pixel array of img
struct pixel  *get_pixel_pointer_from_struct_image_x_y(int x, int y, struct image *img) {
    int pixel_index = img->width*y + (img->width - 1 - x);
    if (pixel_index >= img->n_of_pixels || pixel_index < 0) {
        error(1, 0, "Error in get_pixel_from_struct_image_x_y, coordinates not inside image");
    }
    return &img->pixel_array[pixel_index];
}

// Sets a pixel in img to have the same values as pix
void set_pixel_in_struct_image_x_y(int x, int y, struct image *img, struct pixel *pix) {
    struct pixel *pixel_to_change = get_pixel_pointer_from_struct_image_x_y(x, y, img);
    pixel_to_change->Red = pix->Red;
    pixel_to_change->Blue = pix->Blue;
    pixel_to_change->Green = pix->Green;
}

void print_pixel(struct pixel *ptr_pixel) {
    printf("Red: \t%d\n", ptr_pixel->Red);
    printf("Green: \t%d\n", ptr_pixel->Green);
    printf("Blue: \t%d\n", ptr_pixel->Blue);
}

struct pixel *get_nearest_pixel(int x, int y, struct image *img) {
    while (y < 0) y++;
    while (y > img->height-1) y--;
    while (x < 0) x++;
    while (x > img->width-1) x--;

    return get_pixel_pointer_from_struct_image_x_y(x, y, img);
}

// Adds two pixels together
// either adds the values together or finds
// their maximums
// Noise v. Sensitivity tradeoff
void add_two_pixels(struct pixel *pix1, struct pixel *pix2) {
    pix1->Red = min((pix1->Red + pix2->Red),255);
    pix1->Green = min((pix1->Green + pix2->Green),255);
    pix1->Blue = min((pix1->Blue + pix2->Blue),255);
    //pix1->Red = max(pix1->Red , pix2->Red);
    //pix1->Green = max(pix1->Green , pix2->Green);
    //pix1->Blue = max(pix1->Blue , pix2->Blue);
}

// Adds two images together, stores in first image
void add_two_images(struct image *img_1, struct image *img_2) {
    // Check same dimensions
    if (img_1->width != img_2->width || img_1->height != img_2->height) {
        error(1, 0, "add_two_images requires two images with the same dimensions");
    } else if (img_1->n_of_pixels != img_2->n_of_pixels) {
        error(1, 0, "add_two_images requires two images with the same number of pixels");
    }

    int pixel_index;
    for (pixel_index = 0; pixel_index < img_1->n_of_pixels; pixel_index++) {
        add_two_pixels(&img_1->pixel_array[pixel_index], &img_2->pixel_array[pixel_index]);
    }
}
