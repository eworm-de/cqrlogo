cqrlogo
=======

**CGI QR-Code logo**

The resulting QR-Code should look something like this:

![QR-Code](cqrlogo.png)

It is generated dynamically from referer URL sent by the user agent.

Requirements
------------

To compile and run `cqrlogo` you need:

* [iniparser](https://github.com/ndevilla/iniparser)
* [libpng](http://www.libpng.org/pub/png/libpng.html)
* [zlib](https://www.zlib.net/) (which is a dependency for libpng)
* [qrencode](https://fukuchi.org/works/qrencode/)
* [fcgi](http://www.fastcgi.com/) (for FastCGI support)
* [markdown](https://daringfireball.net/projects/markdown/) (HTML documentation)

Additionally it is expected to have `make` and `pkg-config` around to
successfully compile.

Some systems may require additional development packages for the libraries.
Look for `libpng-dev`, `libz-dev` and `libqrencode-dev` or similar.

For `make check` to work you have to install optional dependency
[zbar](http://zbar.sourceforge.net/).

Build and install
-----------------

Building and installing is very easy. Just run:

> make

followed by:

> make install

This will place a shared library at `/usr/lib/libcqrlogo.so`, an
executable at `/usr/lib/cqrlogo/cqrlogo` and documentation can be found in
`/usr/share/doc/cqrlogo/`.

The file `config.h` (copy from `config.def.h` if it does not exist) provides
some build time configuration, so feel free to make your changes there and
rerun the commands above.

### Static build

In case you do not like the shared library `libcqrlogo.so` for whatever
reason you can build a statically linked version:

> make static

However there is no install routine for the static build. Please note
that this is still dynamically linked to `libpng`, `libz`,
`libiniparser` and `libqrencode`.

Usage
-----

This is a **CGI executable** (*Common Gateway interface*), so it is expected
to be run inside a web server. Consult your web server's documentation
to get information about how to run CGI executables.

By default `cqrlogo` generates a file with scale two, so one QR-Code pixel
results in 2x2 pixels. The border is one pixel (though scale takes effect)
and error correction level is the lowest available.

Runtime options can be given with request method GET. These are available:

* `scale`: scale the QR-Code up by this factor
* `border`: width of the border
* `level`: error correction level

Library
-------

This now uses a shared library the can be used to create CGI executables for
your custom needs. This is minimal sample source code:

    #include <libcqrlogo.h>

    int main(int argc, char **argv) {
            struct cqr_png * png;
            struct cqr_conf conf;

            conf.scale = 2;
            conf.border = 1;
            conf.level = 0;
            conf.overwrite = 1;

            /* generate PNG data */
            if ((png = cqr_encode_qrcode_to_png("https://github.com/eworm-de/cqrlogo", conf, CQR_META_ALL)) == NULL) {
                    fprintf(stderr, "Failed to generate PNG.\n");
                    return EXIT_FAILURE;
            }

            /* print HTTP header */
            fputs(CQR_HEADER_CONTENT_TYPE, stdout);
            fputc('\n', stdout);

            /* write PNG data to stdout */
            if (fwrite(png->buffer, png->size, 1, stdout) != 1) {
                    fprintf(stderr, "Failed writing PNG data to stdout.\n");
                    return EXIT_FAILURE;
            }

            /* free memory we no longer need */
            if (png->size > 0)
                    free(png->buffer);
            free(png);

            return EXIT_SUCCESS;
    }

Save this code to `cqrlogo.c` and run:

> gcc -lcqrlogo -o cqrlogo.cgi cqrlogo.c

This will result in a CGI executable `cqrlogo.cgi`.

### Upstream

URL:
[GitHub.com](https://github.com/eworm-de/cqrlogo#cqrlogo)

Mirror:
[eworm.de](https://git.eworm.de/cgit.cgi/cqrlogo/)
[GitHub.com](https://github.com/eworm-de/cqrlogo#cqrlogo)
