/*
 * (C) 2013-2015 by Christian Hesse <mail@eworm.de>
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
	struct timeval tv;

	struct cqr_png * png;
	struct cqr_bitmap * bitmap;
	struct cqr_conf conf;

#if HAVE_FCGI
	/* loop for requests */
	while (FCGI_Accept() >= 0)
#endif
	{
		/* do the variable initialization within the loop! */
		uri_server_name = NULL;
		pattern = NULL;
		stolen = NULL;

		https = 0;

		/* these default values are defined in config.h */
		conf.scale = QRCODE_SCALE;
		conf.border = QRCODE_BORDER;
		conf.level = QRCODE_LEVEL;
		conf.overwrite = ALLOW_OVERWRITE;

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
				fprintf(stderr, "regcomp() failed, returning nonzero.\n");
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

		cqr_conf_file(server_name, &conf);
		cqr_conf_string(getenv("QUERY_STRING"), &conf);

		/* encode the QR-Code */
		if ((bitmap = cqr_encode_qrcode_to_bitmap(uri, conf)) == NULL) {
			/* uri too long? retry with uri from server name */
			uri = uri_server_name;
			if ((bitmap = cqr_encode_qrcode_to_bitmap(uri, conf)) == NULL) {
				fprintf(stderr, "Could not generate QR-Code.\n");
				return EXIT_FAILURE;
			}
		}

		/* generate PNG data */
		if ((png = cqr_bitmap_to_png(bitmap, uri, CQR_META_ALL)) == NULL) {
			fprintf(stderr, "Failed to generate PNG.\n");
			return EXIT_FAILURE;
		}

		/* print HTTP header */
		fputs(CQR_HEADER_CONTENT_TYPE, stdout);
		fputs(CQR_HEADER_CONTENT_DISPOSITION, stdout);
		fputs(CQR_HEADER_CACHE_CONTROL, stdout);
		fputs(CQR_HEADER_PRAGMA, stdout);
		gettimeofday(&tv, NULL);
		fprintf(stdout, "ETag: %lu%lu\n", tv.tv_sec, tv.tv_usec);
		fputc('\n', stdout);

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
		cqr_bitmap_free(bitmap);
		if (png->size > 0)
			free(png->buffer);
		free(png);
	}

	return EXIT_SUCCESS;
}

// vim: set syntax=c:
