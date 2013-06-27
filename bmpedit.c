/*
 * bmpedit
 * Nicholas Donaldson
 * u5350448
 * 2013
 *
 */

#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <error.h>
#include "bmp_struct_image.h"
#include "filters.h"

// Misc helpful functions

void print_usage();
int str_is_digit_and_radix_point(char *str);

int str_is_digit_and_radix_point(char *str) {
    int str_len = strlen(str);
    int i;
    for (i = 0; i < str_len; i++) {
        if (isdigit(str[i]) || str[i] == '.') continue;
        else return 0;
    }

    return 1;
}

void print_usage() {
    printf("Usage: bmpedit [OPTIONS...] [inputX.bmp]...\n\
\n\
DESCRIPTION:\n\
  This program does simple edits of BMP image files. When the program runs it first prints \n\
  out the width and the height of the input image within the BMP file.  Once this is done \n\
  a filter (or sequence of filters) are applied to the image.  The resulting image is also\n\
  stored using BMP format into an output file.  \n\
\n\
OPTIONS:\n\
  -o FILE        Sets the output file for modified images (default output file is \"out.bmp\").\n\
  -t 0.0-1.0     Apply a threshold filter to the image with a threshold the threshold value given.\n\
  -i             Invert the image colours\n\
  -b 0.0-1.0     Blends two images together according to the blend coefficient, requires input2.bmp\n\
                 0.0 gives image 1 and 1.0 gives image 2\n\
                 Usage: bmpedit -b 0.0-1.0 [OPTIONS...] input1.bmp input2.bmp\n\
  -c x1,y1,x2,y2 Crop: Crops the image from (x1,y1) inclusive to (x2,y2) exclusive, x1 < x2, y1 < y2\n\
  -B 0.0-2.0     Brightness: Changes overall brightness of each pixel by 100*(argument-1)%% while keeping\n\
                 colours in the same proportion\n\
                 1.0 is no change, 0.0 is -100%%, 2.0 is +100%%\n\
  -e             Emboss: Applies an emboss effect, consider using with -g\n\
  -s 0.0-20.0    Sharpen: Makes the image appear sharper by various degrees\n\
                 16.0 is a reasonable value\n\
  -g             Greyscale: Converts the image to greyscale (still RGB though)\n\
  -G repeat,sd   Gaussian blur: A slow gaussian blur, repeat is the number of times it will run,\n\
                 sd is the standard deviation used to generate the values for the blur.\n\
                 In general, the higher the sd, the blurrier, but more repeats are\n\
                 needed for a substantial effect\n\
  -S             Sobel edge detection: A form of edge detection, try with -g\n\
  -h             Displays this usage message.\n");
}


int main (int argc, char* argv[]) {

    // Initialise stuff

    char *output_file_name;
    output_file_name = "out.bmp";

    char *input_file_name;
    char *input_2_file_name;

    // Will be the file descriptor for the BMP file
    int input_file;
    int input_2_file;
    int output_file;

    // Options
    int threshold_is_set = 0;
    double threshold_value = 0.0;

    int blend_is_set = 0;
    double blend_value = 0.0;

    int invert_is_set = 0;

    int crop_is_set = 0;
    char *crop_arg = '\0';

    int brightness_is_set = 0;
    double brightness_value = 0.0;

    int emboss_is_set = 0;

    int greyscale_is_set = 0;

    int sharpen_is_set = 0;
    double sharpen_value = 0.0;

    int sobel_is_set = 0;

    int gaussian_is_set = 0;
    char *gaussian_arg = '\0';

    // Handle command line arguments
    // Based off of http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt

    int option;
    while ((option = getopt (argc, argv, "G:Sgs:eH:B:c:b:iht:o:")) != -1) {
        switch(option) {
            case 'h':
                print_usage();
                break;
            case 'o':
                output_file_name = optarg;
                break;
            case 't':
                threshold_is_set = 1;
                if (!str_is_digit_and_radix_point(optarg)) {
                    error(1, 0, "A number is required for the threshold");
                }
                threshold_value = atof(optarg);
                break;
            case 'b':
                blend_is_set = 1;
                if (!str_is_digit_and_radix_point(optarg)) {
                    error(1, 0, "A number is required for the blend coefficient");
                }
                blend_value = atof(optarg);
                break;
            case 'e':
                emboss_is_set = 1;
                break;
            case 'g':
                greyscale_is_set = 1;
                break;
            case 'S':
                sobel_is_set = 1;
                break;
            case 's':
                sharpen_is_set = 1;
                if (!str_is_digit_and_radix_point(optarg)) {
                    error(1, 0, "A number is required for sharpen");
                }
                sharpen_value = atof(optarg);
                break;
            case 'B':
                brightness_is_set = 1;
                if (!str_is_digit_and_radix_point(optarg)) {
                    error(1, 0, "A number is required for the brightness");
                }
                brightness_value = atof(optarg);
                break;
            case 'i':
                invert_is_set = 1;
                break;
            case 'c':
                crop_is_set = 1;
                crop_arg = optarg;
                break;
            case 'G':
                gaussian_is_set = 1;
                gaussian_arg = optarg;
                break;
            case '?':
                if (optopt == 'o' || optopt == 't') {
                    fprintf (stderr, "-%c requires an argument", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Unknown option -%c.\n", optopt);
                } else {
                    fprintf(stderr, "Unknown option character \\x%x\n", optopt);
                }

                return 1;
                break;
            default:;
        }
    }

    // Grab the input file name

    // Check optind arg exists
    if (optind >= argc) {
        error(1, 0, "An input file is required, or you are missing an argument.\nTry bmpedit -h for help");
    }

    input_file_name = argv[optind];

    // Open the file
    input_file = open(input_file_name, O_RDONLY);

    if (input_file == -1) {
        int errsv = errno;
        error(1, errsv, "Error opening input file");
    }

    // Grab bitmap data and put into struct image
    struct image raw_image;
    bmp_to_struct_image(input_file, &raw_image);

    // Print width and height
    printf("Image width: %dpx\n", raw_image.width);
    printf("Image height: %dpx\n", raw_image.height);


    // Apply the filters

    // Blend
    if (blend_is_set) {

        if (blend_value > 1.0 || blend_value < 0.0) {
            error(1, 0, "Blend value must be between 0.0 and 1.0");
        }

        printf("Blending images...\n");

        // Check the right number of arguments exist
        if (optind+1 >= argc) {
            error(1, 0, "Two input files and a blend coefficient are required for\
 blend.\nTry bmpedit -h for help.");
        }
            
        input_2_file_name = argv[optind+1];
        input_2_file = open(input_2_file_name, O_RDONLY);

        if (input_2_file == -1) {
            int errsv = errno;
            error(1, errsv, "Error opening second input file");
        }

        struct image image_2;
        bmp_to_struct_image(input_2_file, &image_2);

        // Will it blend?
        if (blend_two_images(blend_value, &raw_image, &image_2) == -1) {
            error(1, 0, "Two input images need same dimensions");
        }
    }

    // Gaussian blur
    if (gaussian_is_set) {
        int repeat;
        double standard_deviation;
        parse_gaussian_arg(&repeat, &standard_deviation, gaussian_arg);
        if (repeat < 0) {
            error(1, 0, "Must repeat gaussian blur 1 or more times");
        }
        printf("Applying gaussian blur...\n");
        gaussian_blur(repeat, standard_deviation, &raw_image);
    }

    // Brightness
    if (brightness_is_set) {
        if (brightness_value < 0.0 || brightness_value > 2.0) {
            error(1, 0, "Brightness value must be between 0.0 and 2.0");
        }
        printf("Changing brightness of image...\n");
        brightness_image(brightness_value-1.0, &raw_image);
    }

    // Greyscale
    if (greyscale_is_set) {
        printf("Converting the image to greyscale (RGB)\n");
        greyscale_image(&raw_image);
    }

    // Sobel
    if (sobel_is_set) {
        printf("Applying sobel edge detection...\n");
        sobel_edge_detect_image(&raw_image);
    }

    // Invert
    if (invert_is_set) {
        printf("Inverting image...\n");
        invert_image(&raw_image);
    }

    // Threshold
    if (threshold_is_set) {
        if (threshold_value > 1.0 || threshold_value < 0.0) {
            error(1, 0, "Threshold must be between 0.0 and 1.0");
        }
        printf("Running threshold filter...\n");
        threshold_image(threshold_value, &raw_image);
    }

    // Emboss
    if (emboss_is_set) {
        printf("Embossing image...\n");
        emboss_image(&raw_image);
    }

    // Sharpen
    if (sharpen_is_set) {
        if (sharpen_value < 0.0 || sharpen_value > 20.0) {
            error(1, 0, "Sharpen value must be between 0.0 and 20.0");
        }
        printf("Sharpening image...\n");
        // Magic numbers that make sharpen work
        sharpen_image(8.01 + (20-sharpen_value), &raw_image);
    }

    // Crop
    if (crop_is_set) {
        // parse the argument
        int x1,y1,x2,y2;
        parse_crop_arg(&x1,&y1,&x2,&y2,crop_arg);
        
        if (x1 >= x2 || y1 >= y2) {
            error(1, 0, "Crop needs sensible values\nTry bmpedit -h for help");
        }

        printf("Cropping image...\n");
        crop_image(x1, y1, x2, y2, &raw_image);
        printf("New image width %dpx\n", raw_image.width);
        printf("New image height: %dpx\n", raw_image.height);
    }

    // Write modified image to output file
    output_file = open(output_file_name, O_WRONLY|O_CREAT|O_TRUNC, 0664);
    if (output_file == -1) {
        int errsv = errno;
        error(1, errsv, "Error opening output file");
    }

    struct_image_to_bmp(output_file, &raw_image);

    // Free stuff
    free(raw_image.pixel_array);

    // Close stuff

    close(input_file);
    close(output_file);

    return 0;
}
