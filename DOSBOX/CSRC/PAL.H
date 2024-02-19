/* Header Library for PAL files */

/*** DEF Statements ***/
/* PAL stores pixels in R G B */
#define PAL_RED_ADDRESS_OFFSET 0
#define PAL_GREEN_ADDRESS_OFFSET 1
#define PAL_BLUE_ADDRESS_OFFSET 2
#define PAL_ASCII_DECIMAL_OFFSET '0'
/*
RRRGGGBBB\r\n
RRRGGGBBB\r\n
RRRGGGBBB\r\n
RRRGGGBBB\r\n
<ctrl+z>
*/
unsigned char convert_ascii_to_decimal(buf)
unsigned char* buf;
{
	return (unsigned char)
	(
		((buf[0] - PAL_ASCII_DECIMAL_OFFSET) * 100) +
		((buf[1] - PAL_ASCII_DECIMAL_OFFSET) * 10) +
		 (buf[2] - PAL_ASCII_DECIMAL_OFFSET)
	);
}


/*** Data/Type Memory Overlays ***/
struct pal_t {
	unsigned char rgb[3];
};

struct pal_addresses_t {
	unsigned char* unformatted_palette_data; /* Used to point to raw ascii palette data */
	struct pal_t* formatted_palette_data;
	unsigned int number_of_palettes;
};
