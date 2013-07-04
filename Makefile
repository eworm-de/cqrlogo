# cqrlogo - CGI QR-Code logo for web services

CC	:= gcc
CFLAGS	+= -O2 -Wall -Werror
CFLAGS	+= $(shell pkg-config --cflags --libs gdk-pixbuf-2.0) \
	   $(shell pkg-config --cflags --libs libqrencode)

all: cqrlogo.c
	$(CC) $(CFLAGS) -o cqrlogo cqrlogo.c

clean:
	/bin/rm -f *.o *~ cqrlogo
