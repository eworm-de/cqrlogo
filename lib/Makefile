# cqrlogo - CGI QR-Code logo for web services

# commands
CC	:= gcc
LN	:= ln

# flags
CFLAGS	+= -std=c11 -O2 -fPIC -Wall -Werror
CFLAGS	+= -liniparser
CFLAGS	+= $(shell pkg-config --cflags --libs libpng)
CFLAGS	+= $(shell pkg-config --cflags --libs zlib)
CFLAGS	+= $(shell pkg-config --cflags --libs libqrencode)
LDFLAGS	+= -Wl,-z,now -Wl,-z,relro -pie

all: libcqrlogo.so

libcqrlogo.o: libcqrlogo.c libcqrlogo.h ../config.h ../version.h Makefile
	$(CC) libcqrlogo.c $(CFLAGS) -c -o libcqrlogo.o

libcqrlogo.so: libcqrlogo.c libcqrlogo.h ../config.h ../version.h Makefile
	$(CC) libcqrlogo.c $(CFLAGS) $(LDFLAGS) -shared -Wl,-soname,libcqrlogo.so.$(SOVERSION) -o libcqrlogo.so.$(SOVERSION)
	$(LN) -sf libcqrlogo.so.$(SOVERSION) libcqrlogo.so

clean:
	$(RM) -f libcqrlogo.o libcqrlogo.so libcqrlogo.so.*

distclean:
	$(RM) -f *~ libcqrlogo.o libcqrlogo.so libcqrlogo.so.*
