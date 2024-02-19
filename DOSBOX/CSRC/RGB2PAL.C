/* Program to read in binary .RGB and .SIZ simplified image files and output:
	1.) An ASCII palette file containing decimal RRRGGGBBB, one value per line
*/

/* 

1.) read in SIZ
2.) read in RGB

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
	working_file = fopen(working_file_name,"wb");
	


	/** 3. Algorithm to count the palettes */
	/** 3a.check all previous pixels, if none matches, increment the count and output the palette data*/
	count = 0;
	for ( y = 0; y < siz_image_height(rgb_addresses->siz_data); y++)
	{
		for (x = 0; x < siz_image_width(rgb_addresses->siz_data); x++)
		{
			/* For each pixel, iterate over all previous pixels up to that point */
			for ( py = 0; py < siz_image_height(rgb_addresses->siz_data); py++)
			{
				for ( px = 0; px < siz_image_width(rgb_addresses->siz_data); px++)
				{
					if ( px == x && py == y) /* Yes, so no previous matches, this is a new palette */
					{
						/* Output the palette*/
						fprintf(working_file,"%03u%03u%03u\r\n",
							(unsigned int)rgb_pixel_color(x, y, (unsigned long)RGB_RED_ADDRESS_OFFSET, rgb_addresses),
							(unsigned int)rgb_pixel_color(x, y, (unsigned long)RGB_GREEN_ADDRESS_OFFSET, rgb_addresses),
							(unsigned int)rgb_pixel_color(x, y, (unsigned long)RGB_BLUE_ADDRESS_OFFSET, rgb_addresses));
						/* Increment the count*/
						count++;
						/* Get out of internal loop for this x/y */
						goto previous_pixel_loop_done;
					}
					else /* No? Need to check if the current pixel matches this previous pixel */ 
					{
						if (
							(rgb_pixel_color(px, py, (unsigned long)RGB_RED_ADDRESS_OFFSET, rgb_addresses) ==
							rgb_pixel_color(x, y, (unsigned long)RGB_RED_ADDRESS_OFFSET, rgb_addresses)) &&
							(rgb_pixel_color(px, py, (unsigned long)RGB_GREEN_ADDRESS_OFFSET, rgb_addresses) ==
							rgb_pixel_color(x, y, (unsigned long)RGB_GREEN_ADDRESS_OFFSET, rgb_addresses)) &&
							(rgb_pixel_color(px, py, (unsigned long)RGB_BLUE_ADDRESS_OFFSET, rgb_addresses) ==
							rgb_pixel_color(x, y, (unsigned long)RGB_BLUE_ADDRESS_OFFSET, rgb_addresses))
						)
						{ /* We found a match, get out of internal loop for this x/y */
							goto previous_pixel_loop_done;
						}
						else { /* Haven't found a previous match yet for this x/y, Keep looking! */
							continue;
						}
					}
				}
			}
			previous_pixel_loop_done: /* gotos are verboten...but here we are*/
			continue;
		}
	}
	fprintf(working_file,"%c",'\x1A'); /* add ctrl+z */

	/* Free working file */
	fclose(working_file);
	
	fprintf(stderr,"Found %lu unique palettes\r\n",count);

	/* Free working file name */
	free(working_file_name);

	/* Free RGB Data */
	free(rgb_addresses->rgb_data);
	free(rgb_addresses);
	free(siz_data);

	return 0;
}
