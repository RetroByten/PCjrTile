/* Header Library for RGB and SIZ files */

/*** DEF Statements ***/
/* RGB stores pixels in R G B */
#define RGB_RED_ADDRESS_OFFSET 0
#define RGB_GREEN_ADDRESS_OFFSET 1
#define RGB_BLUE_ADDRESS_OFFSET 2


#define RGB_BITS_PER_PIXEL 24
#define RGB_BITS_PER_BYTE 8
#define RGB_BYTES_PER_PIXEL BMP_BITS_PER_PIXEL/BMP_BITS_PER_BYTE



/*** Data/Type Memory Overlays ***/

struct siz_t {
	unsigned char raw_image_width[4];
	unsigned char raw_image_height[4];
};

struct rgb_addresses_t {
	struct siz_t* siz_data;
	unsigned char* rgb_data;
};

/*** SIZ Function definitions ***/

/* Get the 4 raw bytes at a particular address and convert them to a single 32 bit unsigned int */
unsigned long siz_four_byte_value(address)
unsigned char* address;
{ return (unsigned long)((address[3] << 24) | (address[2] << 16) | (address[1] << 8) | (address[0])); }

/* Write unsigned long to a 4 byte field */
int siz_write_four_byte_value(address, value)
unsigned char* address;
unsigned long value;
{
	address[0] = (unsigned char)(value & 0x000000FF);
	address[1] = (unsigned char)(value & 0x0000FF00) >> 8;
	address[2] = (unsigned char)(value & 0x00FF0000) >> 16;
	address[3] = (unsigned char)(value & 0xFF000000) >> 24;
	return 0;
}

/* Get the value of the rgb image width */
unsigned long siz_image_width(siz_h)
struct siz_t* siz_h;
{ return siz_four_byte_value(siz_h->raw_image_width); }

/* Get the value of the rgb image height */
unsigned long siz_image_height(siz_h)
struct siz_t* siz_h;
{ return siz_four_byte_value(siz_h->raw_image_height); }

/* Given a number of pixels, return entire image size, used when generating a new RGB */
unsigned long siz_calculate_size(siz_h)
struct siz_t* siz_h;
{ return siz_image_width(siz_h) * siz_image_height(siz_h) * (unsigned long)RGB_BYTES_PER_PIXEL; }

/*** RGB Function definitions ***/

/* Internal helper function to get the address offset of a pixel */
unsigned long rgb_address_pixel_xy(x, y, rgb_addresses)
unsigned long x;
unsigned long y;
struct rgb_addresses_t* rgb_addresses;
{ return (x + (y * siz_image_width(rgb_addresses->siz_data))) * (unsigned long)RGB_BYTES_PER_PIXEL; }

/* Return the R, G, or B value of the requested pixel*/
unsigned char rgb_pixel_color(x, y, c, rgb_addresses)
unsigned long x;
unsigned long y;
unsigned long c; /* Should be the #define RGB_RED_ADDRESS_OFFSET, RGB_GREEN_ADDRESS_OFFSET, RGB_BLUE_ADDRESS_OFFSET */
struct rgb_addresses_t* rgb_addresses;
{ return rgb_addresses->rgb_data[rgb_address_pixel_xy(x,y,rgb_addresses) + c]; }

/* Write RGB to an XY coordinate */
int rgb_write_color(x, y, r, g, b, rgb_addresses)
unsigned long x;
unsigned long y;
unsigned char r;
unsigned char g;
unsigned char b;
struct rgb_addresses_t* rgb_addresses;
{
	rgb_addresses->rgb_data[rgb_address_pixel_xy(x, y, rgb_addresses) + RGB_RED_ADDRESS_OFFSET] = r;
	rgb_addresses->rgb_data[rgb_address_pixel_xy(x, y, rgb_addresses) + RGB_GREEN_ADDRESS_OFFSET] = g;
	rgb_addresses->rgb_data[rgb_address_pixel_xy(x, y, rgb_addresses) + RGB_BLUE_ADDRESS_OFFSET] = b;
	return 0;
}
