# cqrlogo - CGI QR-Code logo for web services

PREFIX	:= /usr
LIBDIR	:= $(PREFIX)/lib
APACHECONF	:= /etc/apache/conf/extra/
LIGHTTPDCONF	:= /etc/lighttpd/conf.d/
CC	:= gcc
MD	:= markdown
INSTALL	:= install
CP	:= cp
RM	:= rm
ZBARIMG	:= zbarimg
SED	:= sed
GREP	:= grep
FILE	:= file
CFLAGS	+= -O2 -Wall -Werror
CFLAGS	+= -liniparser
CFLAGS	+= $(shell pkg-config --cflags --libs libpng)
CFLAGS	+= $(shell pkg-config --cflags --libs zlib)
CFLAGS	+= $(shell pkg-config --cflags --libs libqrencode)
# this is just a fallback in case you do not use git but downloaded
# a release tarball...
VERSION := 0.4.0

all: cqrlogo.cgi cqrlogo.fcgi README.html cqrlogo.png

cqrlogo.cgi: cqrlogo.c config.h version.h
	$(CC) $(CFLAGS) $(LDFLAGS) -DHAVE_FCGI=0 -o cqrlogo.cgi cqrlogo.c

cqrlogo.fcgi: cqrlogo.c config.h version.h
	$(CC) $(CFLAGS) -lfcgi $(LDFLAGS) -DHAVE_FCGI=1 -o cqrlogo.fcgi cqrlogo.c

version.h: $(wildcard .git/HEAD .git/index .git/refs/tags/*) Makefile
	echo "#ifndef VERSION" > $@
	echo "#define VERSION \"$(shell git describe --tags --long 2>/dev/null || echo ${VERSION})\"" >> $@
	echo "#endif" >> $@

config.h:
	$(CP) config.def.h config.h

README.html: README.md
	$(MD) README.md > README.html

cqrlogo.png: cqrlogo.cgi
	SERVER_NAME="github.com" HTTP_REFERER="https://github.com/eworm-de/cqrlogo" \
		    QUERY_STRING='scale=4' \
		    ./cqrlogo.cgi | $(SED) '1,/^$$/d' > cqrlogo.png

install: install-bin install-config install-doc

install-bin: cqrlogo.cgi cqrlogo.fcgi
	$(INSTALL) -D -m0755 cqrlogo.cgi $(DESTDIR)$(LIBDIR)/cqrlogo/cqrlogo.cgi
	$(INSTALL) -D -m0755 cqrlogo.fcgi $(DESTDIR)$(LIBDIR)/cqrlogo/cqrlogo.fcgi
	$(INSTALL) -D -m0644 cqrlogo.conf $(DESTDIR)/etc/cqrlogo.conf

install-config: config/apache.conf config/lighttpd.conf
	$(INSTALL) -D -m0644 config/apache.conf $(DESTDIR)$(APACHECONF)/cqrlogo.conf
	$(INSTALL) -D -m0644 config/lighttpd.conf $(DESTDIR)$(LIGHTTPDCONF)/cqrlogo.conf

install-doc: README.html cqrlogo.png
	$(INSTALL) -D -m0644 README.md $(DESTDIR)$(PREFIX)/share/doc/cqrlogo/README.md
	$(INSTALL) -D -m0644 README.html $(DESTDIR)$(PREFIX)/share/doc/cqrlogo/README.html
	$(INSTALL) -D -m0644 cqrlogo.png $(DESTDIR)$(PREFIX)/share/doc/cqrlogo/cqrlogo.png

check:
	$(eval SERVER := www.eworm.de)
	$(RM) -f check.png

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		    ./cqrlogo.cgi | $(SED) '1,/^$$/d' > check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=https://$(SERVER)/ HTTPS=on \
		    ./cqrlogo.cgi | $(SED) '1,/^$$/d' > check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^https://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=0' \
		./cqrlogo.cgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=4' \
		./cqrlogo.cgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=20' \
		./cqrlogo.cgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=0' \
		./cqrlogo.cgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=2' \
		./cqrlogo.cgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10' \
		./cqrlogo.cgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=0' \
		./cqrlogo.cgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=2' \
		./cqrlogo.cgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=4' \
		./cqrlogo.cgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=eworm.net HTTP_REFERER=http://$(SERVER)/ \
		./cqrlogo.cgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | \
		$(GREP) -e '^This QR Code has been stolen from http://eworm.net/!$$'

	SERVER_NAME=eworm.net HTTP_REFERER=https://$(SERVER)/ HTTPS=on \
		./cqrlogo.cgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | \
		$(GREP) -e '^This QR Code has been stolen from https://eworm.net/!$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		    ./cqrlogo.fcgi | $(SED) '1,/^$$/d' > check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=https://$(SERVER)/ HTTPS=on \
		    ./cqrlogo.fcgi | $(SED) '1,/^$$/d' > check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^https://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=0' \
		./cqrlogo.fcgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=4' \
		./cqrlogo.fcgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=20' \
		./cqrlogo.fcgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=0' \
		./cqrlogo.fcgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=2' \
		./cqrlogo.fcgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10' \
		./cqrlogo.fcgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=0' \
		./cqrlogo.fcgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=2' \
		./cqrlogo.fcgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=4' \
		./cqrlogo.fcgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=eworm.net HTTP_REFERER=http://$(SERVER)/ \
		./cqrlogo.fcgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | \
		$(GREP) -e '^This QR Code has been stolen from http://eworm.net/!$$'

	SERVER_NAME=eworm.net HTTP_REFERER=https://$(SERVER)/ HTTPS=on \
		./cqrlogo.fcgi | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | \
		$(GREP) -e '^This QR Code has been stolen from https://eworm.net/!$$'

clean:
	$(RM) -f *.o *~ *.png README.html cqrlogo.cgi cqrlogo.fcgi version.h

distclean:
	$(RM) -f *.o *~ *.png README.html cqrlogo.cgi cqrlogo.fcgi version.h config.h

release:
	git archive --format=tar.xz --prefix=cqrlogo-$(VERSION)/ $(VERSION) > cqrlogo-$(VERSION).tar.xz
	gpg -ab cqrlogo-$(VERSION).tar.xz
