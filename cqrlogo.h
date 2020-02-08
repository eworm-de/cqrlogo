/*
 * (C) 2013-2020 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#ifndef _CQRLOGO_H
#define _CQRLOGO_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <regex.h>
#include <stdbool.h>
#include <sys/time.h>

#if HAVE_FCGI
#include <fcgi_stdio.h>
#endif

#include <libcqrlogo.h>

#define URLPATTERN "^[hH][tT][tT][pP][sS]\\?://%s/"
#define TEXTSTOLEN "This QR Code has been stolen from http%s://%s/!"

/*** main ***/
int main(int argc, char **argv);

#endif /* _CQRLOGO_H */

// vim: set syntax=c:
