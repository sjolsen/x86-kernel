#include "format.h"
#include <stddef.h>

static inline
char digit_to_ascii (uint_fast8_t digit)
{
	if (digit < 10)
		return digit + '0';
	else
		return (digit - 10) + 'A';
}

static inline
void creverse (char* begin, char* end)
{
	if (begin == end)
		return;
	--end;

	while (begin < end) {
		char tmp = *begin;
		*begin = *end;
		*end = tmp;
		++begin;
		--end;
	}
}

static inline
size_t fmt_strlen (const char* str)
{
	size_t len = 0;
	while (str [len] != '\0')
		++len;
	return len;
}


// Extern functions

char* numsep (char* bufstr, char sep)
{
	size_t n = fmt_strlen (bufstr);
	size_t m = n + (n - 1)/3;
	char* src = bufstr + n - 1;
	char* dst = bufstr + m - 1;
	for (int i = 0; src != dst;) {
		if (i == 3) {
			i = 0;
			*dst = sep;
		}
		else {
			i = i + 1;
			*dst = *src;
			--src;
		}
		--dst;
	}
	bufstr [m] = '\0';
	return bufstr;
}

char* format_int (char* buffer, int64_t value, uint_fast8_t mincol, uint_fast8_t base)
{
	if (value < 0) {
		char* numstring = format_uint (buffer + 1, -value, mincol, base);
		--numstring;
		numstring [0] = '-';
		return numstring;
	}
	else
		return format_uint (buffer, value, mincol, base);
}

char* format_uint (char* buffer, uint64_t value, uint_fast8_t mincol, uint_fast8_t base)
{
	char* cursor = buffer;

	do {
		uint64_t div = value / base;
		uint64_t mod = value % base;
		*cursor = digit_to_ascii (mod);
		value = div;
		++cursor;
	}
	while (value != 0);

	while (cursor < buffer + mincol)
		*cursor++ = '0';
	creverse (buffer, cursor);
	*cursor = '\0';

	return buffer;
}
