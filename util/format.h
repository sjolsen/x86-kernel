#ifndef FORMAT_H
#define FORMAT_H

#include <stdint.h>

char* numsep (char* bufstr, char sep);
char* format_int (char* buffer, int64_t value, uint_fast8_t mincol, uint_fast8_t base);
char* format_uint (char* buffer, uint64_t value, uint_fast8_t mincol, uint_fast8_t base);

#endif
