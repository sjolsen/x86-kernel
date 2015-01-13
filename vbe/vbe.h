#ifndef VBE_H
#define VBE_H

#include <stdint.h>

typedef struct vbeptr {
	uint16_t offset;
	uint16_t segment;
} vbeptr;

static inline
void* decode_vbeptr (vbeptr ptr)
{
	return (void*) (((uintptr_t) ptr.segment << 4) + ptr.offset);
}

typedef struct __attribute__ ((packed)) VbeInfoBlock {
	uint8_t  VbeSignature [4];
	uint16_t VbeVersion;
	vbeptr   OemStringPtr;
	uint8_t  Capabilities [4];
	vbeptr   VideoModePtr;
	uint16_t TotalMemory;
	uint16_t OemSoftwareRev;
	vbeptr   OemVendorNamePtr;
	vbeptr   OemProductNamePtr;
	vbeptr   OemProductRevPtr;
	uint8_t  Reserved [222];
	uint8_t  OemData [256];
} VbeInfoBlock;

typedef struct __attribute__ ((packed)) ModeInfoBlock {
	uint16_t ModeAttributes;
	uint8_t  WinAAttributes;
	uint8_t  WinBAttributes;
	uint16_t WinGranularity;
	uint16_t WinSize;
	uint16_t WinASegment;
	uint16_t WinBSegment;
	uint32_t WinFuncPtr;
	uint16_t BytesPerScanLine;

	uint16_t XResolution;
	uint16_t YResolution;
	uint8_t  XCharSize;
	uint8_t  YCharSize;
	uint8_t  NumberOfPlanes;
	uint8_t  BitsPerPixel;
	uint8_t  NumberOfBanks;
	uint8_t  MemoryModel;
	uint8_t  BankSize;
	uint8_t  NumberOfImagePages;
	uint8_t  Reserved0;

	uint8_t  RedMaskSize;
	uint8_t  RedFieldPosition;
	uint8_t  GreenMaskSize;
	uint8_t  GreenFieldPosition;
	uint8_t  BlueMaskSize;
	uint8_t  BlueFieldPosition;
	uint8_t  RsvdMaskSize;
	uint8_t  RsvdFieldPosition;
	uint8_t  DirectColorModeInfo;

	uint32_t PhysBasePtr;
	uint32_t Reserved1;
	uint16_t Reserved2;

	uint16_t LinBytesPerScanLine;
	uint8_t  BnkNumberOfImagePages;
	uint8_t  LinNumberOfImagePages;
	uint8_t  LinRedMaskSize;
	uint8_t  LinRedFieldPosition;
	uint8_t  LinGreenMaskSize;
	uint8_t  LinGreenFieldPosition;
	uint8_t  LinBlueMaskSize;
	uint8_t  LinBlueFieldPosition;
	uint8_t  LinRsvdMaskSize;
	uint8_t  LinRsvdFieldPosition;
	uint8_t  MaxPixelClock;

	uint8_t  Reserved3 [189];
} ModeInfoBlock;

#endif
