/*
 * (C) 2013-2014 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#ifndef _LIBCQRLOGO_H
#define _LIBCQRLOGO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdbool.h>

#include <png.h>
#include <zlib.h>
#include <qrencode.h>
#include <iniparser.h>

/* a bitmap */
struct bitmap_t {
	unsigned int width;
	unsigned int height;
	uint8_t *pixel;
};

/* finished PNG image */
struct png_t {
	unsigned char * buffer;
	size_t size;
};

/* config */
struct cqrconf_t {
	uint8_t scale;
	uint8_t border;
	uint8_t level;
	bool overwrite;
};

#define CQR_COMMENT	0x1
#define CQR_REFERER	0x2
#define CQR_VERSION	0x4
#define CQR_LIBVERSION	0x8

#define VERSIONSTR	VERSION " (" __DATE__ ", " __TIME__ ")"
#define LIBSSTR		"libqrencode %s, libpng %s, zlib %s"

/*** png_write_stdout ***/
void png_write_fn(png_structp png_ptr, png_bytep data, png_size_t length);

#if defined PNG_TEXT_SUPPORTED
/*** add_png_text ***/
png_text * add_png_text(png_text *pngtext, unsigned int *textcount, char *key, char *text);
#endif

/*** generate_png ***/
struct png_t * generate_png (struct bitmap_t *bitmap, const uint8_t meta, const char *uri);

/*** bitmap_new ***/
struct bitmap_t * bitmap_new(int width, int height);
/*** bitmap_free ***/
void bitmap_free(struct bitmap_t * bitmap);

/*** encode_qrcode ***/
struct bitmap_t * encode_qrcode (const char *text, const struct cqrconf_t);

/*** get_query_value ***/
unsigned int get_query_value(const char *query_string, const char *pattern,
	unsigned int value, unsigned int min, unsigned int max);
/*** get_ini_value ***/
unsigned int get_ini_value(dictionary * ini, uint8_t type, const char * section, const char * parameter,
	unsigned int value, unsigned int min, unsigned int max);

/*** cqrconf_file ***/
void cqrconf_file(const char * server_name, struct cqrconf_t * cqrconf);
/*** cqrconf_string ***/
void cqrconf_string(const char * query_string, struct cqrconf_t * cqrconf);

#endif /* _LIBCQRLOGO_H */

// vim: set syntax=c:
