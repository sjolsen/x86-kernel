#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

/// Note: all addresses are physical and must be zero-extended past
/// the machine width.

typedef struct {
	uint64_t present         : 1;
	uint64_t writable        : 1;
	uint64_t user            : 1;
	uint64_t write_through   : 1;
	uint64_t cache_disable   : 1;
	uint64_t accessed        : 1;
	uint64_t                 : 1;
	uint64_t reserved        : 1; // Must be 0
	uint64_t                 : 4;
	uint64_t PDPT_address    : 40;
	uint64_t                 : 11;
	uint64_t execute_disable : 1;
} PML4E;

typedef struct {
	uint64_t present         : 1;
	uint64_t writable        : 1;
	uint64_t user            : 1;
	uint64_t write_through   : 1;
	uint64_t cache_disable   : 1;
	uint64_t accessed        : 1;
	uint64_t dirty           : 1;
	uint64_t page_size       : 1; // Must be 1
	uint64_t global          : 1;
	uint64_t                 : 3;
	uint64_t PAT             : 1;
	uint64_t reserved        : 17; // Must be 0
	uint64_t page_address    : 22; // Maps a 1 GiB page
	uint64_t                 : 11;
	uint64_t execute_disable : 1;
} PDPTE_direct;

typedef struct {
	uint64_t present         : 1;
	uint64_t writable        : 1;
	uint64_t user            : 1;
	uint64_t write_through   : 1;
	uint64_t cache_disable   : 1;
	uint64_t accessed        : 1;
	uint64_t                 : 1;
	uint64_t page_size       : 1; // Must be 0
	uint64_t                 : 4;
	uint64_t PD_address      : 40;
	uint64_t                 : 11;
	uint64_t execute_disable : 1;
} PDPTE_indirect;

typedef union {
	struct {
		uint64_t           : 7;
		uint64_t page_size : 1;
		uint64_t           : 58;
	};
	PDPTE_direct   direct;
	PDPTE_indirect indirect;
} PDPTE;

typedef struct {
	uint64_t present         : 1;
	uint64_t writable        : 1;
	uint64_t user            : 1;
	uint64_t write_through   : 1;
	uint64_t cache_disable   : 1;
	uint64_t accessed        : 1;
	uint64_t dirty           : 1;
	uint64_t page_size       : 1; // Must be 1
	uint64_t global          : 1;
	uint64_t                 : 3;
	uint64_t PAT             : 1;
	uint64_t reserved        : 8;
	uint64_t page_address    : 31; // Maps a 2 MiB page
	uint64_t                 : 11;
	uint64_t execute_disable : 1;
} PDE_direct;

typedef struct {
	uint64_t present         : 1;
	uint64_t writable        : 1;
	uint64_t user            : 1;
	uint64_t write_through   : 1;
	uint64_t cache_disable   : 1;
	uint64_t accessed        : 1;
	uint64_t                 : 1;
	uint64_t page_size       : 1; // Must be 0
	uint64_t                 : 4;
	uint64_t PT_address      : 40;
	uint64_t                 : 11;
	uint64_t execute_disable : 1;
} PDE_indirect;

typedef union {
	struct {
		uint64_t           : 7;
		uint64_t page_size : 1;
		uint64_t           : 58;
	};
	PDE_direct   direct;
	PDE_indirect indirect;
} PDE;

typedef struct {
	uint64_t present         : 1;
	uint64_t writable        : 1;
	uint64_t user            : 1;
	uint64_t write_through   : 1;
	uint64_t cache_disable   : 1;
	uint64_t accessed        : 1;
	uint64_t dirty           : 1;
	uint64_t PAT             : 1;
	uint64_t global          : 1;
	uint64_t                 : 3;
	uint64_t address         : 40; // Maps a 4 kiB page
	uint64_t                 : 11;
	uint64_t execute_disable : 1;
} PTE;

typedef __attribute__ ((aligned (0x1000))) PML4E PML4_table [512];
typedef __attribute__ ((aligned (0x1000))) PDPTE PDP_table [512];
typedef __attribute__ ((aligned (0x1000))) PDE Page_directory [512];
typedef __attribute__ ((aligned (0x1000))) PTE Page_table [512];

#endif
