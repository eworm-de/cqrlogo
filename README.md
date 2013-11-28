cqrlogo
=======

**CGI QR-Code logo**

The resulting QR-Code should look something like this:

![QR-Code](cqrlogo.png)

It is generated dynamically from referer URL sent by the user agent.

Requirements
------------

To compile and run `cqrlogo` you need:

* [iniparser](http://ndevilla.free.fr/iniparser/)
* [libpng](http://www.libpng.org/pub/png/libpng.html)
* [zlib](http://www.zlib.net/) (which is a dependency for libpng)
* [qrencode](http://megaui.net/fukuchi/works/qrencode/index.en.html)
* [markdown](http://daringfireball.net/projects/markdown/) (HTML documentation)

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

This will place an executable at `/usr/share/webapps/cqrlogo/cqrlogo`,
documentation can be found in `/usr/share/doc/cqrlogo/`.

The file `config.h` (copy from `config.def.h` if it does not exist) provides
some build time configuration, so feel free to make your changes there and
rerun the commands above.

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

