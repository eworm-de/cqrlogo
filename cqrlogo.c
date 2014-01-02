/*
 * (C) 2013-2014 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <regex.h>

#include <iniparser.h>
#include <png.h>
#include <zlib.h>
#include <qrencode.h>

#include "config.h"
#include "version.h"

/* define structs and functions */
#include "cqrlogo.h"

#define URLPATTERN "^[hH][tT][tT][pP][sS]\\?://%s/"
#define TEXTSTOLEN "This QR Code has been stolen from http%s://%s/!"

#if defined PNG_TEXT_SUPPORTED && PNG_ENABLE_TEXT
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
int generate_png (struct bitmap_t *bitmap, const char *uri) {
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_byte ** row_pointers = NULL;
	unsigned int x, y;
	uint8_t bit, byte;

	if ((png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
		return 1;

	if ((info_ptr = png_create_info_struct (png_ptr)) == NULL ||
			(setjmp (png_jmpbuf (png_ptr)))) {
		png_destroy_write_struct (&png_ptr, &info_ptr);
		return 1;
	}

	png_set_IHDR (png_ptr, info_ptr, bitmap->width, bitmap->height, 1 /* depth */,
		PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* use best compression */
	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

	/* use compression strategy filtered
	 * this way pngcrush can not optimize any more */
	png_set_compression_strategy(png_ptr, Z_FILTERED);

#if defined PNG_TEXT_SUPPORTED && PNG_ENABLE_TEXT
	unsigned int textcount = 0;
	png_text *pngtext = NULL;

	pngtext = add_png_text(pngtext, &textcount, "comment", "QR-Code created by cqrlogo - https://github.com/eworm-de/cqrlogo");
#	if PNG_ENABLE_TEXT_REFERER
	pngtext = add_png_text(pngtext, &textcount, "referer", (char *)uri);
#	endif

#	if PNG_ENABLE_TEXT_VERSIONS
#	define VERSIONSTR	VERSION " (" __DATE__ ", " __TIME__ ")"
#	define LIBSSTR		"libqrencode %s, libpng %s, zlib %s"
	char *libsstr, *qrver = QRcode_APIVersionString();

	libsstr = malloc(sizeof(LIBSSTR) + strlen(qrver) + strlen(png_libpng_ver) + strlen(zlib_version));
	sprintf(libsstr, LIBSSTR, qrver, png_libpng_ver, zlib_version);

	pngtext = add_png_text(pngtext, &textcount, "version", VERSIONSTR);
	pngtext = add_png_text(pngtext, &textcount, "libs", libsstr);
#	endif

	png_set_text(png_ptr, info_ptr, pngtext, textcount);
	png_free (png_ptr, pngtext);
#	if PNG_ENABLE_TEXT_VERSIONS
	free(libsstr);
#	endif
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

	png_init_io (png_ptr, stdout);
	png_set_rows (png_ptr, info_ptr, row_pointers);
	png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	for (y = 0; y < bitmap->height; ++y)
		png_free (png_ptr, row_pointers[y]);
	png_free (png_ptr, row_pointers);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	fclose(stdout);

	return 0;
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
struct bitmap_t * encode_qrcode (const char *text, unsigned int scale,
		unsigned int border, unsigned int level) {
	QRcode *qrcode;
	struct bitmap_t *bitmap, *scaled;
	int i, j, k, l;
	unsigned char *data;

	qrcode = QRcode_encodeString8bit(text, 0, level);

	/* this happens if the string is too long
	 * possibly we have an URL (referer) that is too long, so the code
	 * automatically falls back to http_server (see main()) */
	if (qrcode == NULL)
		return NULL;

	data = qrcode->data;

	/* wirte QR code to bitmap */
	if ((bitmap = bitmap_new(qrcode->width + border * 2, qrcode->width + border * 2)) == NULL)
		return NULL;
	for (i = border; i < qrcode->width + border; i++)
		for (j = border; j < qrcode->width + border; j++) {
			bitmap->pixel[i * (qrcode->width + border * 2) + j] = !(*data & 0x1) * 0xff;
			data++;
		}

	QRcode_free(qrcode);

	if (scale == 1)
		return bitmap;

	/* scale bitmap */
	if ((scaled = bitmap_new(bitmap->width * scale, bitmap->height * scale)) == NULL)
		return NULL;
	for (i = 0; i < bitmap->height; i++)
		for (j = 0; j < bitmap->width; j++)
			for (k = 0; k < scale; k++)
				for (l = 0; l < scale; l++)
					scaled->pixel[i * bitmap->width * scale * scale + k * bitmap->width * scale + j * scale + l] =
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

/*** main ***/
int main(int argc, char **argv) {
	dictionary * ini;
	const char * http_referer, * server_name, * query_string, * uri;
	char * uri_server_name = NULL, * uri_png = NULL, * pattern = NULL, * stolen = NULL;
	regex_t preg;
	regmatch_t pmatch[1];
	uint8_t https = 0, overwrite = ALLOW_OVERWRITE;

	struct bitmap_t * bitmap;
	unsigned int scale = QRCODE_SCALE, border = QRCODE_BORDER, level = QRCODE_LEVEL;

	/* check if we have environment variables from CGI */
	if ((server_name = getenv("SERVER_NAME")) == NULL) {
		fprintf(stderr, "This is a CGI executable. Running without a web service is not supported.\n"
				"Note that SERVER_NAME needs to be defined, for full features the client has\n"
				"to send referer information.\n");
		return EXIT_FAILURE;
	}

	/* check if we have https connection */
	if (getenv("HTTPS") != NULL)
		https = 1;

	/* assemble uri for use when referer is missing or fails */
	uri_server_name = malloc(10 + strlen(server_name));
	sprintf(uri_server_name, "http%s://%s/", https ? "s" : "", server_name);

	/* get http referer */
	if ((http_referer = getenv("HTTP_REFERER")) != NULL) {
		uri = http_referer;

		/* prepare pattern matching */
		pattern = malloc(sizeof(URLPATTERN) + strlen(server_name));
		sprintf(pattern, URLPATTERN, server_name);
		if (regcomp(&preg, pattern, 0) != 0) {
			fprintf(stderr, "regcomp() failed, returning nonzero\n");
			return EXIT_FAILURE;
		}

		/* check if the QR-Code is for the correct server */
		if (regexec(&preg, http_referer, 1, pmatch, 0) != 0) {
			stolen = malloc(sizeof(TEXTSTOLEN) + strlen(server_name));
			sprintf(stolen, TEXTSTOLEN, https ? "s" : "", server_name);
			uri = stolen;
		}

		regfree(&preg);
		free(pattern);
	} else {
		/* use uri assembled from server name */
		uri = uri_server_name;
	}

	/* parse config file */
	if ((ini = iniparser_load(CONFIGFILE)) == NULL) {
		fprintf(stderr, "cannot parse file " CONFIGFILE ", continue anyway\n");
	/* continue anyway, there is nothing essential in the config file */
	} else {
		scale = get_ini_value(ini, 1, "general", "scale", scale, 1, QRCODE_MAX_SCALE);
		border = get_ini_value(ini, 1, "general", "border", border, 0, QRCODE_MAX_BORDER);
		level = get_ini_value(ini, 1, "general", "level", level, QR_ECLEVEL_L, QR_ECLEVEL_H);
		overwrite = get_ini_value(ini, 0, "general", "allow overwrite", overwrite, 0, 1);

		scale = get_ini_value(ini, 1, server_name, "scale", scale, 1, QRCODE_MAX_SCALE);
		border = get_ini_value(ini, 1, server_name, "border", border, 0, QRCODE_MAX_BORDER);
		level = get_ini_value(ini, 1, server_name, "level", level, QR_ECLEVEL_L, QR_ECLEVEL_H);
		overwrite = get_ini_value(ini, 0, server_name, "allow overwrite", overwrite, 0, 1);

		/* done reading config file, free */
		iniparser_freedict(ini);
	}

	/* get query string and read settings */
	if (overwrite && (query_string = getenv("QUERY_STRING")) != NULL) {
		/* do we have a special scale? */
		scale = get_query_value(query_string, "scale", scale, 1, QRCODE_MAX_SCALE);

		/* width of the border? */
		border = get_query_value(query_string, "border", border, 0, QRCODE_MAX_BORDER);

		/* error correction level? */
		level = get_query_value(query_string, "level", level, QR_ECLEVEL_L, QR_ECLEVEL_H);
	}

	/* encode the QR-Code */
	if ((bitmap = encode_qrcode(uri, scale, border, level)) == NULL) {
		/* uri too long? retry with uri from server name */
		uri = uri_server_name;
		if ((bitmap = encode_qrcode(uri, scale, border, level)) == NULL) {
			fprintf(stderr, "Could not generate QR-Code.\n");
			return EXIT_FAILURE;
		}
	}

	/* print HTTP header */
	printf("Content-Type: image/png\n\n");

	/* cut uri, text in png file may have a max length of 79 chars */
	if (strlen(uri) > 79) {
		uri_png = strdup(uri);
		sprintf(uri_png + 76, "...");
		uri = uri_png;
	}

	/* print PNG data */
	if (generate_png(bitmap, uri)) {
		fprintf(stderr, "Failed to generate PNG.\n");
		return EXIT_FAILURE;
	}

	/* free memory we no longer need */
	if (uri_server_name)
		free(uri_server_name);
	if (stolen)
		free(stolen);
	if (uri_png)
		free(uri_png);
	bitmap_free(bitmap);

	return EXIT_SUCCESS;
}

// vim: set syntax=c:
