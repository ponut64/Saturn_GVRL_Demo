
#ifndef __DEF_H__
# define __DEF_H__

//def.h -- the catch-all "i dunno where else this goes" files

//////////////////////////////////
// Game Resolution

//////////////////////////////////
// Uniform grid cell information / shorthands
//////////////////////////////////
#define CELL_SIZE (2621440) // 40 << 16
#define INV_CELL_SIZE (1638) // 40 / 1
#define CELL_SIZE_INT (40)
#define MAP_V_SCALE (17) //Map data is shifted left by this amount
//////////////////////////////////
#define	HIMEM	(100679680)
#define HWRAM_MODEL_DATA_HEAP_SIZE (256 * 1024)
//////////////////////////////////
#define UNCACHE (0x20000000)
#define VDP2_RAMBASE (0x25E00000)
#define LWRAM (0x200000)
#define LWRAM_END (LWRAM + 0x100000)
//////////////////////////////////
// Polygon draw direction flipping flags
//////////////////////////////////
#define FLIPV (32)
#define FLIPH (16)
#define FLIPHV (48)
//////////////////////////////////
// Fixed point safe-square value
//////////////////////////////////
#define SQUARE_MAX (9633792) //147<<16
//////////////////////////////////
//	The line width of the polygon map area, in vertices (pix) and polygons (ply).
//	The total size is the square of these values.
//////////////////////////////////
#define LCL_MAP_PIX (17)
#define LCL_MAP_PLY (16)
//////////////////////////////////////////////////////////////////////////////
//Sound Numbers
//////////////////////////////////////////////////////////////////////////////

//Variables
extern int game_set_res;
extern POINT zPt;
extern POINT alwaysLow;
//Lives in main.c
extern short * division_table;
extern void * active_HWRAM_ptr;
//System
extern unsigned char * dirty_buf;
extern void * currentAddress;
extern int framerate;
extern int frmul;
extern volatile Uint32 * scuireg;


#endif

