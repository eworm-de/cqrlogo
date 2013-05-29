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

#define QRCODE_SCALE	3

GdkPixbuf * encode_qrcode (char *text) {
       QRcode *qrcode;
       GdkPixbuf *pixbuf, *pixbuf_scaled;
       int i, j, k, rowstride, channels;
       gchar *pixel;
       unsigned char *data;
       
       qrcode = QRcode_encodeData(strlen(text), text, 0, QR_ECLEVEL_L);

       if (qrcode == NULL)
               return NULL;

       data = qrcode->data;

       pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, qrcode->width + 2, qrcode->width + 2);

       pixel = gdk_pixbuf_get_pixels (pixbuf);
       rowstride = gdk_pixbuf_get_rowstride (pixbuf);
       channels = gdk_pixbuf_get_n_channels (pixbuf);

       gdk_pixbuf_fill(pixbuf, 0xffffffff);
       for (i = 1; i <= qrcode->width; i++)
               for (j = 1; j <= qrcode->width; j++) {
                       for (k = 0; k < channels; k++)
                               pixel[i * rowstride + j * channels + k] = !(*data & 0x1) * 0xff;
                       data++;
               }

       pixbuf_scaled = gdk_pixbuf_scale_simple (pixbuf,
		       (qrcode->width + 2) * QRCODE_SCALE,
		       (qrcode->width + 2) * QRCODE_SCALE,
		       GDK_INTERP_NEAREST);

       QRcode_free(qrcode);
       g_object_unref(pixbuf);
       
       return pixbuf_scaled;
}

int main(int argc, char **argv) {
	char * http_referer, * server_name, * pattern;
	regex_t preg;
	size_t nmatch;
	regmatch_t pmatch[2];
	int rc;

	GdkPixbuf *pixbuf;

	gchar *buffer;
	gsize size;

	if ((http_referer = getenv("HTTP_REFERER")) == NULL ||
			(server_name = getenv("SERVER_NAME")) == NULL) {
		printf("This is a CGI executable. Running without a web service is not supported.\n"
				"Note that HTTP_REFERER and SERVER_NAME need to be defined.\n");
		return EXIT_FAILURE;
	} 
	
	pattern = malloc(11 + strlen(server_name));
	sprintf(pattern, "^http://%s/", server_name);
	if ((rc = regcomp(&preg, pattern, 0)) != 0)
		printf("regcomp() failed, returning nonzero (%d)\n", rc);

	if ((rc = regexec(&preg, http_referer, nmatch, pmatch, 0)) != 0) {
		/* stolen... */
		http_referer = malloc(44 + strlen(server_name));
		sprintf(http_referer, "This QR Code has been stolen from http://%s/!", server_name);
	}

	regfree(&preg);
	free(pattern);
	
	if (pixbuf = encode_qrcode(http_referer)) {
		/* print HTTP header */
		printf("Content-Type: image/png\n\n");

		/* print PNG data */
		gdk_pixbuf_save_to_buffer (pixbuf, &buffer, &size, "png", NULL, NULL);
		fwrite (buffer, 1, size, stdout);
	} else {
		printf("Error?!\n");
	}

	if (rc)
		free(http_referer);

	return EXIT_SUCCESS;
}

// vim: set syntax=c:
