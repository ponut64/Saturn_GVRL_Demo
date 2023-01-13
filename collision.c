/*
This file is compiled separately.
*/
///collision.c

#include <SL_DEF.H>
#include "def.h"
#include "mymath.h"
#include "bounder.h"
#include "mloader.h"

#include "collision.h"

int numBoxChecks = 0;

Bool sort_collide(FIXED pos[XYZ], _boundBox * targetBox, int * nearNormalID, int tolerance)
{
	int dotX = realpt_to_plane(pos, targetBox->Xplus, targetBox->pos);
		if(dotX < tolerance){return 0;}
	int dotNX = realpt_to_plane(pos, targetBox->Xneg, targetBox->pos);
		if(dotNX < tolerance){return 0;}
	int dotY = realpt_to_plane(pos, targetBox->Yplus, targetBox->pos);
		if(dotY < tolerance){return 0;}
	int dotNY = realpt_to_plane(pos, targetBox->Yneg, targetBox->pos);
		if(dotNY < tolerance){return 0;}
	int dotZ = realpt_to_plane(pos, targetBox->Zplus, targetBox->pos);
		if(dotZ < tolerance){return 0;}
	int dotNZ = realpt_to_plane(pos, targetBox->Zneg, targetBox->pos);
		if(dotNZ < tolerance){return 0;}

	int leastDistance;
		leastDistance = JO_MIN(JO_MIN(JO_MIN(JO_MIN(JO_MIN(JO_ABS(dotZ), JO_ABS(dotNZ)), JO_ABS(dotX)), JO_ABS(dotNX)), JO_ABS(dotY)), JO_ABS(dotNY));
		if(leastDistance == JO_ABS(dotZ) ){
			*nearNormalID = N_Zp;
		} else if(leastDistance == JO_ABS(dotNZ) ){
			*nearNormalID = N_Zn;
		} else if(leastDistance == JO_ABS(dotX) ){
			*nearNormalID = N_Xp;
		} else if(leastDistance == JO_ABS(dotNX) ){
			*nearNormalID = N_Xn;
		} else if(leastDistance == JO_ABS(dotY) ){
			*nearNormalID = N_Yp;
		} else if(leastDistance == JO_ABS(dotNY) ){
			*nearNormalID = N_Yn;
			}
//End surface collision detection.

	return 1;
}	

int	test_collide_boxes(_boundBox * stator, _boundBox * mover)
{

static FIXED bigRadius = 0;

static POINT centerDif = {0, 0, 0};

static POINT lineEnds[9];

static Bool lineChecks[9];

static int hitFace;
		
static FIXED bigDif = 0;


//Box Populated Check
// if(stator->status[1] != 'C'){
	// return 0;
// }

//Box Distance Culling Check
bigRadius = JO_MAX(JO_MAX(JO_MAX(JO_MAX(JO_ABS(stator->Xplus[X]), JO_ABS(stator->Xplus[Y])), JO_ABS(stator->Xplus[Z])),
		JO_MAX(JO_MAX(JO_ABS(stator->Yplus[X]), JO_ABS(stator->Yplus[Y])), JO_ABS(stator->Yplus[Z]))),
		JO_MAX(JO_MAX(JO_ABS(stator->Zplus[X]), JO_ABS(stator->Zplus[Y])), JO_ABS(stator->Zplus[Z])));
		

centerDif[X] = stator->pos[X] + mover->pos[X];
centerDif[Y] = stator->pos[Y] + mover->pos[Y];
centerDif[Z] = stator->pos[Z] + mover->pos[Z];


bigDif = JO_MAX(JO_MAX(JO_ABS(centerDif[X]), JO_ABS(centerDif[Y])),JO_ABS(centerDif[Z]));

if(bigDif > (bigRadius + (20<<16))) return -1;

numBoxChecks++;

//Box Collision Check
_lineTable moverCFs = {
	.xp0[X] = mover->Xplus[X] 	- mover->pos[X] - mover->velocity[X],
	.xp0[Y] = mover->Xplus[Y] 	- mover->pos[Y] - mover->velocity[Y],
	.xp0[Z] = mover->Xplus[Z] 	- mover->pos[Z] - mover->velocity[Z],
	.xp1[X] = mover->Xneg[X] 	- mover->pos[X] + mover->velocity[X],
	.xp1[Y] = mover->Xneg[Y] 	- mover->pos[Y] + mover->velocity[Y],
	.xp1[Z] = mover->Xneg[Z] 	- mover->pos[Z] + mover->velocity[Z],
	.yp0[X] = mover->Yplus[X] 	- mover->pos[X] - mover->velocity[X],
	.yp0[Y] = mover->Yplus[Y] 	- mover->pos[Y] - mover->velocity[Y],
	.yp0[Z] = mover->Yplus[Z] 	- mover->pos[Z] - mover->velocity[Z],
	.yp1[X] = mover->Yneg[X] 	- mover->pos[X] + mover->velocity[X],
	.yp1[Y] = mover->Yneg[Y] 	- mover->pos[Y] + mover->velocity[Y],
	.yp1[Z] = mover->Yneg[Z] 	- mover->pos[Z] + mover->velocity[Z],
	.zp0[X] = mover->Zplus[X] 	- mover->pos[X] - mover->velocity[X],
	.zp0[Y] = mover->Zplus[Y] 	- mover->pos[Y] - mover->velocity[Y],
	.zp0[Z] = mover->Zplus[Z] 	- mover->pos[Z] - mover->velocity[Z],
	.zp1[X] = mover->Zneg[X] 	- mover->pos[X] + mover->velocity[X],
	.zp1[Y] = mover->Zneg[Y] 	- mover->pos[Y] + mover->velocity[Y],
	.zp1[Z] = mover->Zneg[Z] 	- mover->pos[Z] + mover->velocity[Z]
}; 

		/*
	ABSOLUTE PRIORITY: Once again, the normal hit during collision must be found ABSOLUTELY. ACCURATELY.
	How we did this best before:
	Step 0: Determine which faces face you.
	Step 1: Draw lines to a point from every pair of CFs on the mover to every facing face of the stator->
	Step 2: Test the points and find which one collides and which face it collided with.
	Step 3: Use the normal of that face for collision.

		*/
		

//	Step 0: Determine which faces face you.

	POINT negCenter = {-mover->pos[X], -mover->pos[Y], -mover->pos[Z]};
	//This math figures out what side of the box we're on, with respect to its rotation, too.
	centerDif[X] = realpt_to_plane(negCenter, stator->UVX, stator->pos);
	centerDif[Y] = realpt_to_plane(negCenter, stator->UVY, stator->pos);
	centerDif[Z] = realpt_to_plane(negCenter, stator->UVZ, stator->pos);
//	Step 1: Draw lines to a point from every pair of CFs on the mover to every face of the stator->
	if( centerDif[X] < 0){
		//This means we draw to X+
		lineChecks[0] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Xplus, stator->UVX, stator->pos, lineEnds[0]);
		lineChecks[1] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Xplus, stator->UVX, stator->pos, lineEnds[1]);
		lineChecks[2] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Xplus, stator->UVX, stator->pos, lineEnds[2]);
	} else {
		//This means we draw to X-
		lineChecks[0] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Xneg, stator->UVX, stator->pos, lineEnds[0]);
		lineChecks[1] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Xneg, stator->UVX, stator->pos, lineEnds[1]);
		lineChecks[2] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Xneg, stator->UVX, stator->pos, lineEnds[2]);
	}
	
	if( centerDif[Y] < 0){
		//This means we draw to Y+
		lineChecks[3] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Yplus, stator->UVY, stator->pos, lineEnds[3]);
		lineChecks[4] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Yplus, stator->UVY, stator->pos, lineEnds[4]);
		lineChecks[5] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Yplus, stator->UVY, stator->pos, lineEnds[5]);
	} else {
		//This means we draw to Y-
		lineChecks[3] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Yneg, stator->UVY, stator->pos, lineEnds[3]);
		lineChecks[4] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Yneg, stator->UVY, stator->pos, lineEnds[4]);
		lineChecks[5] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Yneg, stator->UVY, stator->pos, lineEnds[5]);
	}
	
	if( centerDif[Z] < 0){
		//This means we draw to Z+
		lineChecks[6] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Zplus, stator->UVZ, stator->pos, lineEnds[6]);
		lineChecks[7] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Zplus, stator->UVZ, stator->pos, lineEnds[7]);
		lineChecks[8] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Zplus, stator->UVZ, stator->pos, lineEnds[8]);
	} else {
		//This means we draw to Z-
		lineChecks[6] = line_hit_plane_here(moverCFs.xp0, moverCFs.xp1, stator->Zneg, stator->UVZ, stator->pos, lineEnds[6]);
		lineChecks[7] = line_hit_plane_here(moverCFs.yp0, moverCFs.yp1, stator->Zneg, stator->UVZ, stator->pos, lineEnds[7]);
		lineChecks[8] = line_hit_plane_here(moverCFs.zp0, moverCFs.zp1, stator->Zneg, stator->UVZ, stator->pos, lineEnds[8]);
	}
	
//	jo_printf(13, 12, "(%i)", hitFace);

		//Step 2: Test the points and find which one collides and which face it collided with.
	for(int i = 0; i < 9; i++){
		if(lineChecks[i] == 1){
			if(sort_collide(lineEnds[i], stator, &hitFace, -HIT_TOLERANCE) == 1){
				//Step 3: Use the normal of that face for collision.
				return hitFace;
			}
		}
		
	}
		return -1;
}



