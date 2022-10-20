/* Copyright 2019, Tencent Technology (Shenzhen) Co Ltd
 
 This file is part of the Tencent SM (Lite Version) Library.
 
 The Tencent SM (Lite Version) Library is free software; you can redistribute it and/or modify
 it under the terms of either:
 
 * the GNU Lesser General Public License as published by the Free
 Software Foundation; either version 3 of the License, or (at your
 option) any later version.
 
 or
 
 * the GNU General Public License as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any
 later version.
 
 or both in parallel, as here.
 
 The Tencent SM (Lite Version) Library is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 for more details.
 
 You should have received copies of the GNU General Public License and the
 GNU Lesser General Public License along with the Tencent SM (Lite Version) Library.  If not,
 see https://www.gnu.org/licenses/.  */

#ifndef LIBSM_SMS4_LCL_H
#define LIBSM_SMS4_LCL_H

#ifdef __cplusplus
extern "C" {
#endif

#define GET32(pc)  (					\
	((uint32_t)(pc)[0] << 24) ^			\
	((uint32_t)(pc)[1] << 16) ^			\
	((uint32_t)(pc)[2] <<  8) ^			\
	((uint32_t)(pc)[3]))

#define PUT32(st, ct)					\
	(ct)[0] = (uint8_t)((st) >> 24);		\
	(ct)[1] = (uint8_t)((st) >> 16);		\
	(ct)[2] = (uint8_t)((st) >>  8);		\
	(ct)[3] = (uint8_t)(st)

#define ROUNDS(x0, x1, x2, x3, x4)		\
	ROUND(x0, x1, x2, x3, x4, 0);		\
	ROUND(x1, x2, x3, x4, x0, 1);		\
	ROUND(x2, x3, x4, x0, x1, 2);		\
	ROUND(x3, x4, x0, x1, x2, 3);		\
	ROUND(x4, x0, x1, x2, x3, 4);		\
	ROUND(x0, x1, x2, x3, x4, 5);		\
	ROUND(x1, x2, x3, x4, x0, 6);		\
	ROUND(x2, x3, x4, x0, x1, 7);		\
	ROUND(x3, x4, x0, x1, x2, 8);		\
	ROUND(x4, x0, x1, x2, x3, 9);		\
	ROUND(x0, x1, x2, x3, x4, 10);		\
	ROUND(x1, x2, x3, x4, x0, 11);		\
	ROUND(x2, x3, x4, x0, x1, 12);		\
	ROUND(x3, x4, x0, x1, x2, 13);		\
	ROUND(x4, x0, x1, x2, x3, 14);		\
	ROUND(x0, x1, x2, x3, x4, 15);		\
	ROUND(x1, x2, x3, x4, x0, 16);		\
	ROUND(x2, x3, x4, x0, x1, 17);		\
	ROUND(x3, x4, x0, x1, x2, 18);		\
	ROUND(x4, x0, x1, x2, x3, 19);		\
	ROUND(x0, x1, x2, x3, x4, 20);		\
	ROUND(x1, x2, x3, x4, x0, 21);		\
	ROUND(x2, x3, x4, x0, x1, 22);		\
	ROUND(x3, x4, x0, x1, x2, 23);		\
	ROUND(x4, x0, x1, x2, x3, 24);		\
	ROUND(x0, x1, x2, x3, x4, 25);		\
	ROUND(x1, x2, x3, x4, x0, 26);		\
	ROUND(x2, x3, x4, x0, x1, 27);		\
	ROUND(x3, x4, x0, x1, x2, 28);		\
	ROUND(x4, x0, x1, x2, x3, 29);		\
	ROUND(x0, x1, x2, x3, x4, 30);		\
	ROUND(x1, x2, x3, x4, x0, 31)

#ifdef __cplusplus
}
#endif
#endif

