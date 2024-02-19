/* Program to read in binary .RGB, .SIZ, and .PAL file simplified image files and output:
	1.) An ASCII fi11le in MASM 2.0 format with the db or dw statements
*/

/* 

1.) read in SIZ
2.) read in RGB
3.) read in palette
4.) output initial file header
5.) For each pixel, determine which palette it is, and output the byte statements



3.) Algorithm to count the palettes 
3a.) Start count at zero
for each Pixel, check all previous pixels. If none matches, increment count and write out the palette


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
#ifndef __RGBSIZ_H__
#define __RGBSIZ_H__
#include "RGBSIZ.H"
#endif

#ifndef __PAL_H__
#define __PAL_H__
#include "PAL.H"
#endif

/** TODO - instead of writing the data out, first put it in the pal structs? */


/*** Globals ***/
struct siz_t* siz_data;
struct rgb_addresses_t* rgb_addresses;
struct pal_addresses_t* pal_addresses;

/*** Main Function ***/
main(argc, argv, envp)
int argc;
char** argv;
char** envp;
{
	/* LOCAL VARS */
	unsigned long x;
	unsigned long y;
	unsigned long px;
	unsigned long py;
	unsigned long count;
	unsigned char tmp;
	unsigned char* raw_siz; /* Used to write directly to the siz_t */

	char* base_file_name; /* User supplied base file name */
	char* working_file_name; /* Working full file name + extension */
	char* working_file_extension; /* Pointer directly to the extension of working_file_name */
	FILE* working_file;

	/* CODE */
	fprintf(stderr, "PCjr RGB2PAL\r\n");
	if (argc < 2) {
		fprintf(stderr, "Arg $1: Base RGB/SIZ file <name> (omit .RGB/.SIZ extension)\r\n");
		fprintf(stderr, "Output 1: <name>.PAL\r\n");
		return 1;
	}

	/* Read in base file name */
	base_file_name = argv[1];
	fprintf(stderr, "Input BASE file name: %s\r\n", base_file_name);

	/* Calculate length of the base file name + add extension */
	/* y is the length of the base filename in bytes + 4 for the .<3_letter_file_extension> */
	y = (unsigned long)strlen(base_file_name) + (unsigned long)4;
	working_file_name = (char*)calloc((unsigned int)y, (unsigned int)sizeof(char));

	/* Setup filename for .SIZ*/
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

	/** 1.) Read in SIZ **/

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
	}
	fclose(working_file);
	fprintf(stderr, "width=%lu\r\n", siz_image_width(siz_data));
	fprintf(stderr, "height=%lu\r\n", siz_image_height(siz_data));


	/** 2.) Read in the RGB **/

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


	/** Open the PAL file */
	working_file_extension[1] = 'P';
	working_file_extension[2] = 'A';
	working_file_extension[3] = 'L';
	fprintf(stderr, "PAL file name: %s\r\n", working_file_name);

	/* Open file, figure out how big it is */
	/* y = number of bytes in the file */
	working_file = fopen(working_file_name,"rb");
	y = 0;
	while (1) {
		fgetc(working_file);
		if (feof(working_file)) {
			break;
		}
		y++;
	}
	rewind(working_file); /* Reset to beginning of file */

	/* Allocate memory for the address struct and the unformatted data */
	pal_addresses = (struct pal_addresses_t*)calloc((unsigned int)1,(unsigned int)sizeof(struct pal_addresses_t));
	
	pal_addresses->unformatted_palette_data = (unsigned char*)calloc((unsigned int)1,(unsigned int)y);
	
		/* Read in unformatted data and close file, also count the number of new lines */
	pal_addresses->number_of_palettes = 0;
	for (x = 0; x < y; x++) {
		pal_addresses->unformatted_palette_data[x] = (unsigned char)fgetc(working_file);
		if(pal_addresses->unformatted_palette_data[x] != '\x1A'){ /* just don't print if it's ctrl+z */
			fprintf(stderr,"%c",(char)pal_addresses->unformatted_palette_data[x]);
		}
		if (pal_addresses->unformatted_palette_data[x] == '\n'){
			pal_addresses->number_of_palettes++;
		}
	}
	fclose(working_file);
	
	/* Allocate memory for palettes */
	pal_addresses->formatted_palette_data = (struct pal_t*)calloc((unsigned int)sizeof(struct pal_t),(unsigned int)count);
	/* Convert unformated pal data to formatted pal data */
	x = 0;
	for ( count = 0; count < pal_addresses->number_of_palettes; count++){


		pal_addresses->formatted_palette_data[count].rgb[PAL_RED_ADDRESS_OFFSET] = convert_ascii_to_decimal(&pal_addresses->unformatted_palette_data[x + 0]);
		pal_addresses->formatted_palette_data[count].rgb[PAL_GREEN_ADDRESS_OFFSET] = convert_ascii_to_decimal(&pal_addresses->unformatted_palette_data[x + 3]);
		pal_addresses->formatted_palette_data[count].rgb[PAL_BLUE_ADDRESS_OFFSET] = convert_ascii_to_decimal(&pal_addresses->unformatted_palette_data[x + 6]);

		x+=9;
		while (pal_addresses->unformatted_palette_data[x] != '\n') {
			x++;
		}
		x+=1; /* Add the index of the new line */
	}
	
	/* Print out palette values for debugging */
	/*
	for ( count = 0; count < pal_addresses->number_of_palettes; count++){
		fprintf(stderr,"Palette %lu: (%02X,%02X,%02X)\r\n",
		count,
		pal_addresses->formatted_palette_data[count].rgb[PAL_RED_ADDRESS_OFFSET],
		pal_addresses->formatted_palette_data[count].rgb[PAL_GREEN_ADDRESS_OFFSET],
		pal_addresses->formatted_palette_data[count].rgb[PAL_BLUE_ADDRESS_OFFSET]
		);
	}
	*/

	/* Output data to .ASM file */
	working_file_extension[1] = 'A';
	working_file_extension[2] = 'S';
	working_file_extension[3] = 'M';
	fprintf(stderr, "ASM file name: %s\r\n", working_file_name);
	working_file = fopen(working_file_name,"wb");

	fprintf(working_file,"EVEN\r\n");
	working_file_extension[0] = '\0'; /* Overrides original null terminator */
	fprintf(working_file,"%s",working_file_name);
	working_file_extension[0] = '.'; /* Overrides original null terminator */

	for (y = 0; y < siz_image_height(rgb_addresses->siz_data); y++){
		fprintf(working_file," db ");
		for (x = 0; x < siz_image_width(rgb_addresses->siz_data); x+=(unsigned long)2) {
			/* Identify which palette the even pixel is */
			tmp = 0;
			fprintf(stderr,"%lu,%lu:",x,y);
			for (count = 0; count < pal_addresses->number_of_palettes; count++){
				if (
					(rgb_pixel_color(x,y,(unsigned long)RGB_RED_ADDRESS_OFFSET,rgb_addresses) ==
					pal_addresses->formatted_palette_data[count].rgb[PAL_RED_ADDRESS_OFFSET]) &&
					(rgb_pixel_color(x,y,(unsigned long)RGB_GREEN_ADDRESS_OFFSET,rgb_addresses) ==
					pal_addresses->formatted_palette_data[count].rgb[PAL_GREEN_ADDRESS_OFFSET]) &&
					(rgb_pixel_color(x,y,(unsigned long)RGB_BLUE_ADDRESS_OFFSET,rgb_addresses) ==
					pal_addresses->formatted_palette_data[count].rgb[PAL_BLUE_ADDRESS_OFFSET])
				)
				{
					tmp=(unsigned char)(count << 4) | tmp;
					count = pal_addresses->number_of_palettes;
				}
			}
			fprintf(stderr,"[%02X,%02X,%02X]:%01X\r\n",
				rgb_pixel_color(x,y,(unsigned long)RGB_RED_ADDRESS_OFFSET,rgb_addresses),
				rgb_pixel_color(x,y,(unsigned long)RGB_GREEN_ADDRESS_OFFSET,rgb_addresses),
				rgb_pixel_color(x,y,(unsigned long)RGB_BLUE_ADDRESS_OFFSET,rgb_addresses),
				(unsigned char)tmp >> 4
			);
			
			fprintf(stderr,"%lu,%lu:",x+1,y);
			for (count = 0; count < pal_addresses->number_of_palettes; count++){
				if (
					(rgb_pixel_color(x+1,y,(unsigned long)RGB_RED_ADDRESS_OFFSET,rgb_addresses) ==
					pal_addresses->formatted_palette_data[count].rgb[PAL_RED_ADDRESS_OFFSET]) &&
					(rgb_pixel_color(x+1,y,(unsigned long)RGB_GREEN_ADDRESS_OFFSET,rgb_addresses) ==
					pal_addresses->formatted_palette_data[count].rgb[PAL_GREEN_ADDRESS_OFFSET]) &&
					(rgb_pixel_color(x+1,y,(unsigned long)RGB_BLUE_ADDRESS_OFFSET,rgb_addresses) ==
					pal_addresses->formatted_palette_data[count].rgb[PAL_BLUE_ADDRESS_OFFSET])
				)
				{
					tmp=(unsigned char)tmp | (count);
					count = pal_addresses->number_of_palettes;
				}
			}
			fprintf(stderr,"[%02X,%02X,%02X]:%01X\r\n",
				rgb_pixel_color(x+1,y,(unsigned long)RGB_RED_ADDRESS_OFFSET,rgb_addresses),
				rgb_pixel_color(x+1,y,(unsigned long)RGB_GREEN_ADDRESS_OFFSET,rgb_addresses),
				rgb_pixel_color(x+1,y,(unsigned long)RGB_BLUE_ADDRESS_OFFSET,rgb_addresses),
				(unsigned char)tmp & 0x0F
			);
			fprintf(working_file,"%03Xh",tmp);
			if ( x != siz_image_width(rgb_addresses->siz_data) - 2 ){
				fprintf(working_file,",");
			}
		}
		fprintf(working_file,"\r\n");
	}



	fprintf(working_file,"%c",'\x1A');
	fclose(working_file);


	/* Free working file name */
	free(working_file_name);

	/* Free pal addresses */
	free(pal_addresses->unformatted_palette_data);
	free(pal_addresses->formatted_palette_data);
	free(pal_addresses);

	/* Free RGB Data */
	free(rgb_addresses->rgb_data);
	free(rgb_addresses);
	free(siz_data);

	return 0;
}
