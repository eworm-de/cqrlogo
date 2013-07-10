# cqrlogo - CGI QR-Code logo for web services

CC	:= gcc
INSTALL	:= install
CP	:= cp
RM	:= rm
ZBARIMG	:= zbarimg
SED	:= sed
GREP	:= grep
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
	$(RM) -f check.png
	SERVER_NAME=www.eworm.de HTTP_REFERER=http://www.eworm.de/ \
		QUERY_STRING='scale=2&border=1' ./cqrlogo | $(SED) '1,2d' > \
		check.png && $(ZBARIMG) --raw -q check.png | $(GREP) -e \
		'^http://www.eworm.de/$$'

clean:
	$(RM) -f *.o *~ cqrlogo
