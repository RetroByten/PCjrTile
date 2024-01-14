/* Header Library for 24-bit BMP files */

/*** DEF Statements ***/
/* BMP stores pixels in B G R */
#define BMP_BLUE_ADDRESS_OFFSET 0
#define BMP_GREEN_ADDRESS_OFFSET 1
#define BMP_RED_ADDRESS_OFFSET 2

#define BMP_BITS_PER_PIXEL 24
#define BMP_BITS_PER_BYTE 8
#define BMP_BYTES_PER_PIXEL BMP_BITS_PER_PIXEL/BMP_BITS_PER_BYTE
#define BMP_BYTE_ALIGNMENT 4

/*** Data/Type Memory Overlays ***/
/* BMP header memory overlay */
struct bmp_header_t { 	/* Raw byte values */
	unsigned char raw_header[2]; /* "BM" for windows */
	unsigned char raw_size[4]; /* Size of BMP file in bytes */
	unsigned char raw_reserve1[2];
	unsigned char raw_reserve2[2];
	unsigned char raw_offset[4]; /* Starting address offset of pixel data */
};

/* DIB header memory overlay */
struct bmp_dib_header_t { 	/* Raw byte values */
	unsigned char raw_header_size[4];
	unsigned char raw_image_width[4];
	unsigned char raw_image_height[4];
	unsigned char raw_color_planes[2];
	unsigned char raw_bpp[2];
	unsigned char raw_compression[4];
	unsigned char raw_size[4];
	unsigned char raw_horizontal_res[4];
	unsigned char raw_vertical_res[4];
	unsigned char raw_num_colors[4];
	unsigned char raw_num_important_colors[4];
};

/* BMP address struct pointers to the various bits of the BMP data in memory */
struct bmp_addresses_t {
	unsigned char* bmp_data;
	struct bmp_header_t* bmp_h;
	struct bmp_dib_header_t* dib_h;
};

/*** Function definitions ***/

/* Get the 4 raw bytes at a particular address and convert them to a single 32 bit unsigned int */
unsigned long bmp_four_byte_value(address)
unsigned char* address;
{ return (unsigned long)((address[3] << 24) | (address[2] << 16) | (address[1] << 8) | (address[0])); }

/* Get the value of the dib header size */
unsigned long bmp_header_size(bmp_addresses)
struct bmp_addresses_t* bmp_addresses;
{ return bmp_four_byte_value(bmp_addresses->dib_h->raw_header_size); }

/* Get the value of the bmp file_size */
unsigned long bmp_file_size(bmp_addresses)
struct bmp_addresses_t* bmp_addresses;
{ return bmp_four_byte_value(bmp_addresses->bmp_h->raw_size); }

/* Get the value of the dib image width */
unsigned long bmp_image_width(bmp_addresses)
struct bmp_addresses_t* bmp_addresses;
{ return bmp_four_byte_value(bmp_addresses->dib_h->raw_image_width); }

/* Get the value of the dib image height */
unsigned long bmp_image_height(bmp_addresses)
struct bmp_addresses_t* bmp_addresses;
{ return bmp_four_byte_value(bmp_addresses->dib_h->raw_image_height); }

/* Calculate how much padding there is per row, needs to pad to 4 byte alignment */
unsigned long bmp_pixel_row_padding(bmp_addresses)
struct bmp_addresses_t* bmp_addresses;
{
	unsigned long remainder;
	remainder = (bmp_image_width(bmp_addresses) * (unsigned long)BMP_BYTES_PER_PIXEL) % (unsigned long)BMP_BYTE_ALIGNMENT;
	return (remainder == 0) ? (unsigned long)0 : ((unsigned long)BMP_BYTE_ALIGNMENT - remainder);
}

/* Get the address offset of the start of the bmp pixel array */
unsigned long bmp_pixel_array_start(bmp_addresses)
struct bmp_addresses_t* bmp_addresses;
{ return bmp_four_byte_value(bmp_addresses->bmp_h->raw_offset); }

/* Helper function to invert Y since image is stored bottom to top */
unsigned long bmp_invert_y(y, bmp_addresses)
unsigned long y;
struct bmp_addresses_t* bmp_addresses;
{ return (bmp_image_height(bmp_addresses) - (unsigned long)1) - y; }

/* Calculate how much padding would've occured to get to a given inverted Y */
unsigned long bmp_padding_yi(yi, bmp_addresses)
unsigned long yi; /* Inverted Y*/
struct bmp_addresses_t* bmp_addresses;
{ return yi * bmp_pixel_row_padding(bmp_addresses); }

/* Internal Helper Take a requested x/y pixel and output the address in memory */
unsigned long bmp_address_pixel_xy(x, y, bmp_addresses)
unsigned long x;
unsigned long y;
struct bmp_addresses_t* bmp_addresses;
{
	unsigned long yi; /* BMP Inverted vertical coordinate */
	unsigned long padding; /* Padding bytes used per row */
	unsigned long pixel_number;
	yi = bmp_invert_y(y, bmp_addresses);
	padding = bmp_padding_yi(yi, bmp_addresses);
	pixel_number = (yi * bmp_image_width(bmp_addresses)) + x;

	return bmp_pixel_array_start(bmp_addresses) +
		(pixel_number * (unsigned long)BMP_BYTES_PER_PIXEL) +
		padding;
}

/* Return the R, G, or B value of the requested pixel*/
unsigned char bmp_pixel_color(x, y, c, bmp_addresses)
unsigned long x;
unsigned long y;
unsigned long c; /* Should be the #define BMP_RED_ADDRESS_OFFSET, BMP_GREEN_ADDRESS_OFFSET, BMP_BLUE_ADDRESS_OFFSET */
struct bmp_addresses_t* bmp_addresses;
{ return bmp_addresses->bmp_data[bmp_address_pixel_xy(x, y, bmp_addresses) + c]; }

/* Given a number of pixels, return entire image size, used when generating a new BMP */
unsigned long bmp_calculate_size(bmp_addresses)
struct bmp_addresses_t* bmp_addresses;
{	
	return (unsigned long)sizeof(struct bmp_header_t) +
		(unsigned long)sizeof(struct bmp_dib_header_t) +
		((bmp_image_width(bmp_addresses) + bmp_pixel_row_padding(bmp_addresses)) * bmp_image_height(bmp_addresses) * (unsigned long)BMP_BYTES_PER_PIXEL);
}

/* Write unsigned long to a 4 byte field */
int bmp_write_four_byte_value(address, value)
unsigned char* address;
unsigned long value;
{
	address[0] = (unsigned char)(value & 0x000000FF);
	address[1] = (unsigned char)(value & 0x0000FF00) >> 8;
	address[2] = (unsigned char)(value & 0x00FF0000) >> 16;
	address[3] = (unsigned char)(value & 0xFF000000) >> 24;
	return 0;
}

/* Write unsigned int to a 2 byte field */
int bmp_write_two_byte_value(address, value)
unsigned char* address;
unsigned int value;
{
	address[0] = (unsigned char)(value & 0x00FF);
	address[1] = (unsigned char)(value & 0xFF00) >> 8;
	return 0;
}

/* Write unsigned char to a 1 byte field */
int bmp_write_one_byte_value(address, value)
unsigned char* address;
unsigned char value;
{
	address[0] = value;
	return 0;
}

/* Write RGB to an XY coordinate */
int bmp_write_color(x, y, r, g, b, bmp_addresses)
unsigned long x;
unsigned long y;
unsigned char r;
unsigned char g;
unsigned char b;
struct bmp_addresses_t* bmp_addresses;
{
	unsigned char* pixel_address;
	/* Absolute Pixel data address is beginning of bitmap + address of the xy */
	pixel_address = (unsigned char*)((unsigned long)bmp_addresses->bmp_data+bmp_address_pixel_xy(x, y, bmp_addresses));
	pixel_address[BMP_BLUE_ADDRESS_OFFSET] = b;
	pixel_address[BMP_GREEN_ADDRESS_OFFSET] = g;
	pixel_address[BMP_RED_ADDRESS_OFFSET] = r;
	return 0;
}
