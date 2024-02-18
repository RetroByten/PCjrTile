/* Header Library for PAL files */

/*** DEF Statements ***/
/* PAL stores pixels in R G B */
#define PAL_RED_ADDRESS_OFFSET 0
#define PAL_GREEN_ADDRESS_OFFSET 1
#define PAL_BLUE_ADDRESS_OFFSET 2

/*
RRGGBB\r\n
RRGGBB\r\n
RRGGBB\r\n
RRGGBB\r\n
*/


/*** Data/Type Memory Overlays ***/

struct pal_t {
	unsigned char rgb[3];
};

struct pal_addresses_t {
	unsigned char* unformatted_palette_data; /* Used to point to raw ascii palette data */
	struct pal_t* formatted_palette_data;
};
