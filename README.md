cqrlogo
=======

[![GitHub stars](https://img.shields.io/github/stars/eworm-de/cqrlogo?logo=GitHub&style=flat&color=red)](https://github.com/eworm-de/cqrlogo/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/eworm-de/cqrlogo?logo=GitHub&style=flat&color=green)](https://github.com/eworm-de/cqrlogo/network)
[![GitHub watchers](https://img.shields.io/github/watchers/eworm-de/cqrlogo?logo=GitHub&style=flat&color=blue)](https://github.com/eworm-de/cqrlogo/watchers)

**CGI QR-Code logo**

The resulting QR-Code should look something like this:

![QR-Code](cqrlogo.png)

It is generated dynamically from referer URL sent by the user agent.

*Use at your own risk*, pay attention to
[license and warranty](#license-and-warranty), and
[disclaimer on external links](#disclaimer-on-external-links)!

Requirements
------------

To compile and run `cqrlogo` you need:

* [iniparser ↗️](https://github.com/ndevilla/iniparser)
* [libpng ↗️](https://www.libpng.org/pub/png/libpng.html)
* [zlib ↗️](https://www.zlib.net/) (which is a dependency for libpng)
* [qrencode ↗️](https://fukuchi.org/works/qrencode/)
* [fcgi ↗️](https://github.com/FastCGI-Archives/fcgi2) (for FastCGI support)
* [markdown ↗️](https://daringfireball.net/projects/markdown/) (HTML documentation)

Additionally it is expected to have `make` and `pkg-config` around to
successfully compile.

Some systems may require additional development packages for the libraries.
Look for `libpng-dev`, `libz-dev` and `libqrencode-dev` or similar.

For `make check` to work you have to install optional dependency
[zbar ↗️](https://zbar.sourceforge.net/).

Build and install
-----------------

Building and installing is very easy. Just run:

    make

followed by:

    make install

This will place a shared library at `/usr/lib/libcqrlogo.so`, an
executable at `/usr/lib/cqrlogo/cqrlogo` and documentation can be found in
`/usr/share/doc/cqrlogo/`.

The file `config.h` (copy from `config.def.h` if it does not exist) provides
some build time configuration, so feel free to make your changes there and
rerun the commands above.

### Static build

In case you do not like the shared library `libcqrlogo.so` for whatever
reason you can build a statically linked version:

    make static

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

    gcc -lcqrlogo -o cqrlogo.cgi cqrlogo.c

This will result in a CGI executable `cqrlogo.cgi`.

License and warranty
--------------------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[GNU General Public License](COPYING.md) for more details.

Disclaimer on external links
----------------------------

Our website contains links to the websites of third parties ("external
links"). As the content of these websites is not under our control, we
cannot assume any liability for such external content. In all cases, the
provider of information of the linked websites is liable for the content
and accuracy of the information provided. At the point in time when the
links were placed, no infringements of the law were recognisable to us.
As soon as an infringement of the law becomes known to us, we will
immediately remove the link in question.

> 💡️ **Hint**: All external links are marked with an arrow pointing
> diagonally in an up-right (or north-east) direction (↗️).

### Upstream

URL:
[GitHub.com](https://github.com/eworm-de/cqrlogo#cqrlogo)

Mirror:
[eworm.de](https://git.eworm.de/cgit.cgi/cqrlogo/)
[GitHub.com](https://github.com/eworm-de/cqrlogo#cqrlogo)

---
[⬆️ Go back to top](#top)
