/*
 * (C) 2013-2026 by Christian Hesse <mail@eworm.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */

#include "../config.h"
#include "../version.h"

/* define structs and functions */
#include "libcqrlogo.h"

/*** cqr_png_write_fn ***/
void cqr_png_write_fn(png_structp png_ptr, png_bytep data, png_size_t length) {
	struct cqr_png * png;

	png = (struct cqr_png *)png_get_io_ptr(png_ptr);

	png->buffer = realloc(png->buffer, png->size + length);

  	memcpy(png->buffer + png->size, data, length);

	png->size += length;
}

#if defined PNG_TEXT_SUPPORTED
/*** cqr_png_add_text ***/
png_text * cqr_png_add_text(png_text *pngtext, unsigned int *textcount, char *key, char *text) {
	pngtext = realloc(pngtext, ((*textcount) + 1) * sizeof(png_text));

	pngtext[*textcount].compression = PNG_TEXT_COMPRESSION_zTXt;
	pngtext[*textcount].key = key;
	pngtext[*textcount].text = text;

	(*textcount)++;
	return pngtext;
}
#endif

/*** cqr_bitmap_new ***/
struct cqr_bitmap * cqr_bitmap_new(int width, int height) {
	struct cqr_bitmap *bitmap;

	if ((bitmap = malloc(sizeof(struct cqr_bitmap))) == NULL)
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

/*** cqr_bitmap_free ***/
void cqr_bitmap_free(struct cqr_bitmap * bitmap) {
	free(bitmap->pixel);
	free(bitmap);
}

/*** cqr_encode_qrcode_to_bitmap ***/
struct cqr_bitmap * cqr_encode_qrcode_to_bitmap(const char *text, const struct cqr_conf conf) {
	QRcode *qrcode;
	struct cqr_bitmap *bitmap, *scaled;
	int i, j, k, l;
	unsigned char *data;

	qrcode = QRcode_encodeString8bit(text, 0, conf.level);

	/* this happens if the string is too long
	 * possibly we have an URL (referer) that is too long, so the code
	 * automatically falls back to http_server (see main()) */
	if (qrcode == NULL)
		return NULL;

	data = qrcode->data;

	/* wirte QR code to bitmap */
	if ((bitmap = cqr_bitmap_new(qrcode->width + conf.border * 2, qrcode->width + conf.border * 2)) == NULL)
		return NULL;
	for (i = conf.border; i < qrcode->width + conf.border; i++)
		for (j = conf.border; j < qrcode->width + conf.border; j++) {
			bitmap->pixel[i * (qrcode->width + conf.border * 2) + j] = !(*data & 0x1) * 0xff;
			data++;
		}

	QRcode_free(qrcode);

	if (conf.scale == 1)
		return bitmap;

	/* scale bitmap */
	if ((scaled = cqr_bitmap_new(bitmap->width * conf.scale, bitmap->height * conf.scale)) == NULL)
		return NULL;
	for (i = 0; i < bitmap->height; i++)
		for (j = 0; j < bitmap->width; j++)
			for (k = 0; k < conf.scale; k++)
				for (l = 0; l < conf.scale; l++)
					scaled->pixel[i * bitmap->width * conf.scale * conf.scale + k * bitmap->width * conf.scale + j * conf.scale + l] =
						bitmap->pixel[i * bitmap->width + j];


	cqr_bitmap_free(bitmap);

	return scaled;
}

/*** cqr_bitmap_to_png ***/
struct cqr_png * cqr_bitmap_to_png(struct cqr_bitmap *bitmap, const char *text,  const uint8_t meta) {
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_byte ** row_pointers = NULL;
	unsigned int x, y;
	uint8_t bit, byte;
	struct cqr_png * png;

	png = malloc(sizeof(struct cqr_png));
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
	char *referer = NULL, *libsstr = NULL, *qrver;

	if (meta) {
		if (meta & CQR_META_COMMENT)
			pngtext = cqr_png_add_text(pngtext, &textcount, "comment", CQR_COMMENTSTR);

		if (meta & CQR_META_REFERER) {
			referer = strdup(text);

			/* text in png file may have a max length of 79 chars */
			if (strlen(referer) > 79)
				sprintf(referer + 76, "...");

			pngtext = cqr_png_add_text(pngtext, &textcount, "referer", referer);
		}

		if (meta & CQR_META_VERSION)
			pngtext = cqr_png_add_text(pngtext, &textcount, "version", CQR_VERSIONSTR);

		if (meta & CQR_META_LIBVERSION) {
			qrver = QRcode_APIVersionString();

			libsstr = malloc(sizeof(CQR_LIBSSTR) + strlen(qrver) + strlen(png_libpng_ver) + strlen(zlib_version));
			sprintf(libsstr, CQR_LIBSSTR, qrver, png_libpng_ver, zlib_version);

			pngtext = cqr_png_add_text(pngtext, &textcount, "libs", libsstr);
		}

		png_set_text(png_ptr, info_ptr, pngtext, textcount);
		png_free (png_ptr, pngtext);

		if (referer)
			free(referer);
		if (libsstr)
			free(libsstr);
	}
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
	png_set_write_fn(png_ptr, png, cqr_png_write_fn, NULL);

	png_set_rows(png_ptr, info_ptr, row_pointers);
	png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	for (y = 0; y < bitmap->height; ++y)
		png_free(png_ptr, row_pointers[y]);
	png_free(png_ptr, row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return png;
}

/*** cqr_encode_qrcode_to_png ***/
struct cqr_png * cqr_encode_qrcode_to_png(const char *text, const struct cqr_conf conf, const uint8_t meta) {
	struct cqr_bitmap * bitmap;
	struct cqr_png * png;

	if ((bitmap = cqr_encode_qrcode_to_bitmap(text, conf)) == NULL) {
		fprintf(stderr, "Failed encoding QR-Code to bitmap.\n");
		return NULL;
	}
	if ((png = cqr_bitmap_to_png(bitmap, text, meta)) == NULL) {
		fprintf(stderr, "Failed to convert bitmap to png.\n");
		return NULL;
	}

	cqr_bitmap_free(bitmap);

	return png;
}

/*** cqr_get_query_uint ***/
unsigned int cqr_get_query_uint(const char *query_string, const char *pattern,
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

/*** get_query_char ***/
char * cqr_get_query_char(const char *query_string, const char *pattern) {
	char *value = NULL, *cut = NULL;
	const char *tmp =  NULL, *match = NULL;

	if ((match = strstr(query_string, pattern)) == NULL)
		return NULL;

	if ((tmp = strchr(match, '=')) == NULL)
		return NULL;

	if (strlen(tmp) < 1)
		return NULL;

	value = strdup(tmp + 1);

	if ((cut = strchr(value, '&')) != NULL)
		*cut = '\0';

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

/*** cqr_conf_file ***/
void cqr_conf_file(const char * server_name, struct cqr_conf * conf) {
	dictionary * ini;

	/* parse config file */
	if ((ini = iniparser_load(CONFIGFILE)) == NULL) {
		fprintf(stderr, "cannot parse file " CONFIGFILE ", continue anyway\n");
		return;
	}

	conf->scale = get_ini_value(ini, 1, "general", "scale", conf->scale, 1, QRCODE_MAX_SCALE);
	conf->border = get_ini_value(ini, 1, "general", "border", conf->border, 0, QRCODE_MAX_BORDER);
	conf->level = get_ini_value(ini, 1, "general", "level", conf->level, QR_ECLEVEL_L, QR_ECLEVEL_H);
	conf->overwrite = get_ini_value(ini, 0, "general", "allow overwrite", conf->overwrite, false, true);

	conf->scale = get_ini_value(ini, 1, server_name, "scale", conf->scale, 1, QRCODE_MAX_SCALE);
	conf->border = get_ini_value(ini, 1, server_name, "border", conf->border, 0, QRCODE_MAX_BORDER);
	conf->level = get_ini_value(ini, 1, server_name, "level", conf->level, QR_ECLEVEL_L, QR_ECLEVEL_H);
	conf->overwrite = get_ini_value(ini, 0, server_name, "allow overwrite", conf->overwrite, false, true);

	/* done reading config file, free */
	iniparser_freedict(ini);
}

/*** cqr_conf_string ***/
void cqr_conf_string(const char * query_string, struct cqr_conf * conf) {
	if (conf->overwrite == false)
		return;

	if (query_string == NULL)
		return;

	/* do we have a special scale? */
	conf->scale = cqr_get_query_uint(query_string, "scale", conf->scale, 1, QRCODE_MAX_SCALE);

	/* width of the border? */
	conf->border = cqr_get_query_uint(query_string, "border", conf->border, 0, QRCODE_MAX_BORDER);

	/* error correction level? */
	conf->level = cqr_get_query_uint(query_string, "level", conf->level, QR_ECLEVEL_L, QR_ECLEVEL_H);
}
