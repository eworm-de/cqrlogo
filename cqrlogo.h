/*
 * (C) 2013-2022 by Christian Hesse <mail@eworm.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
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
