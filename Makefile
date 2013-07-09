# cqrlogo - CGI QR-Code logo for web services

CC	:= gcc
INSTALL	:= install
CP	:= cp
RM	:= rm
CFLAGS	+= -O2 -Wall -Werror
CFLAGS	+= $(shell pkg-config --cflags --libs gdk-pixbuf-2.0) \
	   $(shell pkg-config --cflags --libs libqrencode)

all: cqrlogo.c config.h
	$(CC) $(CFLAGS) -o cqrlogo cqrlogo.c

config.h:
	$(CP) config.def.h config.h
install:
	$(INSTALL) -D -m0755 cqrlogo $(DESTDIR)/usr/share/webapps/cqrlogo/cqrlogo

clean:
	$(RM) -f *.o *~ cqrlogo
