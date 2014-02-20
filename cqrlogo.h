/*
 * (C) 2013-2014 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#ifndef _CQRLOGO_H
#define _CQRLOGO_H

/* a bitmap */
struct bitmap_t {
	unsigned int width;
	unsigned int height;
	uint8_t *pixel;
};

#if defined PNG_TEXT_SUPPORTED && PNG_ENABLE_TEXT
/*** add_png_text ***/
png_text * add_png_text(png_text *pngtext, unsigned int *textcount, char *key, char *text);
#endif

#if HAVE_FCGI
/*** png_write_stdout ***/
void png_write_stdout(png_structp png_ptr, png_bytep data, png_size_t length);

/*** png_flush_stdout ***/
void png_flush_stdout(png_structp png_ptr);
#endif

/*** generate_png ***/
int generate_png (struct bitmap_t *bitmap, const char *uri);

/*** bitmap_new ***/
struct bitmap_t * bitmap_new(int width, int height);
/*** bitmap_free ***/
void bitmap_free(struct bitmap_t * bitmap);

/*** encode_qrcode ***/
struct bitmap_t * encode_qrcode (const char *text, unsigned int scale,
		unsigned int border, unsigned int level);

/*** get_query_value ***/
unsigned int get_query_value(const char *query_string, const char *pattern,
		unsigned int value, unsigned int min, unsigned int max);

/*** get_ini_value ***/
unsigned int get_ini_value(dictionary * ini, uint8_t type, const char * section, const char * parameter,
		unsigned int value, unsigned int min, unsigned int max);

#endif /* _CQRLOGO_H */

// vim: set syntax=c:
