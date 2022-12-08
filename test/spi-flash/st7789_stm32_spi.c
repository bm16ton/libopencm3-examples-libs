/*
MIT License

Copyright (c) 2020 Avra Mitra

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include "ILI9486_Defines.h"
#include "timing_stm32.h"
#include "st7789_stm32_spi.h"
#include <libopencm3/stm32/f4/nvic.h>
//#include "xpt2046.h"
#include <stdio.h>

#define ST_BUFFER_SIZE_BYTES	1024
//TFT width and height default global variables
uint16_t st_tftwidth = 480;
uint16_t st_tftheight = 320;


/**
 * Set an area for drawing on the display with start row,col and end row,col.
 * User don't need to call it usually, call it only before some functions who don't call it by default.
 * @param x1 start column address.
 * @param y1 start row address.
 * @param x2 end column address.
 * @param y2 end row address.
 */
void st_set_address_window(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
	_st_write_command_16bit(TFT_CASET);

	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_ACTIVE;
	#endif
	ST_DC_DAT;
	_st_write_data_16bit((uint8_t)(x1 >> 8));
	_st_write_data_16bit((uint8_t)x1);
	_st_write_data_16bit((uint8_t)(x2 >> 8));
	_st_write_data_16bit((uint8_t)x2);


	_st_write_command_16bit(TFT_PASET);
	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_ACTIVE;
	#endif
	ST_DC_DAT;
	_st_write_data_16bit((uint8_t)(y1 >> 8));
	_st_write_data_16bit((uint8_t)y1);
	_st_write_data_16bit((uint8_t)(y2 >> 8));
	_st_write_data_16bit((uint8_t)y2);

	_st_write_command_16bit(TFT_RAMWR);
}



/*
 * Render a character glyph on the display. Called by `_st_draw_string_main()`
 * User need NOT call it
 */
void _st_render_glyph(uint16_t x, uint16_t y, uint16_t fore_color, uint16_t back_color, const tImage *glyph, uint8_t is_bg)
{
	uint16_t width = 0, height = 0;

	width = glyph->width;
	height = glyph->height;

	uint16_t temp_x = x;
	uint16_t temp_y = y;

	uint8_t mask = 0x80;
	uint8_t bit_counter = 0;

	const uint8_t *glyph_data_ptr = (const uint8_t *)(glyph->data);
	uint8_t glyph_data = 0;

	// font bitmaps are stored in column major order (scanned from left-to-right, not the conventional top-to-bottom)
	// as font glyphs have heigher height than width, this scanning saves some storage.
	// So, we also render in left-to-right manner.

	// Along x axis (width)
	for (int i = 0; i < width; i++)
	{
		// Along y axis (height)
		for (int j = 0; j < height; j++)
		{

			// load new data only when previous byte (or word, depends on glyph->dataSize) is completely traversed by the mask
			// bit_counter = 0 means glyph_data is completely traversed by the mask
			if (bit_counter == 0)
			{
				glyph_data = *glyph_data_ptr++;
				bit_counter = glyph->dataSize;
			}
			// Decrement bit counter
			bit_counter--;

			//If pixel is blank
			if (glyph_data & mask)
			{
				//Has background color (not transparent bg)
				if (is_bg)
				{
					st_draw_pixel(temp_x, temp_y, back_color);
				}
			}

			//if pixel is not blank
			else
			{
				st_draw_pixel(temp_x, temp_y, fore_color);
			}

			glyph_data <<= 1;
			temp_y++;
		}

		//New col starts. So, row is set to initial value and col is increased by one
		temp_y = y;
		temp_x++;

		//Reset the bit counter cause we're moving to next column, so we start with a new byte
		bit_counter = 0;
	}
}

void lcd_draw_char(uint16_t x, uint16_t y, const tFont *font, char c)
{
    uint16_t fore_color = ST_COLOR_WHITE;
    uint16_t back_color = ST_COLOR_BLACK;
	const tImage *img = NULL;
	for (uint8_t i = 0; i < font->length; i++)
	{
		if (font->chars[i].code == c)
		{
			img = font->chars[i].image;
			break;
		}
	}
	// No glyph (img) found, so return from this function
	if (img == NULL)
	{
		return;
	}

		_st_render_glyph(x, y, fore_color, back_color, img, 0);
}


/**
 * Renders a string by drawing each character glyph from the passed string.
 * Called by `st_draw_string()` and `st_draw_string_withbg()`.
 * Text is wrapped automatically if it hits the screen boundary.
 * x_padding and y_padding defines horizontal and vertical distance (in px) between two characters
 * is_bg=1 : Text will habe background color,   is_bg=0 : Text will have transparent background
 * User need NOT call it.
 */

void _st_draw_string_main(uint16_t x, uint16_t y, char *str, uint16_t fore_color, uint16_t back_color, const tFont *font, uint8_t is_bg)
{
	uint16_t x_temp = x;
	uint16_t y_temp = y;

	uint8_t x_padding = 0;
	uint8_t y_padding = 0;
	const tImage *img = NULL;
	uint16_t width = 0, height = 0;



	while (*str)
	{
		if (*str == '\n')
		{
			x_temp = x;					//go to first col
			y_temp += (font->chars[0].image->height + y_padding);	//go to next row (row height = height of space)
		}

		else if (*str == '\t')
		{
			x_temp += 4 * (font->chars[0].image->height + y_padding);	//Skip 4 spaces (width = width of space)
		}
		else
		{
			for (uint8_t i = 0; i < font->length; i++)
			{
				if (font->chars[i].code == *str)
				{
					img = font->chars[i].image;
					break;
				}
			}
			// No glyph (img) found, so return from this function
			if (img == NULL)
			{
				return;
			}

			width = img->width;
			height = img->height;

			if(y_temp + (height + y_padding) > st_tftheight - 1)	//not enough space available at the bottom
				return;
			if (x_temp + (width + x_padding) > st_tftwidth - 1)	//not enough space available at the right side
			{
				x_temp = x;					//go to first col
				y_temp += (height + y_padding);	//go to next row
			}


			if (is_bg)
				_st_render_glyph(x_temp, y_temp, fore_color, back_color, img, 1);
			else
				_st_render_glyph(x_temp, y_temp, fore_color, back_color, img, 0);
			x_temp += (width + x_padding);		//next char position
		}


		str++;
	}
}


/**
 * Draws a character at a given position, fore color, back color.
 * @param x Start col address
 * @param y Start row address
 * @param character the ASCII character to be drawn
 * @param fore_color foreground color
 * @param back_color background color
 * @param font Pointer to the font of the character
 * @param is_bg Defines if character has background or not (transparent)
 */
void st_draw_char(uint16_t x, uint16_t y, char character, uint16_t fore_color, uint16_t back_color, const tFont *font, uint8_t is_bg)
{
	const tImage *img = NULL;
	for (uint8_t i = 0; i < font->length; i++)
	{
		if (font->chars[i].code == character)
		{
			img = font->chars[i].image;
			break;
		}
	}
	// No glyph (img) found, so return from this function
	if (img == NULL)
	{
		return;
	}

	if (is_bg)
		_st_render_glyph(x, y, fore_color, back_color, img, 1);
	else
		_st_render_glyph(x, y, fore_color, back_color, img, 0);
}

void st_draw_hex_ptr(uint16_t x, uint16_t y, uint8_t *data, uint16_t color, const tFont *font) {
char batman[sizeof(data)];
uint8_t i;  
    for(i =0; i < sizeof(data); i++) {
    sprintf(batman, "%02x", data[i]);
//    batman[sizeof(batman)] = '\0';
    st_draw_string(x, y, "0x", color, font);

    st_draw_string(x, y, batman, color, font);
    }
}

/**
 * Draws a string on the display with `font` and `color` at given position.
 * Background of this string is transparent
 * @param x Start col address
 * @param y Start y address
 * @param str pointer to the string to be drawn
 * @param color 16-bit RGB565 color of the string
 * @param font Pointer to the font of the string
 */
void st_draw_string(uint16_t x, uint16_t y, char *str, uint16_t color, const tFont *font)
{
	_st_draw_string_main(x, y, str, color, 0, font, 0);
}


/**
 * Draws a string on the display with `font`, `fore_color`, and `back_color` at given position.
 * The string has background color
 * @param x Start col address
 * @param y Start y address
 * @param str pointer to the string to be drawn
 * @param foe_color 16-bit RGB565 color of the string
 * @param back_color 16-bit RGB565 color of the string's background
 * @param font Pointer to the font of the string
 */
void st_draw_string_withbg(uint16_t x, uint16_t y, char *str, uint16_t fore_color, uint16_t back_color, const tFont *font)
{
	_st_draw_string_main(x, y, str, fore_color, back_color, font, 1);
}


/**
 * Draw a bitmap image on the display
 * @param x Start col address
 * @param y Start row address
 * @param bitmap Pointer to the image data to be drawn
 */

void st_draw_bitmap(uint16_t x, uint16_t y, const tImage *bitmap)
{
	uint16_t width = 0, height = 0;
	width = bitmap->width;
	height = bitmap->height;
	st_set_address_window(x, y, x + width-1, y + height-1);

	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_ACTIVE;
	#endif
	ST_DC_DAT;

	#ifndef ST_USE_SPI_DMA
		uint32_t bytes_to_write = width * height * 2;
		uint16_t transfer_size = ST_BUFFER_SIZE_BYTES;
		uint32_t src_start_address = 0;

		while (bytes_to_write)
		{
			transfer_size = (bytes_to_write < transfer_size) ? bytes_to_write : transfer_size;
			dma_start((void *)(&bitmap->data[src_start_address]), transfer_size);
			src_start_address += ST_BUFFER_SIZE_BYTES;
			bytes_to_write -= transfer_size;
		}
		
	#else

		uint32_t total_pixels = width * height;
		for (uint16_t pixels = 0; pixels < total_pixels; pixels++)
		{
			ST_WRITE_8BIT((uint8_t)(bitmap->data[2*pixels]));
			ST_WRITE_8BIT((uint8_t)(bitmap->data[2*pixels + 1]));
		}

	#endif

	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_IDLE;
	#endif
}

void st_draw_bitmap_nodma(uint16_t x, uint16_t y, const tImage *bitmap)
{
	uint16_t width = 0, height = 0;
	width = bitmap->width;
	height = bitmap->height;
	st_set_address_window(x, y, x + width-1, y + height-1);

	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_ACTIVE;
	#endif
	ST_DC_DAT;

		uint32_t total_pixels = width * height;
		for (uint16_t pixels = 0; pixels < total_pixels; pixels++)
		{
			ST_WRITE_8BIT((uint8_t)(bitmap->data[2*pixels]));
			ST_WRITE_8BIT((uint8_t)(bitmap->data[2*pixels + 1]));
		}



	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_IDLE;
	#endif
}

/**
 * Fills `len` number of pixels with `color`.
 * Call st_set_address_window() before calling this function.
 * @param color 16-bit RGB565 color value
 * @param len 32-bit number of pixels
 */

void st_fill_color(uint16_t color, uint32_t len)
{
	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_ACTIVE;
	#endif
	ST_DC_DAT;
	uint8_t color_high = color >> 8;
	uint8_t color_low = color;

	#ifndef ST_USE_SPI_DMA		
		uint8_t disp_buffer[ST_BUFFER_SIZE_BYTES];
		for (uint16_t i = 0; i < ST_BUFFER_SIZE_BYTES; i = i+2)
		{
			disp_buffer[i] = color_high;
			disp_buffer[i + 1] = color_low;
		}

		// len is pixel count. But each pixel is 2 bytes. So, multiply by 2
		uint32_t bytes_to_write = len * 2;
		uint16_t transfer_size = ST_BUFFER_SIZE_BYTES;
		while (bytes_to_write)
		{
			transfer_size = (bytes_to_write < transfer_size) ? bytes_to_write : transfer_size;
			dma_start(disp_buffer, transfer_size);
			bytes_to_write -= transfer_size;
		}

	#else
		/*
		* Here, macros are directly called (instead of inline functions) for performance increase
		*/
		uint16_t blocks = (uint16_t)(len / 64); // 64 pixels/block
		uint8_t  pass_count;

		// Write first pixel
		ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low);
		len--;

		while(blocks--)
		{
			pass_count = 16;
			while(pass_count--)
			{
				ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low); 	ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low); //2
				ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low); 	ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low); //4
			}
		}
		pass_count = len & 63;
		while (pass_count--)
		{
			// write here the remaining data
			ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low);
		}
		
	#endif

	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_IDLE;
	#endif
}

void st_fill_color_nodma(uint16_t color, uint32_t len)
{
	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_ACTIVE;
	#endif
	ST_DC_DAT;
	uint8_t color_high = color >> 8;
	uint8_t color_low = color;
		uint16_t blocks = (uint16_t)(len / 64); // 64 pixels/block
		uint8_t  pass_count;

		// Write first pixel
		ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low);
		len--;

		while(blocks--)
		{
			pass_count = 16;
			while(pass_count--)
			{
				ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low); 	ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low); //2
				ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low); 	ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low); //4
			}
		}
		pass_count = len & 63;
		while (pass_count--)
		{
			// write here the remaining data
			ST_WRITE_8BIT(color_high); ST_WRITE_8BIT(color_low);
		}
		ST_CS_IDLE;
}
/**
 * Fills `len` number of pixels with `color`.
 * Call st_set_address_window() before calling this function.
 * @param color_arr pointer to uint8_t array. Each 16-bit color is seperated into two 8-bit `high` and `low` components
 * @param bytes 32-bit number of bytes in the array (= no. of pixels x2)
 */

void st_fill_color_array(uint8_t *color_arr, uint32_t bytes)
{
	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_ACTIVE;
	#endif
	ST_DC_DAT;

	#ifndef ST_USE_SPI_DMA		
	
		// len is pixel count. But each pixel is 2 bytes. So, multiply by 2
		uint32_t bytes_to_write = bytes;
		uint16_t transfer_size = ST_BUFFER_SIZE_BYTES;
		while (bytes_to_write)
		{
			transfer_size = (bytes_to_write < transfer_size) ? bytes_to_write : transfer_size;
			dma_start(color_arr, transfer_size);
			bytes_to_write -= transfer_size;
		}

	#else
		/*
		* Here, macros are directly called (instead of inline functions) for performance increase
		*/
		uint32_t len = bytes / 2;
		uint16_t blocks = (uint16_t)(len / 64); // 64 pixels/block
		uint8_t  pass_count;

		// Write first pixel
		ST_WRITE_8BIT(*color_arr); ST_WRITE_8BIT(*(++color_arr));
		--len;

		while(blocks--)
		{
			pass_count = 16;
			while(pass_count--)
			{
				ST_WRITE_8BIT(*(++color_arr)); ST_WRITE_8BIT(*(++color_arr)); 	ST_WRITE_8BIT(*(++color_arr)); ST_WRITE_8BIT(*(++color_arr)); //2
				ST_WRITE_8BIT(*(++color_arr)); ST_WRITE_8BIT(*(++color_arr)); 	ST_WRITE_8BIT(*(++color_arr)); ST_WRITE_8BIT(*(++color_arr)); //4
			}
		}
		pass_count = len & 63;
		while (pass_count--)
		{
			// write here the remaining data
			ST_WRITE_8BIT(*(++color_arr)); ST_WRITE_8BIT(*(++color_arr));
		}
		
	#endif

	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_IDLE;
	#endif
}


/**
 * Fills a rectangular area with `color`.
 * Before filling, performs area bound checking
 * @param x Start col address
 * @param y Start row address
 * @param w Width of rectangle
 * @param h Height of rectangle
 * @param color 16-bit RGB565 color
 */
void st_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	if (x >= st_tftwidth || y >= st_tftheight || w == 0 || h == 0)
		return;
	if (x + w - 1 >= st_tftwidth)
		w = st_tftwidth - x;
	if (y + h - 1 >= st_tftheight)
		h = st_tftheight - y;

	st_set_address_window(x, y, x + w - 1, y + h - 1);
	st_fill_color(color, (uint32_t)w * (uint32_t)h);
}


/*
 * Same as `st_fill_rect()` but does not do bound checking, so it's slightly faster
 */
void st_fill_rect_fast(uint16_t x1, uint16_t y1, uint16_t w, uint16_t h, uint16_t color)
{
	st_set_address_window(x1, y1, x1 + w - 1, y1 + h - 1);
	st_fill_color(color, (uint32_t)w * (uint32_t)h);
}


/**
 * Fill the entire display (screen) with `color`
 * @param color 16-bit RGB565 color
 */
void st_fill_screen(uint16_t color)
{
	st_set_address_window(0, 0, st_tftwidth - 1, st_tftheight - 1);
	st_fill_color(color, (uint32_t)st_tftwidth * (uint32_t)st_tftheight);
}

void st_fill_screen_nodma(uint16_t color)
{
	st_set_address_window(0, 0, st_tftwidth - 1, st_tftheight - 1);
	st_fill_color_nodma(color, (uint32_t)st_tftwidth * (uint32_t)st_tftheight);
}

/**
 * Draw a rectangle
*/
void st_draw_rectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
	// Perform bound checking
	if (x >= st_tftwidth || y >= st_tftheight || w == 0 || h == 0)
		return;
	if (x + w - 1 >= st_tftwidth)
		w = st_tftwidth - x;
	if (y + h - 1 >= st_tftheight)
		h = st_tftheight - y;

	_st_draw_fast_h_line(x, y, x+w-1, 1, color);
	_st_draw_fast_h_line(x, y+h, x+w-1, 1, color);
	_st_draw_fast_v_line(x, y, y+h-1, 1, color);
	_st_draw_fast_v_line(x+w, y, y+h-1, 1, color);


}

/*
 * Called by st_draw_line().
 * User need not call it
 */
void _st_plot_line_low(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t width, uint16_t color)
{
	int16_t dx = x1 - x0;
	int16_t dy = y1 - y0;
	int8_t yi = 1;
	uint8_t pixels_per_point = width * width;	//no of pixels making a point. if line width is 1, this var is 1. if 2, this var is 4 and so on
	uint8_t color_high = (uint8_t)(color >> 8);
	uint8_t color_low = (uint8_t)color;
	if (dy < 0)
	{
		yi = -1;
		dy = -dy;
	}

	int16_t D = 2*dy - dx;
	uint16_t y = y0;
	uint16_t x = x0;

	while (x <= x1)
	{
		st_set_address_window(x, y, x+width-1, y+width-1);
		//Drawing all the pixels of a single point

		#ifdef ST_RELEASE_WHEN_IDLE
			ST_CS_ACTIVE;
		#endif
		ST_DC_DAT;
		for (uint8_t pixel_cnt = 0; pixel_cnt < pixels_per_point; pixel_cnt++)
		{
			_st_write_data_16bit(color_high);
			_st_write_data_16bit(color_low);
		}
		#ifdef ST_RELEASE_WHEN_IDLE
			ST_CS_IDLE;
		#endif

		if (D > 0)
		{
			y = y + yi;
			D = D - 2*dx;
		}
		D = D + 2*dy;
		x++;
	}
}


/*
 * Called by st_draw_line().
 * User need not call it
 */
void _st_plot_line_high(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t width, uint16_t color)
{
	int16_t dx = x1 - x0;
	int16_t dy = y1 - y0;
	int8_t xi = 1;
	uint8_t pixels_per_point = width * width;	//no of pixels making a point. if line width is 1, this var is 1. if 2, this var is 4 and so on
	uint8_t color_high = (uint8_t)(color >> 8);
	uint8_t color_low = (uint8_t)color;

	if (dx < 0)
	{
		xi = -1;
		dx = -dx;
	}

	int16_t D = 2*dx - dy;
	uint16_t y = y0;
	uint16_t x = x0;

	while (y <= y1)
	{
		st_set_address_window(x, y, x+width-1, y+width-1);
		//Drawing all the pixels of a single point

		#ifdef ST_RELEASE_WHEN_IDLE
			ST_CS_ACTIVE;
		#endif
		ST_DC_DAT;
		for (uint8_t pixel_cnt = 0; pixel_cnt < pixels_per_point; pixel_cnt++)
		{
			_st_write_data_16bit(color_high);
			_st_write_data_16bit(color_low);
		}
		#ifdef ST_RELEASE_WHEN_IDLE
			ST_CS_IDLE;
		#endif

		if (D > 0)
		{
			x = x + xi;
			D = D - 2*dy;
		}
		D = D + 2*dx;
		y++;
	}
}


/**
 * Draw a line from (x0,y0) to (x1,y1) with `width` and `color`.
 * @param x0 start column address.
 * @param y0 start row address.
 * @param x1 end column address.
 * @param y1 end row address.
 * @param width width or thickness of the line
 * @param color 16-bit RGB565 color of the line
 */
void st_draw_line(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t width, uint16_t color)
{
	/*
	* Brehensen's algorithm is used.
	* Not necessarily start points has to be less than end points.
	*/

	if (x0 == x1)	//vertical line
	{
		_st_draw_fast_v_line(x0, y0, y1, width, color);
	}
	else if (y0 == y1)		//horizontal line
	{
		_st_draw_fast_h_line(x0, y0, x1, width, color);
	}

	else
	{
		if (abs(y1 - y0) < abs(x1 - x0))
		{
			if (x0 > x1)
				_st_plot_line_low(x1, y1, x0, y0, width, color);
			else
				_st_plot_line_low(x0, y0, x1, y1, width, color);
		}

		else
		{
			if (y0 > y1)
				_st_plot_line_high(x1, y1, x0, y0, width, color);
			else
				_st_plot_line_high(x0, y0, x1, y1, width, color) ;
		}
	}

}


/*
 * Called by st_draw_line().
 * User need not call it
 */
void _st_draw_fast_h_line(uint16_t x0, uint16_t y0, uint16_t x1, uint8_t width, uint16_t color)
{
	/*
	* Draw a horizontal line very fast
	*/
	if (x0 < x1)
		st_set_address_window(x0, y0, x1, y0+width-1);	//as it's horizontal line, y1=y0.. must be.
	else
		st_set_address_window(x1, y0, x0, y0+width-1);	
	st_fill_color(color, (uint32_t)width * (uint32_t)abs(x1 - x0 + 1));
}


/*
 * Called by st_draw_line().
 * User need not call it
 */
void _st_draw_fast_v_line(uint16_t x0, uint16_t y0, uint16_t y1, uint8_t width, uint16_t color)
{
	/*
	* Draw a vertical line very fast
	*/
	if (y0 < y1)
		st_set_address_window(x0, y0, x0+width-1, y1);	//as it's vertical line, x1=x0.. must be.
	else
		st_set_address_window(x0, y1, x0+width-1, y0);	
	st_fill_color(color, (uint32_t)width * (uint32_t)abs(y1 - y0 + 1));
}


/**
 * Draw a pixel at a given position with `color`
 * @param x Start col address
 * @param y Start row address
 */
void st_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{


	st_set_address_window(x, y, x, y);
	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_ACTIVE;
	#endif
	ST_DC_DAT;
	ST_WRITE_8BIT((uint8_t)(color >> 8));
	ST_WRITE_8BIT((uint8_t)color);
	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_IDLE;
	#endif
}
/*
void st_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
	
//	* Why?: This function is mainly added in the driver so that  ui libraries can use it.
//	* example: LittlevGL requires user to supply a function that can draw pixel
	

	st_set_address_window(x, y, x, y);
	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_ACTIVE;
	#endif
	ST_DC_DAT;
	uint8_t color_high = color >> 8;
	uint8_t color_low = color;
				_st_write_data_16bit(color_high); _st_write_data_16bit(color_low); 	_st_write_data_16bit(color_high); _st_write_data_16bit(color_low); //2
				_st_write_data_16bit(color_high); _st_write_data_16bit(color_low); 	_st_write_data_16bit(color_high); _st_write_data_16bit(color_low); //4
//	_st_write_data_16bit((uint8_t)(color >> 8));
//	_st_write_data_16bit((uint8_t)color);
	#ifdef ST_RELEASE_WHEN_IDLE
		ST_CS_IDLE;
	#endif
}
*/

/**
 * Rotate the display clockwise or anti-clockwie set by `rotation`
 * @param rotation Type of rotation. Supported values 0, 1, 2, 3
 */
void st_rotate_display(uint8_t rotation)
{
	/*
	* 	(uint8_t)rotation :	Rotation Type
	* 					0 : Default landscape
	* 					1 : Potrait 1
	* 					2 : Landscape 2
	* 					3 : Potrait 2
	*/
	// Set max rotation value to 4
	rotation = rotation % 4;
	_st_write_command_16bit(TFT_MADCTL);		//Memory Access Control
	switch (rotation)
	{		
		case 0:
			
			_st_write_data_16bit(TFT_MAD_BGR);	// Default
			st_tftheight = 320;
			st_tftwidth = 480;
			break;
		case 1:
			_st_write_data_16bit(TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_BGR);
			st_tftheight = 320;
			st_tftwidth = 480;
			break;
		case 2:
			_st_write_data_16bit(TFT_MAD_MY | TFT_MAD_MV | TFT_MAD_BGR);
			st_tftheight = 320;
			st_tftwidth = 480;
//			st_set_address_window( 0, 120,  240,  240);
			break;
		case 3:
			_st_write_data_16bit(TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_BGR);
			st_tftheight = 320;
			st_tftwidth = 480;
			break;
	}
}

static void spi_init(void)
{
    // Set pin mode for SPI managed pins to alternate function
    gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE,
        ST_SCL		  // SCK - serial clock
        | ST_SDA	  // MOSI - master out slave in
        | ST_CS	  // NSS - slave select
        | ST_MISO //miso
//       | TS_CS_PIN  // gpio A9
    );

    // Set alternate function for SPI managed pins to AF5 for SPI2
    gpio_set_af(GPIOB, GPIO_AF5,
        ST_SCL       // SPI2_SCK
        | ST_SDA    // SPI2_MOSI
        | ST_CS     // SPI2_NSS
        | ST_MISO //miso
//        | TS_CS_PIN  // gpio A9
    );

    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO9);
	gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
							GPIO9);
	gpio_set(GPIOB, GPIO9);
    // Enable SPI periperal clock
    rcc_periph_clock_enable(RCC_SPI2);

    // Initialize SPI2 as master
    spi_init_master(
        SPI2,
        SPI_CR1_BAUDRATE_FPCLK_DIV_2,
        SPI_CR1_CPOL_CLK_TO_0_WHEN_IDLE,   // CPOL: Clock low when idle
        SPI_CR1_CPHA_CLK_TRANSITION_1,     // CPHA: Clock phase: read on rising edge of clock
        SPI_CR1_DFF_8BIT,
        SPI_CR1_MSBFIRST);

    spi_set_full_duplex_mode(SPI2);
    spi_disable_crc(SPI2);
    spi_enable_software_slave_management(SPI2);
    spi_set_nss_high(SPI2);
    SPI_I2SCFGR(SPI2) &= ~SPI_I2SCFGR_I2SMOD;
    // Have SPI peripheral manage NSS pin (pulled low when SPI enabled)
    spi_enable_ss_output(SPI2);
    gpio_set(GPIOB, GPIO9);
    spi_enable(SPI2);
}


void tftdma(void) {
     rcc_periph_clock_enable(RCC_SYSCFG);
    rcc_periph_clock_enable(RCC_DMA1);
	nvic_enable_irq(NVIC_DMA1_STREAM4_IRQ);
	dma_stream_reset(DMA1, DMA_STREAM4);
	dma_set_priority(DMA1, DMA_STREAM4, DMA_SxCR_PL_HIGH);
	dma_set_memory_size(DMA1, DMA_STREAM4, DMA_SxCR_MSIZE_8BIT);
	dma_set_peripheral_size(DMA1, DMA_STREAM4, DMA_SxCR_PSIZE_8BIT);
	dma_enable_memory_increment_mode(DMA1, DMA_STREAM4);
	dma_disable_peripheral_increment_mode(DMA1, DMA_STREAM4);
//	dma_enable_circular_mode(DMA1, DMA_STREAM4);
	dma_set_transfer_mode(DMA1, DMA_STREAM4, DMA_SxCR_DIR_MEM_TO_PERIPHERAL);
	dma_set_peripheral_address(DMA1, DMA_STREAM4, (uint32_t) &SPI_DR(SPI2));
	dma_set_memory_address(DMA1, DMA_STREAM4, (uint32_t) 0);
	dma_set_number_of_data(DMA1, DMA_STREAM4, 0);
//	dma_set_memory_address(DMA1, DMA_STREAM4, (uint32_t) tfttx);
//	dma_set_number_of_data(DMA1, DMA_STREAM4, 1024);
//	dma_enable_half_transfer_interrupt(DMA1, DMA_STREAM4);
//	dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM4);
	
//	dma_enable_stream(DMA1, DMA_STREAM4); 
//	spi_enable_tx_dma(SPI2);
//    nvic_enable_irq(NVIC_DMA1_STREAM4_IRQ);
    dma_enable_transfer_complete_interrupt(DMA1, DMA_STREAM4);
    dma_channel_select(DMA1, DMA_STREAM4, DMA_SxCR_CHSEL_0);
    dma_enable_stream(DMA1, DMA_STREAM4);
}
 
void dma1_stream4_isr(void) {
//printf("start of dma irq\n");
if (dma_get_interrupt_flag(DMA1, DMA_STREAM4, DMA_TCIF)) {
    dma_clear_interrupt_flags(DMA1, DMA_STREAM4, DMA_TCIF);
    }
    while (SPI_SR(SPI2) & SPI_SR_BSY);
        ;

        // Turn our DMA channel back off, in preparation of the next transfer
        spi_disable_tx_dma(SPI2);
        dma_disable_stream(DMA1, DMA_STREAM4);
        ST_CS_IDLE;
//printf("end of dma irq\n");        
    
}


void dma_start(void *tfttx, size_t data_size) {
    // Note - manipulating the memory address/size of the DMA controller cannot
    // be done while the channel is enabled. Ensure any previous transfer has
    // completed and the channel is disabled before you start another transfer.
    // Tell the DMA controller to start reading memory data from this address
    
    dma_set_memory_address(DMA1, DMA_STREAM4, (uint32_t) tfttx);
    // Configure the number of bytes to transfer
    dma_set_number_of_data(DMA1, DMA_STREAM4, data_size);
//    printf("start of dma_start datasize = %d\n", data_size);
    ST_DC_DAT;
    // Since we're manually controlling our register clock, move it low now
	ST_CS_ACTIVE;
    // Enable the DMA channel.
	dma_enable_stream(DMA1, DMA_STREAM4);
    // Finally, enable SPI DMA transmit. This call is what actually starts the
    // DMA transfer.
    spi_enable_tx_dma(SPI2);
	for (unsigned i = 0; i < 40000; i++)
	  {
		__asm__("nop");
	  }
//    printf("middle of dma_start\n");
//    printf("middle of dma_start\n");
//    while(DMA_SCR(DMA1, DMA_STREAM4) & DMA_SxCR_EN) {
//    ;
//    }
    
//    while (DMA_SNDTR(SPI2, DMA_STREAM4)) {
//    ;
//    }
    
//    while (SPI_SR(SPI2) & SPI_SR_BSY) {
//        ;
//    }
       
//       spi_disable_tx_dma(SPI2);
       dma_disable_stream(DMA1, DMA_STREAM4);
       ST_CS_IDLE;    
//     printf("end of dma_start\n");
}

static void gpio_setup(void)
{
    // Enable GPIOB clock
    rcc_periph_clock_enable(RCC_GPIOB);

    // TFT Reset pin, pull low for 10uS or more to initiate reset
    // Reset takes up to 120ms
 //   gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ST_RST);
 //   gpio_set(GPIOB, ST_RST);

    // TFT A0 - D/C pin, 0:Command 1:Data
    gpio_mode_setup(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, ST_DC);
    gpio_clear(GPIOB, ST_DC);
}

/**
 * Initialize the display driver
 */
void st_init()
{
    rcc_periph_clock_enable(RCC_SPI2);
	// Set gpio clock
	ST_CONFIG_GPIO_CLOCK();
	// Configure gpio output dir and mode
	gpio_setup();
	// If using DMA, config SPI DMA
//	#ifdef ST_USE_SPI_DMA
//		ST_CONFIG_SPI_DMA();
//	#endif
	// Configure SPI settings
	spi_init();

	#ifdef ST_HAS_CS
		ST_CS_ACTIVE;
	#endif

	// Hardwae reset is not mandatory if software rest is done

/*
	_st_write_command_16bit(ST7789_SWRESET);	//1: Software reset, no args, w/delay: delay(150)
	_st_fixed_delay();

	_st_write_command_16bit(ST7789_SLPOUT);	// 2: Out of sleep mode, no args, w/delay: delay(500)
	_st_fixed_delay();

	_st_write_command_16bit(ST7789_COLMOD);	// 3: Set color mode, 1 arg, delay: delay(10)
	_st_write_data_8bit(ST7789_COLOR_MODE_65K | ST7789_COLOR_MODE_16BIT);	// 65K color, 16-bit color
	_st_fixed_delay();

	_st_write_command_16bit(ST7789_MADCTL);	// 4: Memory access ctrl (directions), 1 arg:
	_st_write_data_8bit(ST7789_MADCTL_RGB);	// RGB Color
//	_st_write_data_8bit(ST7789_MADCTL_MY | ST7789_MADCTL_MX | ST7789_MADCTL_RGB);
//			st_tftheight = 240;
//			st_tftwidth = 240;
	_st_write_command_16bit(ST7789_INVON);	// 5: Inversion ON (but why?) delay(10)
	_st_fixed_delay();

	_st_write_command_16bit(ST7789_NORON);	// 6: Normal display on, no args, w/delay: delay(10)
	_st_fixed_delay();


	_st_write_command_16bit(ST7789_DISPON);	// 7: Main screen turn on, no args, w/delay: delay(500)
	_st_fixed_delay();
	
	st_rotate_display(3);
*/
//_st_write_command_16bit(0x01);
delay(17);
    _st_write_command_16bit(0x01); // SW reset
    delay(120);

    _st_write_command_16bit(0x11); // Sleep out, also SW reset
    delay(120);

    _st_write_command_16bit(0x3A);
    
      _st_write_data_16bit(0x55);           // 16 bit colour interface
  
  //    _st_write_data_16bit(0x66);           // 18 bit colour interface


    _st_write_command_16bit(0xC2);
    _st_write_data_16bit(0x44);

    _st_write_command_16bit(0xC5);
    _st_write_data_16bit(0x00);
    _st_write_data_16bit(0x00);
    _st_write_data_16bit(0x00);
    _st_write_data_16bit(0x00);

    _st_write_command_16bit(0xE0);
    _st_write_data_16bit(0x0F);
    _st_write_data_16bit(0x1F);
    _st_write_data_16bit(0x1C);
    _st_write_data_16bit(0x0C);
    _st_write_data_16bit(0x0F);
    _st_write_data_16bit(0x08);
    _st_write_data_16bit(0x48);
    _st_write_data_16bit(0x98);
    _st_write_data_16bit(0x37);
    _st_write_data_16bit(0x0A);
    _st_write_data_16bit(0x13);
    _st_write_data_16bit(0x04);
    _st_write_data_16bit(0x11);
    _st_write_data_16bit(0x0D);
    _st_write_data_16bit(0x00);
 
    _st_write_command_16bit(0xE1);
    _st_write_data_16bit(0x0F);
    _st_write_data_16bit(0x32);
    _st_write_data_16bit(0x2E);
    _st_write_data_16bit(0x0B);
    _st_write_data_16bit(0x0D);
    _st_write_data_16bit(0x05);
    _st_write_data_16bit(0x47);
    _st_write_data_16bit(0x75);
    _st_write_data_16bit(0x37);
    _st_write_data_16bit(0x06);
    _st_write_data_16bit(0x10);
    _st_write_data_16bit(0x03);
    _st_write_data_16bit(0x24);
    _st_write_data_16bit(0x20);
    _st_write_data_16bit(0x00);
 
 
      _st_write_command_16bit(TFT_INVOFF);
  
  //    _st_write_command_16bit(TFT_INVON);
   
 
    _st_write_command_16bit(0x36);
    _st_write_data_16bit(0x28);
    _st_write_data_16bit(0x00);
    _st_write_command_16bit(0x29);                     // display on
    delay(150);
//    st_rotate_display(2);
 
}

void _st_fixed_delay()
{
	for (uint16_t i = 0; i < 5000; i++)
		__asm__("nop");
}

