#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>

extern const uint32_t ktext32_base;
extern const uint32_t ktext32_size;

extern const uint32_t krodata_base;
extern const uint32_t krodata_size;

extern const uint32_t kdata_base;
extern const uint32_t kdata_size;

extern const uint32_t kbss_base;
extern const uint32_t kbss_size;

extern const uint32_t ktext_base;
extern const uint32_t ktext_size;

extern const uint32_t kernel_base;
extern const uint32_t kernel_size;

#endif
