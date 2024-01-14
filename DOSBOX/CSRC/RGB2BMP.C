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

/*** Project library ***/
#ifndef __BMP_H__
#define __BMP_H__
#include "BMP.H"
#endif

/*** Globals ***/
struct bmp_addresses_t* bmp_addresses;

/*** Main Function ***/
main(argc, argv, envp)
int argc;
char** argv;
char** envp;
{
	/* LOCAL VARS */
	unsigned long x;
	unsigned long y;

	unsigned char r;
	unsigned char g;
	unsigned char b;

	char* base_file_name; /* User supplied base file name */
	char* working_file_name; /* Working full file name + extension */
	char* working_file_extension; /* Pointer directly to the extension of working_file_name */
	FILE* working_file;

	unsigned char* read_buffer;
	unsigned long bmp_size;
	unsigned long image_width;
	unsigned long image_height;
	unsigned long image_padding;

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

	/* Open SIZ file as read-binary */
	/* Determine BMP file size then reset the ptr */
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

	/* Read the file contents into a buffer */
	read_buffer = (unsigned char*)calloc((unsigned int)y, (unsigned int)sizeof(unsigned char));
	fprintf(stderr, "SIZ file contents: ");
	for (x = 0; x < y; x++) {
		read_buffer[x] = (unsigned char)fgetc(working_file);
		fprintf(stderr, "%02X", read_buffer[x]);
	}
	fprintf(stderr, "\r\n");
	fclose(working_file);

	/* Set up the temporary initial bmp headers data so we can use the BMP functions */
	bmp_addresses = (struct bmp_addresses_t*)calloc((unsigned int)1, (unsigned int)sizeof(struct bmp_addresses_t));
	bmp_addresses->bmp_data = (unsigned char*)calloc((unsigned int)1, (unsigned int)sizeof(struct bmp_header_t) + (unsigned int)sizeof(struct bmp_dib_header_t));
	bmp_addresses->bmp_h = (struct bmp_header_t*)bmp_addresses->bmp_data;
	bmp_addresses->dib_h = (struct bmp_dib_header_t*)((unsigned long)bmp_addresses->bmp_data + (unsigned long)sizeof(struct bmp_header_t));
	fprintf(stderr, "bmp_data=%lu\r\n", (unsigned long)bmp_addresses->bmp_data);
	fprintf(stderr, "bmp_h=%lu\r\n", (unsigned long)bmp_addresses->bmp_h);
	fprintf(stderr, "dib_h=%lu\r\n", (unsigned long)bmp_addresses->dib_h);

	/* Set sizes from stored SIZ data */
	bmp_addresses->dib_h->raw_image_width[0] = read_buffer[0];
	bmp_addresses->dib_h->raw_image_width[1] = read_buffer[1];
	bmp_addresses->dib_h->raw_image_width[2] = read_buffer[2];
	bmp_addresses->dib_h->raw_image_width[3] = read_buffer[3];
	bmp_addresses->dib_h->raw_image_height[0] = read_buffer[4];
	bmp_addresses->dib_h->raw_image_height[1] = read_buffer[5];
	bmp_addresses->dib_h->raw_image_height[2] = read_buffer[6];
	bmp_addresses->dib_h->raw_image_height[3] = read_buffer[7];
	free(read_buffer);

	/* Now that we have a tempory BMP header set, use BMP functions to get data to make the real BMP */
	image_width = bmp_image_width(bmp_addresses);
	image_height = bmp_image_height(bmp_addresses);
	fprintf(stderr, "X,Y=%lu,%lu\r\n", image_width, image_height);
	image_padding = bmp_pixel_row_padding(bmp_addresses) * image_height;
	bmp_size = (unsigned long)sizeof(struct bmp_header_t) +
		(unsigned long)sizeof(struct bmp_dib_header_t) +
		image_padding +
		(image_width * image_height * (unsigned long)BMP_BYTES_PER_PIXEL);
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
	bmp_write_four_byte_value(bmp_addresses->dib_h->raw_image_width, image_width);
	bmp_write_four_byte_value(bmp_addresses->dib_h->raw_image_height, image_height);
	bmp_write_two_byte_value(bmp_addresses->dib_h->raw_color_planes, (unsigned int)1);
	bmp_write_two_byte_value(bmp_addresses->dib_h->raw_bpp, (unsigned int)BMP_BITS_PER_PIXEL);

	fprintf(stderr, "Header: %c%c\r\n", (char)bmp_addresses->bmp_h->raw_header[0], (char)bmp_addresses->bmp_h->raw_header[1]);
	fprintf(stderr, "Size: %lu\r\n", bmp_file_size(bmp_addresses));
	fprintf(stderr, "Pixel array start: %lu\r\n", bmp_pixel_array_start(bmp_addresses));

	/* TODO (#4) - Look at horizontal/vertical resolution? Can probably ignore */

	/* OPEN RGB FILE */
	/* Output the .RGB file */
	/* Change working file extension to .RGB */
	working_file_extension[1] = 'R';
	working_file_extension[2] = 'G';
	working_file_extension[3] = 'B';
	fprintf(stderr, "RGB file name: %s\r\n", working_file_name);

	/* Open the working file as read-binary */
	/* Write each RGB triplet to the BMP image structure */
	working_file = fopen(working_file_name, "rb");
	fprintf(stderr, "RGB file contents: ");
	for (y = 0; y < bmp_image_height(bmp_addresses); y++) {
		for (x = 0; x < bmp_image_width(bmp_addresses); x++) {
			r = (unsigned char)fgetc(working_file);
			g = (unsigned char)fgetc(working_file);
			b = (unsigned char)fgetc(working_file);
			fprintf(stderr,"(%02X,%02X,%02X)",(unsigned char)r, (unsigned char)g, (unsigned char)b);
			bmp_write_color(x, y, r, g, b, bmp_addresses);
		}
	}
	fprintf(stderr, "\r\n");
	fclose(working_file);

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

	/* Free bmp_data and addresses */
	free(bmp_addresses->bmp_data);
	free(bmp_addresses);

	return 0;
}
