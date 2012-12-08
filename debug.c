/*
 * This file is part of ltrace.
 * Copyright (C) 2003,2008,2009 Juan Cespedes
 * Copyright (C) 2006 Ian Wienand
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */

#include <stdio.h>
#include <stdarg.h>

#include "common.h"
#include "backend.h"

void
debug_(int level, const char *file, int line, const char *fmt, ...) {
	char buf[1024];
	va_list args;

	if (!(options.debug & level)) {
		return;
	}
	va_start(args, fmt);
	vsnprintf(buf, 1024, fmt, args);
	va_end(args);

	output_line(NULL, "DEBUG: %s:%d: %s", file, line, buf);
	fflush(options.output);
}

/*
 * The following section provides a way to print things, like hex dumps,
 * with out using buffered output.  This was written by Steve Munroe of IBM.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ptrace.h>

static int
xwritehexl(long i) {
	int rc = 0;
	char text[17];
	int j;
	unsigned long temp = (unsigned long)i;

	for (j = 15; j >= 0; j--) {
		char c;
		c = (char)((temp & 0x0f) + '0');
		if (c > '9') {
			c = (char)(c + ('a' - '9' - 1));
		}
		text[j] = c;
		temp = temp >> 4;
	}

	rc = write(1, text, 16);
	return rc;
}

static int
xwritec(char c) {
	char temp = c;
	char *text = &temp;
	int rc = 0;
	rc = write(1, text, 1);
	return rc;
}

static int
xwritecr(void) {
	return xwritec('\n');
}

static int
xwritedump(void *ptr, long addr, int len) {
	int rc = 0;
	long *tprt = (long *)ptr;
	int i;

	for (i = 0; i < len; i += 8) {
		xwritehexl(addr);
		xwritec('-');
		xwritec('>');
		xwritehexl(*tprt++);
		xwritecr();
		addr += sizeof(long);
	}

	return rc;
}

int
xinfdump(long pid, void *ptr, int len)
{
	unsigned char buf[len];
	size_t got = umovebytes(pid2proc(pid), ptr, buf, len);
	if (got == (size_t)-1)
		return -1;
	return xwritedump(buf, (long)ptr, got);
}
