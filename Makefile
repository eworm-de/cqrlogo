# cqrlogo - CGI QR-Code logo for web services

CC	:= gcc
INSTALL	:= install
CP	:= cp
RM	:= rm
ZBARIMG	:= zbarimg
SED	:= sed
GREP	:= grep
FILE	:= file
CFLAGS	+= -O2 -Wall -Werror
CFLAGS	+= $(shell pkg-config --cflags --libs libpng) \
	   $(shell pkg-config --cflags --libs zlib) \
	   $(shell pkg-config --cflags --libs libqrencode)
VERSION	= $(shell git describe --tags --long)

all: cqrlogo.c config.h
	$(CC) $(CFLAGS) -o cqrlogo cqrlogo.c \
		-DVERSION="\"$(VERSION)\""

config.h:
	$(CP) config.def.h config.h

install:
	$(INSTALL) -D -m0755 cqrlogo $(DESTDIR)/usr/share/webapps/cqrlogo/cqrlogo

check:
	$(eval SERVER := www.eworm.de)
	$(RM) -f check.png

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		    ./cqrlogo | $(SED) '1,/^$$/d' > check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=https://$(SERVER)/ HTTPS=on \
		    ./cqrlogo | $(SED) '1,/^$$/d' > check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^https://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=0' \
		./cqrlogo | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=4' \
		./cqrlogo | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=10' \
		./cqrlogo | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=0' \
		./cqrlogo | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=2' \
		./cqrlogo | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10' \
		./cqrlogo | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=0' \
		./cqrlogo | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=2' \
		./cqrlogo | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=$(SERVER) HTTP_REFERER=http://$(SERVER)/ \
		QUERY_STRING='scale=2&border=10&level=4' \
		./cqrlogo | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | $(GREP) -e '^http://$(SERVER)/$$'

	SERVER_NAME=eworm.net HTTP_REFERER=http://$(SERVER)/ \
		./cqrlogo | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | \
		$(GREP) -e '^This QR Code has been stolen from http://eworm.net/!$$'

	SERVER_NAME=eworm.net HTTP_REFERER=https://$(SERVER)/ HTTPS=on \
		./cqrlogo | $(SED) '1,/^$$/d' > \
		check.png
	$(FILE) check.png | $(GREP) 'PNG image data'
	$(ZBARIMG) --raw -q check.png | \
		$(GREP) -e '^This QR Code has been stolen from https://eworm.net/!$$'

clean:
	$(RM) -f *.o *~ cqrlogo
