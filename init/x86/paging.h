#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

/// Note: all addresses are physical and must be zero-extended past
/// the machine width. Addresses are stored as though their
/// least-significant bits are colocated with lower structure bits;
/// thus, addresses are aligned as if these bits were zero.

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
		uint64_t present   : 1;
		uint64_t           : 6;
		uint64_t page_size : 1;
		uint64_t           : 54;
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
		uint64_t present   : 1;
		uint64_t           : 6;
		uint64_t page_size : 1;
		uint64_t           : 56;
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
	uint64_t page_address    : 40; // Maps a 4 kiB page
	uint64_t                 : 11;
	uint64_t execute_disable : 1;
} PTE;

typedef __attribute__ ((aligned (0x1000))) PML4E PML4_table [512];
typedef __attribute__ ((aligned (0x1000))) PDPTE PDP_table [512];
typedef __attribute__ ((aligned (0x1000))) PDE Page_directory [512];
typedef __attribute__ ((aligned (0x1000))) PTE Page_table [512];

_Static_assert (sizeof (PML4E)          == 8, "PML4E not sized correctly");
_Static_assert (sizeof (PDPTE_direct)   == 8, "PDPTE_direct not sized correctly");
_Static_assert (sizeof (PDPTE_indirect) == 8, "PDPTE_indirect not sized correctly");
_Static_assert (sizeof (PDPTE)          == 8, "PDPTE not sized correctly");
_Static_assert (sizeof (PDE_direct)     == 8, "PDE_direct not sized correctly");
_Static_assert (sizeof (PDE_indirect)   == 8, "PDE_indirect not sized correctly");
_Static_assert (sizeof (PDE)            == 8, "PDE not sized correctly");
_Static_assert (sizeof (PTE)            == 8, "PTE not sized correctly");

#endif
