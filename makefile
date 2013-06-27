CC = gcc
CFLAGS = -g -Wall -O3 -lm

all: bmpedit

bmp_struct_image.o: bmp_struct_image.c

image_data_helper_functions.o: image_data_helper_functions.c

convolution_kernels.o: image_data_helper_functions.o convolution_kernels.c

filters.o: convolution_kernels.o image_data_helper_functions.o

bmpedit: convolution_kernels.o filters.o image_data_helper_functions.o bmp_struct_image.o bmpedit.c
	gcc -o bmpedit convolution_kernels.o filters.o image_data_helper_functions.o bmp_struct_image.o bmpedit.c -lm

clean:
	rm bmpedit
	rm *.o


