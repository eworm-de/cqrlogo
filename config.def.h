/*
 * (C) 2011-2013 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#ifndef _CONFIG_H
#define _CONFIG_H

/* pixels are scaled up by this factor */
#define QRCODE_SCALE		2
/* this is the maximum scale factor */
#define QRCODE_MAX_SCALE	8

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
#define QRCODE_LEVEL	QR_ECLEVEL_L
/* note that changing the level at runtime requies
 * a numeric value from 0 to 3 */

/* if you really, really, really want to save some bytes...
 * It is possible to disable text information in PNG file completly, though
 * nobody will have an idea where you got this great software...
 * So please do not. */
#define PNG_ENABLE_TEXT	1
/* do you want version information within the PNG file? */
#define PNG_ENABLE_TEXT_VERSIONS 1
/* add referer information to the PNG file? */
#define PNG_ENABLE_TEXT_REFERER 1

#endif /* _CONFIG_H */

// vim: set syntax=c:
