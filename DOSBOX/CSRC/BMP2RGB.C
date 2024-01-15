/* Program to read in a 24-bit BMP and output:
1.) <name>.SIZ, a binary file containing 8 bytes (4 bytes - Image Width, 4 bytes - Image Height)
2.) <name>.RGB, a binary file containing WIDTH*HEIGHT*3 bytes per pixel (Red, Green, Blue)
*/

/*** System/Compiler Libraries ***/
#ifndef __STDIO_H__
#define __STDIO_H__
#include <stdio.h>
#endif

#ifndef __STDLIB_H__
#define __STDLIB_H__
#include <stdlib.h>
#endif

/*** Project libraries ***/
#ifndef __BMP_H__
#define __BMP_H__
#include "BMP.H"
#endif

#ifndef __RGBSIZ_H__
#define __RGBSIZ_H__
#include "RGBSIZ.H"
#endif

/*** Globals ***/
struct bmp_addresses_t* bmp_addresses;
struct siz_t* siz_data;
struct rgb_addresses_t* rgb_addresses;

/*** Main Function ***/
main(argc, argv, envp)
int argc;
char** argv;
char** envp;
{
	/* LOCAL VARS */
	unsigned long x;
	unsigned long y;

	char* base_file_name; /* User supplied base file name */
	char* working_file_name; /* Working full file name + extension */
	char* working_file_extension; /* Pointer directly to the extension of working_file_name */
	FILE* working_file;

	/* CODE */
	fprintf(stderr, "PCjr BMP2RGB\r\n");
	if ( argc < 2 ) {
		fprintf(stderr, "Arg $1: 24-bpp BMP file <name> (omit .BMP extension)\r\n");
		fprintf(stderr, "Output 1: <name>.SIZ\r\n");
		fprintf(stderr, "Output 2: <name>.RGB\r\n");
		return 1;
	}
	
	/* Read in base file name */
	base_file_name = argv[1];
	fprintf(stderr, "Input Base File Name: %s\r\n", base_file_name);

	/* Calculate length of the base file name + add extension */
	/* y is the length of the base filename in bytes + 4 for the .<3_letter_file_extension> */
	y = (unsigned long)strlen(base_file_name) + (unsigned long)4;
	working_file_name = (char*)calloc((unsigned int)y, (unsigned int)sizeof(char));

	/* Setup file_name for BMP */
	/* x is the loop counter */
	for (x = 0; x < (unsigned long)strlen(base_file_name); x++) {
		working_file_name[x] = base_file_name[x];
	}
	working_file_extension = &(working_file_name[x]); /* Store pointer to beginning of extension, currently null terminator is there */
	working_file_extension[0] = '.'; /* Overrides original null terminator */
	working_file_extension[1] = 'B';
	working_file_extension[2] = 'M';
	working_file_extension[3] = 'P';
	working_file_extension[4] = '\0'; /* Adds new null terminator */
	fprintf(stderr, "BMP file name: %s\r\n", working_file_name);
	
	/* Open BMP file as read-binary */
	working_file = fopen(working_file_name, "rb");

	/* Determine BMP file size then reset the ptr */
	/* y = number of bytes in the file */
	y = 0;
	while (1) {
		fgetc(working_file);
		if (feof(working_file)) {
			break;
		}
		y++;
	}
	rewind(working_file); /* Reset to beginning of file */
	fprintf(stderr, "BMP File Size: %lu bytes\r\n", y);

	/** TODO(#3) - Check if file size is too big, bail if it is, close file/memory **/
	
	/* Allocate memory for the BMP addresses structure */
	bmp_addresses = (struct bmp_addresses_t*)calloc((unsigned int)1, (unsigned int)sizeof(struct bmp_addresses_t));
	fprintf(stderr, "bmp_addresses=%lu\r\n", (unsigned long)bmp_addresses);

	/* Allocate memory for BMP data */
	/* y is the number of bytes in the file still */
	bmp_addresses->bmp_data = (unsigned char*)calloc((unsigned int)y, (unsigned int)sizeof(unsigned char));
	fprintf(stderr, "bmp_data=%lu\r\n", (unsigned long)bmp_addresses->bmp_data);

	/* Read BMP data into buffer */
	/* x = current byte index, y = number of bytes in the file still */
	fprintf(stderr, "BMP file contents: ");
	for (x = 0; x < y; x++) {
		bmp_addresses->bmp_data[x] = (unsigned char)fgetc(working_file);
		fprintf(stderr, "%02X", bmp_addresses->bmp_data[x]);
	}
	fprintf(stderr, "\r\n");
	fclose(working_file);

	/* Map BMP header struct to overlay BMP data */
	/* bmp_addresses->bmp_data is a pointer to the first byte of the BMP file data in memory */
	/* Set the bmp_h pointer to that address since bmp header is the first thing in the data */
	bmp_addresses->bmp_h = (struct bmp_header_t*)bmp_addresses->bmp_data;
	fprintf(stderr, "bmp_h=%lu\r\n", (unsigned long)bmp_addresses->bmp_h);
	fprintf(stderr, "file_size=%lu\r\n", bmp_file_size(bmp_addresses));
	
	/* Map DIB header struct to overlay BMP data */
	/* Start from the address address if the beginnig of the bmp_h */
	/* Add the size of the bmp_header_t struct in bytes */
	/* Set the dib_h pointer to that address */
	bmp_addresses->dib_h = (struct bmp_dib_header_t*)((unsigned long)bmp_addresses->bmp_h + (unsigned long)sizeof(struct bmp_header_t));
	fprintf(stderr, "dib_h=%lu\r\n", (unsigned long)bmp_addresses->dib_h);
	/* Print important values (DIB header) */
	fprintf(stderr, "header_size=%lu\r\n", bmp_header_size(bmp_addresses));
	fprintf(stderr, "image_width=%lu\r\n", bmp_image_width(bmp_addresses));
	fprintf(stderr, "image_height=%lu\r\n", bmp_image_height(bmp_addresses));

	/** TODO(#2) - Check if X/Y size is too big for memory, bail if it is, close file/memory **/


	/* Allocate memory for the siz_t*/
	siz_data = (struct siz_t*)calloc((unsigned int)1, (unsigned int)sizeof(struct siz_t));

	/* Set the width and height from the BMP data */
	siz_write_four_byte_value(siz_data->raw_image_width, bmp_image_width(bmp_addresses));
	siz_write_four_byte_value(siz_data->raw_image_height, bmp_image_height(bmp_addresses));
	fprintf(stderr, "siz_data->image_width=%lu\r\n", siz_image_width(siz_data));
	fprintf(stderr, "siz_data->image_height=%lu\r\n", siz_image_height(siz_data));

	/* Output the .SIZ file */
	/* Change working file extension to .SIZ, leave the '.' and '\0' alone */
	working_file_extension[1] = 'S';
	working_file_extension[2] = 'I';
	working_file_extension[3] = 'Z';
	fprintf(stderr, "Output file name: %s\r\n", working_file_name);
	
	/* Open the working file as write-binary */
	/* Output image width and image height */
	working_file = fopen(working_file_name, "wb");
	fwrite((char*)siz_data,(int)sizeof(struct siz_t),(int)1, working_file);
	fclose(working_file);

	/* Allocate memory for the rgb_addresses_t */
	rgb_addresses = (struct rgb_addresses_t*)calloc((unsigned int)1,(unsigned int)sizeof(struct rgb_addresses_t));
	rgb_addresses->siz_data = siz_data;
	rgb_addresses->rgb_data = (unsigned char*)calloc((unsigned int)1, (unsigned int)siz_calculate_size(rgb_addresses->siz_data));

	/* Set the RGB pixels from the BMP pixels*/
	for (y = 0; y < bmp_image_height(bmp_addresses); y++) {
		for (x = 0; x < bmp_image_width(bmp_addresses); x++ ) {
			rgb_write_color(x, y,
				bmp_pixel_color(x, y, (unsigned long)BMP_RED_ADDRESS_OFFSET, bmp_addresses),
				bmp_pixel_color(x, y, (unsigned long)BMP_GREEN_ADDRESS_OFFSET, bmp_addresses),
				bmp_pixel_color(x, y, (unsigned long)BMP_BLUE_ADDRESS_OFFSET, bmp_addresses),
				rgb_addresses);
		}
	}

	/* Output the .RGB file */
	/* Change working file extension to .RGB */
	working_file_extension[1] = 'R';
	working_file_extension[2] = 'G';
	working_file_extension[3] = 'B';
	fprintf(stderr, "Output file name: %s\r\n", working_file_name);

	/* Open the working file as write-binary */
	/* Write the RGB data to a file */ 
	working_file = fopen(working_file_name, "wb");
	fwrite((char*)rgb_addresses->rgb_data,(int)siz_calculate_size(rgb_addresses->siz_data),(int)1,working_file);
	fclose(working_file);

	/* Free the file_name memory */
	free(working_file_name);

	/* Free RGB Data */
	free(rgb_addresses->rgb_data);
	free(rgb_addresses);
	free(siz_data);

	/* Free BMP data buffer*/
	free(bmp_addresses->bmp_data);
	free(bmp_addresses);
	return 0;
}
