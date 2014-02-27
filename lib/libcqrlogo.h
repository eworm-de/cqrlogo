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
struct cqr_bitmap {
	unsigned int width;
	unsigned int height;
	uint8_t *pixel;
};

/* finished PNG image */
struct cqr_png {
	unsigned char * buffer;
	size_t size;
};

/* config */
struct cqr_conf {
	uint8_t scale;
	uint8_t border;
	uint8_t level;
	bool overwrite;
};

const char * cqr_mimeheader = "Content-Type: image/png\n\n";

#define CQR_META_COMMENT	0x1
#define CQR_META_REFERER	0x2
#define CQR_META_VERSION	0x4
#define CQR_META_LIBVERSION	0x8
#define CQR_META_ALL		CQR_META_COMMENT + CQR_META_REFERER + CQR_META_VERSION + CQR_META_LIBVERSION

#define CQR_COMMENTSTR	"QR-Code created by cqrlogo - https://github.com/eworm-de/cqrlogo"
#define CQR_VERSIONSTR	VERSION " (" __DATE__ ", " __TIME__ ")"
#define CQR_LIBSSTR	"libqrencode %s, libpng %s, zlib %s"

/*** cqr_png_write_fn ***/
void cqr_png_write_fn(png_structp png_ptr, png_bytep data, png_size_t length);

#if defined PNG_TEXT_SUPPORTED
/*** cqr_png_add_text ***/
png_text * cqr_png_add_text(png_text *pngtext, unsigned int *textcount, char *key, char *text);
#endif

/*** cqr_bitmap_new ***/
struct cqr_bitmap * cqr_bitmap_new(int width, int height);
/*** cqr_bitmap_free ***/
void cqr_bitmap_free(struct cqr_bitmap * bitmap);

/*** cqr_encode_qrcode_to_bitmap ***/
struct cqr_bitmap * cqr_encode_qrcode_to_bitmap(const char *text, const struct cqr_conf conf);
/*** cqr_bitmap_to_png ***/
struct cqr_png * cqr_bitmap_to_png(struct cqr_bitmap *bitmap, const char *text, const uint8_t meta);
/*** cqr_encode_qrcode_to_png ***/
struct cqr_png * cqr_encode_qrcode_to_png(const char *text, const struct cqr_conf conf, const uint8_t meta);

/*** cqr_get_query_value ***/
unsigned int cqr_get_query_value(const char *query_string, const char *pattern,
	unsigned int value, unsigned int min, unsigned int max);
/*** cqr_get_ini_value ***/
unsigned int cqr_get_ini_value(dictionary * ini, uint8_t type, const char * section, const char * parameter,
	unsigned int value, unsigned int min, unsigned int max);

/*** cqr_conf_file ***/
void cqr_conf_file(const char * server_name, struct cqr_conf * conf);
/*** cqr_conf_string ***/
void cqr_conf_string(const char * query_string, struct cqr_conf * conf);

#endif /* _LIBCQRLOGO_H */

// vim: set syntax=c:
