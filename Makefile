# cqrlogo - CGI QR-Code logo for web services

# pathes
PREFIX		:= /usr
APACHECONF	:= /etc/httpd/conf/extra/
LIGHTTPDCONF	:= /etc/lighttpd/conf.d/
# commands
CC	:= gcc
CP	:= cp
FILE	:= file
GREP	:= grep
INSTALL	:= install
LN	:= ln
MD	:= markdown
RM	:= rm
SED	:= sed
PNGCHK	:= pngcheck
ZBARIMG	:= zbarimg
# flags
CFLAGS	+= -std=c11 -O2 -Wall -Werror
CFLAGSSTATIC	= $(CFLAGS)
CFLAGSSTATIC	+= -liniparser
CFLAGSSTATIC	+= $(shell pkg-config --cflags --libs libpng)
CFLAGSSTATIC	+= $(shell pkg-config --cflags --libs zlib)
CFLAGSSTATIC	+= $(shell pkg-config --cflags --libs libqrencode)
# calls to compiled binary files
CQRLOGO_CGI	+= LD_LIBRARY_PATH=lib/ ./cqrlogo.cgi
CQRLOGO_FCGI	+= LD_LIBRARY_PATH=lib/ ./cqrlogo.fcgi
# this is just a fallback in case you do not use git but downloaded
# a release tarball...
VERSION := 0.5.2
# library abi version
SOVERSION	:= 0

all: lib/libcqrlogo.so cqrlogo.cgi cqrlogo.fcgi README.html cqrlogo.png

static: cqrlogo.cgi.static cqrlogo.fcgi.static

lib/libcqrlogo.so: lib/Makefile lib/libcqrlogo.c lib/libcqrlogo.h config.h version.h
	SOVERSION=$(SOVERSION) $(MAKE) -C lib libcqrlogo.so

lib/libcqrlogo.o: lib/Makefile lib/libcqrlogo.c lib/libcqrlogo.h config.h version.h
	$(MAKE) -C lib libcqrlogo.o

cqrlogo.cgi: lib/libcqrlogo.so cqrlogo.c cqrlogo.h config.h version.h
	$(CC) $(CFLAGS) -lcqrlogo -Llib/ -Ilib/ $(LDFLAGS) -DHAVE_FCGI=0 -o cqrlogo.cgi cqrlogo.c

cqrlogo.cgi.static: lib/libcqrlogo.o cqrlogo.c cqrlogo.h config.h version.h
	$(CC) $(CFLAGSSTATIC) -Ilib/ $(LDFLAGS) -DHAVE_FCGI=0 -o cqrlogo.cgi.static cqrlogo.c lib/libcqrlogo.o

cqrlogo.fcgi: lib/libcqrlogo.so cqrlogo.c cqrlogo.h config.h version.h
	$(CC) $(CFLAGS) -lcqrlogo -Llib/ -Ilib/ -lfcgi $(LDFLAGS) -DHAVE_FCGI=1 -o cqrlogo.fcgi cqrlogo.c

cqrlogo.fcgi.static: lib/libcqrlogo.o cqrlogo.c cqrlogo.h config.h version.h
	$(CC) $(CFLAGSSTATIC) -Ilib/ -lfcgi $(LDFLAGS) -DHAVE_FCGI=1 -o cqrlogo.fcgi.static cqrlogo.c lib/libcqrlogo.o

version.h: $(wildcard .git/HEAD .git/index .git/refs/tags/*) Makefile
	echo "#ifndef VERSION" > $@
	echo "#define VERSION \"$(shell git describe --tags --long 2>/dev/null || echo ${VERSION})\"" >> $@
	echo "#endif" >> $@

config.h: config.def.h
	$(CP) config.def.h config.h

README.html: README.md
	$(MD) README.md > README.html

cqrlogo.png: cqrlogo.cgi
	SERVER_NAME="github.com" HTTP_REFERER="https://github.com/eworm-de/cqrlogo" \
		QUERY_STRING='scale=4' \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > cqrlogo.png

install: install-bin install-config install-doc

install-bin: lib/libcqrlogo.so lib/libcqrlogo.h cqrlogo.cgi cqrlogo.fcgi cqrlogo.conf
	$(INSTALL) -D -m0755 lib/libcqrlogo.so.$(SOVERSION) $(DESTDIR)$(PREFIX)/lib/libcqrlogo.so.$(SOVERSION)
	$(INSTALL) -D -m0755 lib/libcqrlogo.h $(DESTDIR)$(PREFIX)/include/libcqrlogo.h
	$(LN) -sf libcqrlogo.so.$(SOVERSION) $(DESTDIR)$(PREFIX)/lib/libcqrlogo.so
	$(INSTALL) -D -m0755 cqrlogo.cgi $(DESTDIR)$(PREFIX)/lib/cqrlogo/cqrlogo.cgi
	$(INSTALL) -D -m0755 cqrlogo.fcgi $(DESTDIR)$(PREFIX)/lib/cqrlogo/cqrlogo.fcgi
	$(INSTALL) -D -m0644 cqrlogo.conf $(DESTDIR)/etc/cqrlogo.conf

install-config: config/apache.conf config/lighttpd.conf
	$(INSTALL) -D -m0644 config/apache.conf $(DESTDIR)$(APACHECONF)/cqrlogo.conf
	$(INSTALL) -D -m0644 config/lighttpd.conf $(DESTDIR)$(LIGHTTPDCONF)/cqrlogo.conf

install-doc: README.md README.html cqrlogo.png
	$(INSTALL) -D -m0644 README.md $(DESTDIR)$(PREFIX)/share/doc/cqrlogo/README.md
	$(INSTALL) -D -m0644 README.html $(DESTDIR)$(PREFIX)/share/doc/cqrlogo/README.html
	$(INSTALL) -D -m0644 cqrlogo.png $(DESTDIR)$(PREFIX)/share/doc/cqrlogo/cqrlogo.png

check:
	$(eval SERVER := www.eworm.de)
	$(RM) -f check.png

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=https://$(SERVER)/ HTTPS=on \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^https://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=0' \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=4' \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=20' \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=0' \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=2' \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10' \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=0' \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=2' \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=4' \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=eworm.net HTTP_REFERER=http://$(SERVER)/ \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | \
		$(GREP) -e '^This QR Code has been stolen from http://eworm.net/!$$'

	SERVER_NAME=eworm.net HTTP_REFERER=https://$(SERVER)/ HTTPS=on \
		$(CQRLOGO_CGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | \
		$(GREP) -e '^This QR Code has been stolen from https://eworm.net/!$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=https://$(SERVER)/ HTTPS=on \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^https://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=0' \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=4' \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=20' \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=0' \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=2' \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10' \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=0' \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=2' \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=4' \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=eworm.net HTTP_REFERER=http://$(SERVER)/ \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | \
		$(GREP) -e '^This QR Code has been stolen from http://eworm.net/!$$'

	SERVER_NAME=eworm.net HTTP_REFERER=https://$(SERVER)/ HTTPS=on \
		$(CQRLOGO_FCGI) | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(PNGCHK) check.png
	$(ZBARIMG) --raw -q check.png | \
		$(GREP) -e '^This QR Code has been stolen from https://eworm.net/!$$'

clean:
	$(RM) -f *.png README.html cqrlogo.cgi cqrlogo.fcgi cqrlogo.cgi.static cqrlogo.fcgi.static version.h
	$(MAKE) -C lib clean

distclean:
	$(RM) -f *~ *.png README.html cqrlogo.cgi cqrlogo.fcgi cqrlogo.cgi.static cqrlogo.fcgi.static version.h config.h
	$(MAKE) -C lib distclean

release:
	git archive --format=tar.xz --prefix=cqrlogo-$(VERSION)/ $(VERSION) > cqrlogo-$(VERSION).tar.xz
	gpg -ab cqrlogo-$(VERSION).tar.xz
