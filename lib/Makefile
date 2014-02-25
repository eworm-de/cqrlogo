# cqrlogo - CGI QR-Code logo for web services

CC	:= gcc
LN	:= ln
CFLAGS	+= -O2 -Wall -Werror
CFLAGS	+= -liniparser
CFLAGS	+= $(shell pkg-config --cflags --libs libpng)
CFLAGS	+= $(shell pkg-config --cflags --libs zlib)
CFLAGS	+= $(shell pkg-config --cflags --libs libqrencode)

all: libcqrlogo.so

libcqrlogo.so: libcqrlogo.c libcqrlogo.h ../config.h ../version.h
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -Wl,-soname,libcqrlogo.so.$(SOVERSION) -o libcqrlogo.so.$(SOVERSION) libcqrlogo.c
	$(LN) -sf libcqrlogo.so.$(SOVERSION) libcqrlogo.so

clean:
	$(RM) -f *.o *~ libcqrlogo.so libcqrlogo.so.*
