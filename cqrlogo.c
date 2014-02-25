/*
 * (C) 2013-2014 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include "config.h"
#include "version.h"

#include "cqrlogo.h"

/*** main ***/
int main(int argc, char **argv) {
	const char * http_referer, * server_name, * uri;
	char * uri_server_name, * pattern, * stolen;
	regex_t preg;
	regmatch_t pmatch[1];
	uint8_t https;

	struct png_t * png;
	struct bitmap_t * bitmap;
	struct cqrconf_t cqrconf;

#if HAVE_FCGI
	/* loop for requests */
	while (FCGI_Accept() >= 0) {
#endif
	/* do the variable initialization within the loop! */
	uri_server_name = NULL;
	pattern = NULL;
	stolen = NULL;

	https = 0;

	/* these default values are defined in config.h */
	cqrconf.scale = QRCODE_SCALE;
	cqrconf.border = QRCODE_BORDER;
	cqrconf.level = QRCODE_LEVEL;
	cqrconf.overwrite = ALLOW_OVERWRITE;

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

	cqrconf_file(server_name, &cqrconf);
	cqrconf_string(getenv("QUERY_STRING"), &cqrconf);

	/* encode the QR-Code */
	if ((bitmap = encode_qrcode(uri, cqrconf)) == NULL) {
		/* uri too long? retry with uri from server name */
		uri = uri_server_name;
		if ((bitmap = encode_qrcode(uri, cqrconf)) == NULL) {
			fprintf(stderr, "Could not generate QR-Code.\n");
			return EXIT_FAILURE;
		}
	}

	/* print HTTP header */
	printf("Content-Type: image/png\n\n");

	/* generate PNG data */
	if ((png = generate_png(bitmap, CQR_COMMENT|CQR_REFERER|CQR_VERSION|CQR_LIBVERSION, uri)) == NULL) {
		fprintf(stderr, "Failed to generate PNG.\n");
		return EXIT_FAILURE;
	}

	/* write PNG data to stdout */
	if (fwrite(png->buffer, png->size, 1, stdout) != 1) {
		fprintf(stderr, "Failed writing PNG data to stdout.\n");
		return EXIT_FAILURE;
	}

	/* free memory we no longer need */
	if (uri_server_name)
		free(uri_server_name);
	if (stolen)
		free(stolen);
	bitmap_free(bitmap);
	if (png->size > 0)
		free(png->buffer);
	free(png);

#if HAVE_FCGI
	/* end of loop */
	}
#endif

	return EXIT_SUCCESS;
}

// vim: set syntax=c:
