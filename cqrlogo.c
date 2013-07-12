/*
 * (C) 2013 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <regex.h>

#include <png.h>
#include <zlib.h>
#include <qrencode.h>

#include "config.h"

#define URLPATTERN "^[hH][tT][tT][pP][sS]\\?://%s/"
#define TEXTSTOLEN "This QR Code has been stolen from http%s://%s/!"

/* a bitmap */
struct bitmap_t {
	int width;
	int height;
	uint8_t *pixel;
};

/*** generate_png ***/
int generate_png (struct bitmap_t *bitmap, const char *uri) {
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_byte ** row_pointers = NULL;
	int x, y, depth = 8;

	if ((png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL)
		return 1;

	if ((info_ptr = png_create_info_struct (png_ptr)) == NULL ||
			(setjmp (png_jmpbuf (png_ptr)))) {
		png_destroy_write_struct (&png_ptr, &info_ptr);
		return 1;
	}

	png_set_IHDR (png_ptr, info_ptr, bitmap->width, bitmap->height, depth,
		PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	png_set_compression_level(png_ptr, Z_BEST_COMPRESSION);

#ifdef PNG_TEXT_SUPPORTED
#define VERSIONSTR	VERSION " (" __DATE__ ", " __TIME__ ", libpng %s, zlib %s)"
	png_text text[3];
	char *version;
	
	version = malloc(sizeof(VERSIONSTR) + strlen(png_libpng_ver) + strlen(zlib_version));
	sprintf(version, VERSIONSTR, png_libpng_ver, zlib_version);

	text[0].compression = PNG_TEXT_COMPRESSION_zTXt;
	text[0].key = "comment";
	text[0].text = "QR-Code created by cqrlogo - https://github.com/eworm-de/cqrlogo";

	text[1].compression = PNG_TEXT_COMPRESSION_zTXt;
	text[1].key = "version";
	text[1].text = version;

	text[2].compression = PNG_TEXT_COMPRESSION_zTXt;
	text[2].key = "referer";
	text[2].text = (char*)uri;

	png_set_text(png_ptr, info_ptr, text, 3);
	free(version);
#endif

	row_pointers = png_malloc (png_ptr, bitmap->height * sizeof (png_byte *));
	for (y = 0; y < bitmap->height; ++y) {
		png_byte *row = png_malloc (png_ptr, sizeof (uint8_t) * bitmap->width);
		row_pointers[y] = row;
		for (x = 0; x < bitmap->width; ++x) {
			*row++ = bitmap->pixel[y * bitmap->width + x];
		}
	}

	png_init_io (png_ptr, stdout);
	png_set_rows (png_ptr, info_ptr, row_pointers);
	png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

	for (y = 0; y < bitmap->height; y++) {
		png_free (png_ptr, row_pointers[y]);
	}
	png_free (png_ptr, row_pointers);

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
struct bitmap_t * encode_qrcode (const char *text, unsigned int scale, unsigned int border, unsigned int level) {
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

/*** get_value ***/
int get_value(const char *query_string, const char *pattern, unsigned int *value, unsigned int def, unsigned int min, unsigned int max) {
	char *match = NULL, *newpattern = NULL;

	newpattern = strdup(pattern);

	newpattern = realloc(newpattern, strlen(newpattern) + 2);
	sprintf(newpattern + strlen(newpattern), "=");

	if ((match = strstr(query_string, newpattern)) != NULL) {
		newpattern = realloc(newpattern, strlen(newpattern) + 3);
		sprintf(newpattern + strlen(newpattern), "%%u");

		if ((sscanf(match, newpattern, value)) > 0)
			if (*value < min || *value > max)
				*value = def;
	}

	free(newpattern);

	return EXIT_SUCCESS;
}

/*** main ***/
int main(int argc, char **argv) {
	const char * http_referer, * server_name, * query_string, * uri;
	char * uri_server_name = NULL, * uri_png = NULL, * pattern = NULL, * stolen = NULL;
	regex_t preg;
	regmatch_t pmatch[1];
	unsigned int https = 0;

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

	/* get query string and read settings */
	if ((query_string = getenv("QUERY_STRING")) != NULL) {
		/* do we have a special scale? */
		get_value(query_string, "scale", &scale, QRCODE_SCALE, 1, QRCODE_MAX_SCALE);

		/* width of the border? */
		get_value(query_string, "border", &border, QRCODE_BORDER, 0, QRCODE_MAX_BORDER);

		/* error correction level? */
		get_value(query_string, "level", &level, QRCODE_LEVEL, 0, QR_ECLEVEL_H);
	}

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
	free(bitmap);

	return EXIT_SUCCESS;
}

// vim: set syntax=c:
