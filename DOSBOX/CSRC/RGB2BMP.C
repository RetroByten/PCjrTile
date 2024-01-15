/* Program to read in binary .RGB and .SIZ simplified image files and output:
	1.) A 24-bpp BMP file
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

	unsigned char* raw_siz; /* Used to write directly to the siz_t */

	char* base_file_name; /* User supplied base file name */
	char* working_file_name; /* Working full file name + extension */
	char* working_file_extension; /* Pointer directly to the extension of working_file_name */
	FILE* working_file;

	unsigned long bmp_size;

	/* CODE */
	fprintf(stderr, "PCjr RGB2BMP\r\n");
	if (argc < 2) {
		fprintf(stderr, "Arg $1: Base RGB/SIZ file <name> (omit .RGB/.SIZ extension)\r\n");
		fprintf(stderr, "Output 1: <name>.BMP\r\n");
		return 1;
	}

	/* Read in base file name */
	base_file_name = argv[1];
	fprintf(stderr, "Input BASE file name: %s\r\n", base_file_name);

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
	working_file_extension[1] = 'S';
	working_file_extension[2] = 'I';
	working_file_extension[3] = 'Z';
	working_file_extension[4] = '\0'; /* Adds new null terminator */
	fprintf(stderr, "SIZ file name: %s\r\n", working_file_name);

	/* Allocate the siz_t and point raw_siz for direct writing */
	siz_data = (struct siz_t*)calloc((unsigned int)1, (unsigned int)sizeof(struct siz_t));
	raw_siz = (unsigned char*)siz_data;
	/* Open SIZ file as read-binary */
	/* Determine SIZ file size then reset the ptr */
	/* y = number of bytes in the file */
	working_file = fopen(working_file_name, "rb");
	y = 0;
	while (1) {
		fgetc(working_file);
		if (feof(working_file)) {
			break;
		}
		y++;
	}
	rewind(working_file); /* Reset to beginning of file */
	fprintf(stderr, "SIZ file size: %d bytes\r\n", y);

	/* Read the SIZ contents into the SIZ */
	for (x = 0; x < y; x++) {
		raw_siz[x] = (unsigned char)fgetc(working_file);
		/*siz_data->raw_image_width[x] = (unsigned char)fgetc(working_file);*/
	}
	fclose(working_file);
	fprintf(stderr, "width=%lu\r\n", siz_image_width(siz_data));
	fprintf(stderr, "height=%lu\r\n", siz_image_height(siz_data));

	/* Set up the temporary initial bmp headers data so we can use the BMP functions */
	/* TODO (#6) - look at this later to see if it can be simplified at all, probably not */
	bmp_addresses = (struct bmp_addresses_t*)calloc((unsigned int)1, (unsigned int)sizeof(struct bmp_addresses_t));
	bmp_addresses->bmp_data = (unsigned char*)calloc((unsigned int)1, (unsigned int)sizeof(struct bmp_header_t) + (unsigned int)sizeof(struct bmp_dib_header_t));
	bmp_addresses->bmp_h = (struct bmp_header_t*)bmp_addresses->bmp_data;
	bmp_addresses->dib_h = (struct bmp_dib_header_t*)((unsigned long)bmp_addresses->bmp_data + (unsigned long)sizeof(struct bmp_header_t));

	/* Set sizes from stored SIZ data */
	bmp_write_four_byte_value(bmp_addresses->dib_h->raw_image_width, siz_image_width(siz_data));
	bmp_write_four_byte_value(bmp_addresses->dib_h->raw_image_height, siz_image_height(siz_data));
	
	fprintf(stderr, "dib_h->image_width=%lu\r\n", bmp_image_width(bmp_addresses));
	fprintf(stderr, "dib_h->image_height=%lu\r\n", bmp_image_height(bmp_addresses));

	/* Now that we have a tempory BMP header set, use BMP functions to get data to make the real BMP */
	bmp_size = bmp_calculate_size(bmp_addresses);
	fprintf(stderr, "bmp_size=%lu\r\n", bmp_size);
	
	/* Free the temporary initial bmp data now that we have actual calculations */
	free(bmp_addresses->bmp_data);

	/* Allocate new BMP that has enough room for headers, pixels, and padding */
	bmp_addresses->bmp_data = (unsigned char*)calloc((unsigned int)1, (unsigned int)bmp_size);
	/* Set the new addresses */
	bmp_addresses->bmp_h = (struct bmp_header_t*)bmp_addresses->bmp_data;
	bmp_addresses->dib_h = (struct bmp_dib_header_t*)((unsigned long)bmp_addresses->bmp_data + (unsigned long)sizeof(struct bmp_header_t));

	/* Start filling in header data */
	bmp_write_one_byte_value(bmp_addresses->bmp_h->raw_header, (unsigned char)'B');
	bmp_write_one_byte_value((unsigned char*)((unsigned long)bmp_addresses->bmp_h->raw_header + (unsigned long)1), (unsigned char)'M');
	bmp_write_four_byte_value(bmp_addresses->bmp_h->raw_size, bmp_size);
	bmp_write_four_byte_value(bmp_addresses->bmp_h->raw_offset, (unsigned long)sizeof(struct bmp_header_t) + (unsigned long)sizeof(struct bmp_dib_header_t));
	bmp_write_four_byte_value(bmp_addresses->dib_h->raw_header_size, (unsigned long)sizeof(struct bmp_dib_header_t));
	bmp_write_four_byte_value(bmp_addresses->dib_h->raw_image_width, siz_image_width(siz_data));
	bmp_write_four_byte_value(bmp_addresses->dib_h->raw_image_height, siz_image_height(siz_data));
	bmp_write_two_byte_value(bmp_addresses->dib_h->raw_color_planes, (unsigned int)1);
	bmp_write_two_byte_value(bmp_addresses->dib_h->raw_bpp, (unsigned int)BMP_BITS_PER_PIXEL);

	fprintf(stderr, "Header: %c%c\r\n", (char)bmp_addresses->bmp_h->raw_header[0], (char)bmp_addresses->bmp_h->raw_header[1]);
	fprintf(stderr, "Size: %lu\r\n", bmp_file_size(bmp_addresses));
	fprintf(stderr, "Pixel array start: %lu\r\n", bmp_pixel_array_start(bmp_addresses));

	/* TODO (#4) - Look at horizontal/vertical resolution? Can probably ignore */

	/* Allocate memory for the rgb_addresses_t */
	rgb_addresses = (struct rgb_addresses_t*)calloc((unsigned int)1, (unsigned int)sizeof(struct rgb_addresses_t));
	rgb_addresses->siz_data = siz_data;
	rgb_addresses->rgb_data = (unsigned char*)calloc((unsigned int)1, (unsigned int)siz_calculate_size(rgb_addresses->siz_data));


	/* OPEN RGB FILE */
	/* Output the .RGB file */
	/* Change working file extension to .RGB */
	working_file_extension[1] = 'R';
	working_file_extension[2] = 'G';
	working_file_extension[3] = 'B';
	fprintf(stderr, "RGB file name: %s\r\n", working_file_name);

	/* Read in the RGB data into the RGB structure */
	working_file = fopen(working_file_name, "rb");
	fprintf(stderr, "RGB contents: ");
	for (x = 0; x < siz_calculate_size(rgb_addresses->siz_data); x++) {
		rgb_addresses->rgb_data[x] = (unsigned char)fgetc(working_file);
		fprintf(stderr,"%02X", rgb_addresses->rgb_data[x]);
	}
	fprintf(stderr, "\r\n");
	fclose(working_file);
	
	/* Transfer each pixel from RGB to BMP */
	/* We know RGB data itself is good*/
	fprintf(stderr,"BMP Pixel contents: ");
	for (y = 0; y < siz_image_height(rgb_addresses->siz_data); y++) {
		for (x = 0; x < siz_image_width(rgb_addresses->siz_data); x++) {
			bmp_write_color(x, y,
				rgb_pixel_color(x, y, (unsigned long)RGB_RED_ADDRESS_OFFSET, rgb_addresses),
				rgb_pixel_color(x, y, (unsigned long)RGB_GREEN_ADDRESS_OFFSET, rgb_addresses),
				rgb_pixel_color(x, y, (unsigned long)RGB_BLUE_ADDRESS_OFFSET, rgb_addresses),
				bmp_addresses);
			fprintf(stderr, "(%02X,%02X,%02X)",
				bmp_pixel_color(x, y, (unsigned long)BMP_RED_ADDRESS_OFFSET, bmp_addresses),
				bmp_pixel_color(x, y, (unsigned long)BMP_GREEN_ADDRESS_OFFSET, bmp_addresses),
				bmp_pixel_color(x, y, (unsigned long)BMP_BLUE_ADDRESS_OFFSET, bmp_addresses));
		}
	}
	fprintf(stderr, "\r\n");

	/* OPEN BMP FILE */
	/* Output the .BMP file */
	/* Change working file extension to .BMP */
	working_file_extension[1] = 'B';
	working_file_extension[2] = 'M';
	working_file_extension[3] = 'P';
	fprintf(stderr, "BMP file name: %s\r\n", working_file_name);

	/* Open the working file as write-binary */
	/* Write the BMP data to file */
	working_file = fopen(working_file_name, "wb");
	fwrite(bmp_addresses->bmp_data, (int)bmp_file_size(bmp_addresses), (int)1, working_file);
	fclose(working_file);

	/* Free working file name */
	free(working_file_name);

	/* Free RGB Data */
	free(rgb_addresses->rgb_data);
	free(rgb_addresses);
	free(siz_data);

	/* Free bmp_data and addresses */
	free(bmp_addresses->bmp_data);
	free(bmp_addresses);

	return 0;
}
