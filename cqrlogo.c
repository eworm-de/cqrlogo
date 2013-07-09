/*
 * (C) 2013 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#include <gdk-pixbuf/gdk-pixbuf.h>
#include <qrencode.h>

/* pixels are scaled up by this factor */
#define QRCODE_SCALE	2

/* width of the border
 * this is defined to at least 4, but works well with less */
# define QRCODE_BORDER	1

/* error correction level used for QR code
 * possible values: QR_ECLEVEL_L (lowest, about 7% error can be corrected)
 *                  QR_ECLEVEL_M (about 15%)
 *                  QR_ECLEVEL_Q (about 25%)
 *                  QR_ECLEVEL_H (highest, about 30%)
 * image size raises with higher levels */
#define QRCODE_LEVEL	QR_ECLEVEL_L

GdkPixbuf * encode_qrcode (char *text, int scale, int border) {
       QRcode *qrcode;
       GdkPixbuf *pixbuf, *pixbuf_scaled;
       int i, j, k, rowstride, channels;
       guchar *pixel;
       unsigned char *data;
       
       qrcode = QRcode_encodeData(strlen(text), (unsigned char *)text, 0, QRCODE_LEVEL);

       if (qrcode == NULL)
               return NULL;

       data = qrcode->data;

       pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8,
		       qrcode->width + border * 2, qrcode->width + border * 2);

       pixel = gdk_pixbuf_get_pixels (pixbuf);
       rowstride = gdk_pixbuf_get_rowstride (pixbuf);
       channels = gdk_pixbuf_get_n_channels (pixbuf);

       gdk_pixbuf_fill(pixbuf, 0xffffffff);
       for (i = border; i < qrcode->width + border; i++)
               for (j = border; j < qrcode->width + border; j++) {
                       for (k = 0; k < channels; k++)
                               pixel[i * rowstride + j * channels + k] = !(*data & 0x1) * 0xff;
                       data++;
               }

       pixbuf_scaled = gdk_pixbuf_scale_simple (pixbuf,
		       (qrcode->width + border * 2) * scale,
		       (qrcode->width + border * 2) * scale,
		       GDK_INTERP_NEAREST);

       QRcode_free(qrcode);
       g_object_unref(pixbuf);
       
       return pixbuf_scaled;
}

int main(int argc, char **argv) {
	char * http_referer, * server_name, * pattern;
	regex_t preg;
	regmatch_t pmatch[1];
	int rc = 0, referer = 0;
	size_t bytes = 0;

	GdkPixbuf *pixbuf;
	char *match = NULL;
	int scale = QRCODE_SCALE, border = QRCODE_BORDER;

	gchar *buffer;
	gsize size;

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
		if ((rc = regcomp(&preg, pattern, 0)) != 0)
			fprintf(stderr, "regcomp() failed, returning nonzero (%d)\n", rc);

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
			sscanf(match, "scale=%u", &scale);

		/* width of the border? */
		if ((match = strstr(query_string, "border=")) != NULL)
			sscanf(match, "border=%u", &border);
	}

	/* initialize type system for glib < 2.36 */
#ifndef GLIB_VERSION_2_36
	g_type_init();
#endif
	
	if ((pixbuf = encode_qrcode(http_referer, scale, border)) == NULL) {
		if ((pixbuf = encode_qrcode(server_name, scale, border)) == NULL) {
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
	gdk_pixbuf_save_to_buffer (pixbuf, &buffer, &size, "png", NULL,
			"compression", "9",
			"tEXt::comment", "QR-Code created by cqrlogo - https://github.com/eworm-de/cqrlogo",
			"tEXt::referer", http_referer, NULL);
	if ((bytes = fwrite (buffer, 1, size, stdout)) != size)
		fprintf(stderr, "fwrite() failed, wrote %zu of %zu bytes.\n", bytes, (size_t)size);

	if (referer)
		free(http_referer);
	g_object_unref(pixbuf);

	return EXIT_SUCCESS;
}

// vim: set syntax=c:
