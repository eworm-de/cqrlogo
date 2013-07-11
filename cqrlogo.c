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

/* a bitmap */
struct bitmap_t {
	int width;
	int height;
	uint8_t *pixel;
};

/*** generate_png ***/
int generate_png (struct bitmap_t *bitmap, char *http_referer) {
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
	png_text text[3];
	char *version;
	const char *v_libpng = png_libpng_ver, *v_zlib = zlib_version;
	
	version = malloc(18 + strlen(VERSION) + strlen(v_libpng) + strlen(v_zlib));
	sprintf(version, VERSION " (libpng %s, zlib %s)", png_libpng_ver, zlib_version);

	text[0].compression = PNG_TEXT_COMPRESSION_zTXt;
	text[0].key = "comment";
	text[0].text = "QR-Code created by cqrlogo - https://github.com/eworm-de/cqrlogo";

	text[1].compression = PNG_TEXT_COMPRESSION_zTXt;
	text[1].key = "version";
	text[1].text = version;

	text[2].compression = PNG_TEXT_COMPRESSION_zTXt;
	text[2].key = "referer";
	text[2].text = http_referer;

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
struct bitmap_t * encode_qrcode (char *text, unsigned int scale, unsigned int border, unsigned int level) {
       QRcode *qrcode;
       struct bitmap_t *bitmap, *scaled;
       int i, j, k, l;
       unsigned char *data;
       
       qrcode = QRcode_encodeData(strlen(text), (unsigned char *)text, 0, level);

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

int main(int argc, char **argv) {
	char * http_referer, * server_name, * pattern;
	regex_t preg;
	regmatch_t pmatch[1];
	int referer = 0;

	struct bitmap_t * bitmap;
	char *match = NULL;
	unsigned int scale = QRCODE_SCALE, border = QRCODE_BORDER, level = QRCODE_LEVEL;

	/* get query string for later use */
	char * query_string = getenv("QUERY_STRING");

	/* check if we have environment variables from CGI */
	if ((server_name = getenv("SERVER_NAME")) == NULL) {
		fprintf(stderr, "This is a CGI executable. Running without a web service is not supported.\n"
				"Note that SERVER_NAME needs to be defined, for full features the client has\n"
				"to send referer information.\n");
		return EXIT_FAILURE;
	} 
	if ((http_referer = getenv("HTTP_REFERER")) == NULL) {
		http_referer = server_name;
	} else {	
		/* prepare pattern matching */
		pattern = malloc(28 + strlen(server_name));
		sprintf(pattern, "^[hH][tT][tT][pP][sS]\\?://%s/", server_name);
		if (regcomp(&preg, pattern, 0) != 0) {
			fprintf(stderr, "regcomp() failed, returning nonzero\n");
			return EXIT_FAILURE;
		}

		/* check if the QR-Code is for the correct server */
		if ((referer = regexec(&preg, http_referer, 1, pmatch, 0)) != 0) {
			http_referer = malloc(44 + strlen(server_name));
			sprintf(http_referer, "This QR Code has been stolen from %s!", server_name);
		}

		regfree(&preg);
		free(pattern);
	}

	if (query_string ) {
		/* do we have a special scale? */
		if ((match = strstr(query_string, "scale=")) != NULL)
			if ((sscanf(match, "scale=%u", &scale)) > 0)
				if (scale < 1 || scale > QRCODE_MAX_SCALE)
					scale = QRCODE_SCALE;

		/* width of the border? */
		if ((match = strstr(query_string, "border=")) != NULL)
			if ((sscanf(match, "border=%u", &border)) > 0)
				if (border > QRCODE_MAX_BORDER)
					border = QRCODE_BORDER;

		/* error correction level? */
		if ((match = strstr(query_string, "level=")) != NULL)
			if ((sscanf(match, "level=%u", &level)) > 0)
				if (level > QR_ECLEVEL_H)
					level = QRCODE_LEVEL;
	}

	if ((bitmap = encode_qrcode(http_referer, scale, border, level)) == NULL) {
		if ((bitmap = encode_qrcode(server_name, scale, border, level)) == NULL) {
			fprintf(stderr, "Could not generate QR-Code.\n");
			return EXIT_FAILURE;
		}
	}

	/* print HTTP header */
	printf("Content-Type: image/png\n\n");

	/* cut http_referer, text in png file may have a max length of 79 chars */
	if (strlen(http_referer) > 79) {
		if (!referer) {
			http_referer = strdup(http_referer);
			referer++;
		}
		sprintf(http_referer + 76, "...");
	}

	/* print PNG data */
	if (generate_png(bitmap, http_referer)) {
		fprintf(stderr, "Failed to generate PNG.\n");
		return EXIT_FAILURE;
	}

	if (referer)
		free(http_referer);
	free(bitmap);

	return EXIT_SUCCESS;
}

// vim: set syntax=c:
