#ifndef __COLLISION_H__
#define __COLLISION_H__

#include "bounder.h"

#define HIT_TOLERANCE (6553)

//////////////////////////////////
//	Numerical Normal ID Shorthands
//////////////////////////////////
#define N_Xp (0)
#define N_Xn (1)
#define N_Yp (2)
#define N_Yn (3)
#define N_Zp (4)
#define N_Zn (5)

typedef struct {
	FIXED xp0[XYZ];
	FIXED xp1[XYZ];
	FIXED yp0[XYZ];
	FIXED yp1[XYZ];
	FIXED zp0[XYZ];
	FIXED zp1[XYZ];
} _lineTable;

Bool 	sort_collide(FIXED pos[XYZ], _boundBox * targetBox, int* nearNormalID, int tolerance);
int		test_collide_boxes(_boundBox * stator, _boundBox * mover);

#endif

