/*=============================================================================
Copyright (C) 2016 Kristina Brooks
All rights reserved.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

FILE DESCRIPTION
Panic routine.

=============================================================================*/


#include "xprintf.h"

void panic(const char* fmt,  ...) {
	xputs("panic(): ");

	va_list arp;
	va_start(arp, fmt);
	xvprintf(fmt, arp);
	va_end(arp);

	xputc('\n');

	for (;;) {
		__asm__ __volatile__ ("nop" :::);
	}
}