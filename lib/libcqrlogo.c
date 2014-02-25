/*
 * (C) 2013-2014 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include "../config.h"
#include "../version.h"

/* define structs and functions */
#include "libcqrlogo.h"

/*** png_write_stdout ***/
void png_write_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
	struct png_t * png;

	png = (struct png_t *)png_get_io_ptr(png_ptr);

	png->buffer = realloc(png->buffer, png->size + length);

  	memcpy(png->buffer + png->size, data, length);

	png->size += length;
}

#if defined PNG_TEXT_SUPPORTED
/*** add_png_text ***/
png_text * add_png_text(png_text *pngtext, unsigned int *textcount, char *key, char *text) {
	pngtext = realloc(pngtext, ((*textcount) + 1) * sizeof(png_text));

	pngtext[*textcount].compression = PNG_TEXT_COMPRESSION_zTXt;
	pngtext[*textcount].key = key;
	pngtext[*textcount].text = text;

	(*textcount)++;
	return pngtext;
}
#endif

/*** generate_png ***/
struct png_t * generate_png (struct bitmap_t *bitmap, const uint8_t meta, const char *uri) {
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_byte ** row_pointers = NULL;
	unsigned int x, y;
	uint8_t bit, byte;
	struct png_t * png;

	png = malloc(sizeof(struct png_t));
	png->buffer = NULL;
	png->size = 0;

	if ((png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
		return NULL;

	if ((info_ptr = png_create_info_struct (png_ptr)) == NULL ||
			(setjmp (png_jmpbuf (png_ptr)))) {
		png_destroy_write_struct (&png_ptr, &info_ptr);
		return NULL;
	}

	png_set_IHDR (png_ptr, info_ptr, bitmap->width, bitmap->height, 1 /* depth */,
		PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* use best compression */
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	/* use compression strategy filtered
	 * this way pngcrush can not optimize any more */
	png_set_compression_strategy(png_ptr, Z_FILTERED);

#if defined PNG_TEXT_SUPPORTED
	unsigned int textcount = 0;
	png_text *pngtext = NULL;
	char *curi = NULL, *libsstr = NULL, *qrver;

	if (meta & CQR_COMMENT)
		pngtext = add_png_text(pngtext, &textcount, "comment", "QR-Code created by cqrlogo - https://github.com/eworm-de/cqrlogo");

	if (meta & CQR_REFERER) {
		curi = strdup(uri);

		/* text in png file may have a max length of 79 chars */
		if (strlen(curi) > 79)
			sprintf(curi + 76, "...");

		pngtext = add_png_text(pngtext, &textcount, "referer", curi);
	}

	if (meta & CQR_VERSION)
		pngtext = add_png_text(pngtext, &textcount, "version", VERSIONSTR);

	if (meta & CQR_LIBVERSION) {
		qrver = QRcode_APIVersionString();

		libsstr = malloc(sizeof(LIBSSTR) + strlen(qrver) + strlen(png_libpng_ver) + strlen(zlib_version));
		sprintf(libsstr, LIBSSTR, qrver, png_libpng_ver, zlib_version);

		pngtext = add_png_text(pngtext, &textcount, "libs", libsstr);
	}

	png_set_text(png_ptr, info_ptr, pngtext, textcount);
	png_free (png_ptr, pngtext);

	if (curi)
		free(curi);
	if (libsstr)
		free(libsstr);
#endif

	row_pointers = png_malloc (png_ptr, bitmap->height * sizeof (png_byte *));
	for (y = 0; y < bitmap->height; ++y) {
		/* we need to round up, need a complete byte for less than eight bits */
		row_pointers[y] = png_malloc (png_ptr, (sizeof(uint8_t) * bitmap->width + 7) / 8);
		for (x = 0; x < bitmap->width; ++x) {
			/* bit are written in reverse order! */
			bit = 7 - (x % 8);
			byte = x / 8;
			if (bitmap->pixel[y * bitmap->width + x])
				row_pointers[y][byte] |= 1 << (bit);
			else
				row_pointers[y][byte] &= ~(1 << (bit));
		}
	}

	/* with FastCGI we can not just open stdout for writing...
	 * define a write function instead */
	png_set_write_fn(png_ptr, png, png_write_fn, NULL);

	png_set_rows (png_ptr, info_ptr, row_pointers);
	png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	for (y = 0; y < bitmap->height; ++y)
		png_free (png_ptr, row_pointers[y]);
	png_free (png_ptr, row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return png;
}

/*** bitmap_new ***/
struct bitmap_t * bitmap_new(int width, int height) {
	struct bitmap_t *bitmap;

	if ((bitmap = malloc(sizeof(struct bitmap_t))) == NULL)
		return NULL;

	bitmap->width = width;
	bitmap->height = height;
	if ((bitmap->pixel = malloc(width * height * sizeof(uint8_t))) == NULL) {
		free(bitmap);
		return NULL;
	}

	/* initialize with white */
	memset(bitmap->pixel, 0xff, width * height);

	return bitmap;
}

/*** bitmap_free ***/
void bitmap_free(struct bitmap_t * bitmap) {
	free(bitmap->pixel);
	free(bitmap);
}

/*** encode_qrcode ***/
struct bitmap_t * encode_qrcode (const char *text, const struct cqrconf_t cqrconf) {
	QRcode *qrcode;
	struct bitmap_t *bitmap, *scaled;
	int i, j, k, l;
	unsigned char *data;

	qrcode = QRcode_encodeString8bit(text, 0, cqrconf.level);

	/* this happens if the string is too long
	 * possibly we have an URL (referer) that is too long, so the code
	 * automatically falls back to http_server (see main()) */
	if (qrcode == NULL)
		return NULL;

	data = qrcode->data;

	/* wirte QR code to bitmap */
	if ((bitmap = bitmap_new(qrcode->width + cqrconf.border * 2, qrcode->width + cqrconf.border * 2)) == NULL)
		return NULL;
	for (i = cqrconf.border; i < qrcode->width + cqrconf.border; i++)
		for (j = cqrconf.border; j < qrcode->width + cqrconf.border; j++) {
			bitmap->pixel[i * (qrcode->width + cqrconf.border * 2) + j] = !(*data & 0x1) * 0xff;
			data++;
		}

	QRcode_free(qrcode);

	if (cqrconf.scale == 1)
		return bitmap;

	/* cqrconf.scale bitmap */
	if ((scaled = bitmap_new(bitmap->width * cqrconf.scale, bitmap->height * cqrconf.scale)) == NULL)
		return NULL;
	for (i = 0; i < bitmap->height; i++)
		for (j = 0; j < bitmap->width; j++)
			for (k = 0; k < cqrconf.scale; k++)
				for (l = 0; l < cqrconf.scale; l++)
					scaled->pixel[i * bitmap->width * cqrconf.scale * cqrconf.scale + k * bitmap->width * cqrconf.scale + j * cqrconf.scale + l] =
						bitmap->pixel[i * bitmap->width + j];


	bitmap_free(bitmap);

	return scaled;
}

/*** get_query_value ***/
unsigned int get_query_value(const char *query_string, const char *pattern,
		unsigned int value, unsigned int min, unsigned int max) {
	char *match = NULL, *newpattern = NULL;
	unsigned int length;
	int tmp = -1;

	newpattern = strdup(pattern);

	length = strlen(newpattern);
	/* length is without null termination, allocacte 4 bytes so we
	 * have "=", "%u" and null termination */
	newpattern = realloc(newpattern, length + 4);
	sprintf(newpattern + length, "=");

	if ((match = strstr(query_string, newpattern)) != NULL) {
		sprintf(newpattern + length + 1, "%%u");

		if ((sscanf(match, newpattern, &tmp)) > 0)
			if (tmp >= min && tmp <= max)
				value = tmp;
	}

	free(newpattern);

	return value;
}

/*** get_ini_value ***/
unsigned int get_ini_value(dictionary * ini, uint8_t type, const char * section, const char * parameter,
		unsigned int value, unsigned int min, unsigned int max) {
	char * key;
	unsigned int tmp;

	key = malloc(strlen(section) + strlen(parameter) + 2);
	sprintf(key, "%s:%s", section, parameter);

	if (type)
		tmp = iniparser_getint(ini, key, value);
	else
		tmp = iniparser_getboolean(ini, key, value);

	if (tmp >= min && tmp <= max)
		value = tmp;

	free(key);

	return value;
}

/*** cqrconf_file ***/
void cqrconf_file(const char * server_name, struct cqrconf_t * cqrconf) {
	dictionary * ini;

	/* parse config file */
	if ((ini = iniparser_load(CONFIGFILE)) == NULL) {
		fprintf(stderr, "cannot parse file " CONFIGFILE ", continue anyway\n");
		return;
	}

	cqrconf->scale = get_ini_value(ini, 1, "general", "scale", cqrconf->scale, 1, QRCODE_MAX_SCALE);
	cqrconf->border = get_ini_value(ini, 1, "general", "border", cqrconf->border, 0, QRCODE_MAX_BORDER);
	cqrconf->level = get_ini_value(ini, 1, "general", "level", cqrconf->level, QR_ECLEVEL_L, QR_ECLEVEL_H);
	cqrconf->overwrite = get_ini_value(ini, 0, "general", "allow overwrite", cqrconf->overwrite, false, true);

	cqrconf->scale = get_ini_value(ini, 1, server_name, "scale", cqrconf->scale, 1, QRCODE_MAX_SCALE);
	cqrconf->border = get_ini_value(ini, 1, server_name, "border", cqrconf->border, 0, QRCODE_MAX_BORDER);
	cqrconf->level = get_ini_value(ini, 1, server_name, "level", cqrconf->level, QR_ECLEVEL_L, QR_ECLEVEL_H);
	cqrconf->overwrite = get_ini_value(ini, 0, server_name, "allow overwrite", cqrconf->overwrite, false, true);

	/* done reading config file, free */
	iniparser_freedict(ini);
}

/*** cqrconf_string ***/
void cqrconf_string(const char * query_string, struct cqrconf_t * cqrconf) {
	if (cqrconf->overwrite == false)
		return;

	if (query_string == NULL)
		return;

	/* do we have a special scale? */
	cqrconf->scale = get_query_value(query_string, "scale", cqrconf->scale, 1, QRCODE_MAX_SCALE);

	/* width of the border? */
	cqrconf->border = get_query_value(query_string, "border", cqrconf->border, 0, QRCODE_MAX_BORDER);

	/* error correction level? */
	cqrconf->level = get_query_value(query_string, "level", cqrconf->level, QR_ECLEVEL_L, QR_ECLEVEL_H);
}
