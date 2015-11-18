#ifndef _VH_BFN2_H_
#define _VH_BFN2_H_

typedef struct { char szName[30]; char szZ; char attrs; } stBFN2HDR;
typedef struct { __int32 offs; u8 t; u8 w; u8 h; u8 res; } stBFN2CHR;

// ------------------------------------
// BFN2
// ------------------------------------
// stBFN2HDR sHDR; // [Header] / 32 bytes
// stBFN2CHR sCHR[256]; //  [OFFSET, TOP, W, H, res ] x 256 = 5*256 = 1280 bytes
// [U8 DATA] = ... bytes


#endif
