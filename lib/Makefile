# cqrlogo - CGI QR-Code logo for web services

CC	:= gcc
LN	:= ln
CFLAGS	+= -O2 -Wall -Werror
CFLAGS	+= -liniparser
CFLAGS	+= $(shell pkg-config --cflags --libs libpng)
CFLAGS	+= $(shell pkg-config --cflags --libs zlib)
CFLAGS	+= $(shell pkg-config --cflags --libs libqrencode)

all: libcqrlogo.so

libcqrlogo.o: libcqrlogo.c libcqrlogo.h ../config.h ../version.h Makefile
	$(CC) -c $(CFLAGS) $(LDFLAGS) -fPIC -o libcqrlogo.o libcqrlogo.c

libcqrlogo.so: libcqrlogo.o
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC -Wl,-soname,libcqrlogo.so.$(SOVERSION) -o libcqrlogo.so.$(SOVERSION) libcqrlogo.o
	$(LN) -sf libcqrlogo.so.$(SOVERSION) libcqrlogo.so

clean:
	$(RM) -f libcqrlogo.o libcqrlogo.so libcqrlogo.so.*

distclean:
	$(RM) -f *~ libcqrlogo.o libcqrlogo.so libcqrlogo.so.*
