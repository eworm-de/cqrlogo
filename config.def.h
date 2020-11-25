/*
 * (C) 2013-2020 by Christian Hesse <mail@eworm.de>
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

#ifndef _CONFIG_H
#define _CONFIG_H

/* path to the config file */
#define CONFIGFILE		"/etc/cqrlogo.conf"

/* whether or not defaults or settings from config may be overwritten
 * by query string */
#define ALLOW_OVERWRITE		1

/* pixels are scaled up by this factor */
#define QRCODE_SCALE		2
/* this is the maximum scale factor */
#define QRCODE_MAX_SCALE	16

/* width of the border
 * this is defined to at least 4, but works well with less */
# define QRCODE_BORDER		1
/* this is the maximum border width */
# define QRCODE_MAX_BORDER	8

/* error correction level used for QR code
 * possible values: QR_ECLEVEL_L (lowest, about 7% error can be corrected)
 *                  QR_ECLEVEL_M (about 15%)
 *                  QR_ECLEVEL_Q (about 25%)
 *                  QR_ECLEVEL_H (highest, about 30%)
 * image size raises with higher levels */
#define QRCODE_LEVEL		QR_ECLEVEL_L
/* note that changing the level at runtime requies
 * a numeric value from 0 to 3 */

#endif /* _CONFIG_H */

// vim: set syntax=c:
