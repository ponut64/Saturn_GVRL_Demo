/*
This file is compiled separately.
*/

///HIGH LEVEL ALERT///
/**THIS FUNCTION HAS HIGH PROBABILITY OF CAUSING CRITICAL GLITCHES WHEN IT DON'T WORK RIGHT**/
#include <SL_DEF.H>
#include "def.h"
#include "mymath.h"


#include "bounder.h"

_boundBox BoundBoxHost[MAX_PHYS_PROXY];
_boundBox * RBBs; 
_boundBox pl_RBB;
_object_arguments bound_box_starter;
Uint8 curBoxes = 0;

//Usage:
// param X, Y, Z: The location of the box (center)
// param xrot, yrot, zrot: the rotation of the box.
// param source_data->x_radius, source_data->y_radius, source_data->z_radius: the X, Y, and Z radius of the box.
//note: export models as -Y forward, Z up.
// param bbox: the bounding box struct to be modified.
void	makeBoundBox(_object_arguments * source_data)
{
	//Give the box its previous location
	source_data->modified_box->prevPos[X] = source_data->modified_box->pos[X];
	source_data->modified_box->prevPos[Y] = source_data->modified_box->pos[Y];
	source_data->modified_box->prevPos[Z] = source_data->modified_box->pos[Z];
	//Give the box its location
	source_data->modified_box->pos[X] = source_data->x_location;
	source_data->modified_box->pos[Y] = source_data->y_location;
	source_data->modified_box->pos[Z] = source_data->z_location;
	//Give the box its rotation
	source_data->modified_box->rot[X] = source_data->x_rotation;
	source_data->modified_box->rot[Y] = source_data->y_rotation;
	source_data->modified_box->rot[Z] = source_data->z_rotation;
	//Give the box its radius
	source_data->modified_box->radius[X] = source_data->x_radius;
	source_data->modified_box->radius[Y] = source_data->y_radius;
	source_data->modified_box->radius[Z] = source_data->z_radius;

	 FIXED sinX = slSin(source_data->modified_box->rot[X]);
	 FIXED cosX = slCos(source_data->modified_box->rot[X]);
	 FIXED sinY = slSin(source_data->modified_box->rot[Y]);
	 FIXED cosY = slCos(source_data->modified_box->rot[Y]);
	 FIXED sinZ = slSin(source_data->modified_box->rot[Z]);
	 FIXED cosZ = slCos(source_data->modified_box->rot[Z]);

	//SETUP UNIT VECTOR X
	source_data->modified_box->UVX[X] = fxm(cosY, cosZ);
	source_data->modified_box->UVX[Y] = fxm(sinZ, cosX) + fxm(fxm(sinX, sinY), cosZ);
	source_data->modified_box->UVX[Z] = -fxm(fxm(sinY, cosZ), cosX) + fxm(sinZ, sinX);
	
	source_data->modified_box->Xplus[X] = fxm((source_data->modified_box->radius[X]), source_data->modified_box->UVX[X]);
	source_data->modified_box->Xplus[Y] = fxm((source_data->modified_box->radius[X]), source_data->modified_box->UVX[Y]);
	source_data->modified_box->Xplus[Z] = fxm((source_data->modified_box->radius[X]), source_data->modified_box->UVX[Z]);

	//SETUP UNIT VECTOR Y
	source_data->modified_box->UVY[X] = -fxm(sinZ, cosY);
	source_data->modified_box->UVY[Y] = fxm(cosZ, cosX) - fxm(fxm(sinX, sinY), sinZ);
	source_data->modified_box->UVY[Z] = fxm(sinX, cosZ) + fxm(fxm(sinY, sinZ), cosX);

	source_data->modified_box->Yplus[X] = fxm((source_data->modified_box->radius[Y]), source_data->modified_box->UVY[X]);
	source_data->modified_box->Yplus[Y] = fxm((source_data->modified_box->radius[Y]), source_data->modified_box->UVY[Y]);
	source_data->modified_box->Yplus[Z] = fxm((source_data->modified_box->radius[Y]), source_data->modified_box->UVY[Z]);
	
	//SETUP UNIT VECTOR Z
	source_data->modified_box->UVZ[X] = (sinY);
	source_data->modified_box->UVZ[Y] = -fxm(sinX, cosY);
	source_data->modified_box->UVZ[Z] = fxm(cosX, cosY);
	
	source_data->modified_box->Zplus[X] = fxm((source_data->modified_box->radius[Z]), source_data->modified_box->UVZ[X]);
	source_data->modified_box->Zplus[Y] = fxm((source_data->modified_box->radius[Z]), source_data->modified_box->UVZ[Y]);
	source_data->modified_box->Zplus[Z] = fxm((source_data->modified_box->radius[Z]), source_data->modified_box->UVZ[Z]);
	//---------------------------------------------------------------------------------------------------------------
	source_data->modified_box->UVNX[X] = -source_data->modified_box->UVX[X];
	source_data->modified_box->UVNX[Y] = -source_data->modified_box->UVX[Y];
	source_data->modified_box->UVNX[Z] = -source_data->modified_box->UVX[Z];
	source_data->modified_box->UVNY[X] = -source_data->modified_box->UVY[X];
	source_data->modified_box->UVNY[Y] = -source_data->modified_box->UVY[Y];
	source_data->modified_box->UVNY[Z] = -source_data->modified_box->UVY[Z];
	source_data->modified_box->UVNZ[X] = -source_data->modified_box->UVZ[X];
	source_data->modified_box->UVNZ[Y] = -source_data->modified_box->UVZ[Y];
	source_data->modified_box->UVNZ[Z] = -source_data->modified_box->UVZ[Z];
	//axis given: Y (on Z axis and does not change Y axis) circle going only right/left, forward/backward
	source_data->modified_box->Xneg[X] = -fxm((source_data->x_radius), source_data->modified_box->UVX[X]);
	source_data->modified_box->Xneg[Y] = -fxm((source_data->x_radius), source_data->modified_box->UVX[Y]);
	source_data->modified_box->Xneg[Z] = -fxm((source_data->x_radius), source_data->modified_box->UVX[Z]);
	//axis given: X (on Y axis and does not change X axis) circle going only up/down, forward/backward
	source_data->modified_box->Yneg[X] = -fxm((source_data->y_radius), source_data->modified_box->UVY[X]);
	source_data->modified_box->Yneg[Y] = -fxm((source_data->y_radius), source_data->modified_box->UVY[Y]);
	source_data->modified_box->Yneg[Z] = -fxm((source_data->y_radius), source_data->modified_box->UVY[Z]);
	//axis given: Z (on X axis and does not change Z axis) Circle going only up/down, left/right
	source_data->modified_box->Zneg[X] = -fxm((source_data->z_radius), source_data->modified_box->UVZ[X]);
	source_data->modified_box->Zneg[Y] = -fxm((source_data->z_radius), source_data->modified_box->UVZ[Y]);
	source_data->modified_box->Zneg[Z] = -fxm((source_data->z_radius), source_data->modified_box->UVZ[Z]);
	//end of negative
	
	//Sort and assign X, Y, and Z maximum normals. (For macros)
	//Warning: Sorting is GONE. :(

	//Determine a velocity from the difference of current and last position
	segment_to_vector(source_data->modified_box->prevPos, source_data->modified_box->pos, source_data->modified_box->velocity);
}

//Usage:
// param X, Y, Z: The location of the box (center)
// param xrot, yrot, zrot: the rotation of the box.
// param source_data->x_radius, source_data->y_radius, source_data->z_radius: the X, Y, and Z radius of the box.
// param bbox: the bounding box struct to be modified.
//Modified version to suit a Y axis gimbal lock. In other words, Y rotation is around a fixed post.
///Also, this doesn't make a box. It's just matrix generation, but with a radius and some velocities sprinkled on top; you can _make_ a box from it.
void	make2AxisBox(_object_arguments * source_data)
{
	//Give the box its previous location
	source_data->modified_box->prevPos[X] = source_data->modified_box->pos[X];
	source_data->modified_box->prevPos[Y] = source_data->modified_box->pos[Y];
	source_data->modified_box->prevPos[Z] = source_data->modified_box->pos[Z];
	//Give the box its location
	source_data->modified_box->pos[X] = source_data->x_location;
	source_data->modified_box->pos[Y] = source_data->y_location;
	source_data->modified_box->pos[Z] = source_data->z_location;
	//Give the box its rotation
	source_data->modified_box->rot[X] = source_data->x_rotation;
	source_data->modified_box->rot[Y] = source_data->y_rotation;
	source_data->modified_box->rot[Z] = source_data->z_rotation;
	//Give the box its radius
	source_data->modified_box->radius[X] = source_data->x_radius;
	source_data->modified_box->radius[Y] = source_data->y_radius;
	source_data->modified_box->radius[Z] = source_data->z_radius;

	 FIXED sinX = slSin(source_data->modified_box->rot[X]);
	 FIXED cosX = slCos(source_data->modified_box->rot[X]);
	 FIXED sinY = slSin(source_data->modified_box->rot[Y]);
	 FIXED cosY = slCos(source_data->modified_box->rot[Y]);
	 FIXED sinZ = slSin(source_data->modified_box->rot[Z]);
	 FIXED cosZ = slCos(source_data->modified_box->rot[Z]);
	//SETUP UNIT VECTOR X
	///left - right points. Affected as: X rotation causes no movement. Y rotation causes movement on X-Z axis. Z rotation causes movement on Y-X axis.
	source_data->modified_box->UVX[X] = fxm(cosY, cosZ) - fxm(fxm(sinY, -sinX), -sinZ);
	source_data->modified_box->UVX[Y] = fxm(sinZ, cosX);
	source_data->modified_box->UVX[Z] = -fxm(sinY, cosZ) - fxm(fxm(-sinX, -sinZ), cosY);
	
	source_data->modified_box->Xplus[X] = fxm((source_data->x_radius), source_data->modified_box->UVX[X]);
	source_data->modified_box->Xplus[Y] = fxm((source_data->x_radius), source_data->modified_box->UVX[Y]);
	source_data->modified_box->Xplus[Z] = fxm((source_data->x_radius), source_data->modified_box->UVX[Z]);

	//SETUP UNIT VECTOR Y
	///up - down points. Affected as: X rotation causes movement on Y-Z axis. Y rotation causes no movement. Z rotation causes movement on Y-X axis.
	source_data->modified_box->UVY[X] = -fxm(fxm(sinY, sinX), cosZ) - fxm(sinZ, cosY);
	source_data->modified_box->UVY[Y] = fxm(cosX, cosZ);
	source_data->modified_box->UVY[Z] = fxm(fxm(-sinX, cosY), cosZ) + fxm(sinZ, sinY);

	source_data->modified_box->Yplus[X] = fxm((source_data->y_radius), source_data->modified_box->UVY[X]);
	source_data->modified_box->Yplus[Y] = fxm((source_data->y_radius), source_data->modified_box->UVY[Y]);
	source_data->modified_box->Yplus[Z] = fxm((source_data->y_radius), source_data->modified_box->UVY[Z]);

	///Fwd - back points. Affected as: Y rotation causes movement on X-Z axis. X rotation causes movement on Y-Z. Z rotation causes no movement.
	//SETUP UNIT VECTOR Z
	source_data->modified_box->UVZ[X] = fxm(sinY, cosX);
	source_data->modified_box->UVZ[Y] = (sinX);
	source_data->modified_box->UVZ[Z] = fxm(cosX, cosY);
	
	source_data->modified_box->Zplus[X] = fxm((source_data->z_radius), source_data->modified_box->UVZ[X]);
	source_data->modified_box->Zplus[Y] = fxm((source_data->z_radius), source_data->modified_box->UVZ[Y]);
	source_data->modified_box->Zplus[Z] = fxm((source_data->z_radius), source_data->modified_box->UVZ[Z]);
	//---------------------------------------------------------------------------------------------------------------
	source_data->modified_box->UVNX[X] = -source_data->modified_box->UVX[X];
	source_data->modified_box->UVNX[Y] = -source_data->modified_box->UVX[Y];
	source_data->modified_box->UVNX[Z] = -source_data->modified_box->UVX[Z];
	source_data->modified_box->UVNY[X] = -source_data->modified_box->UVY[X];
	source_data->modified_box->UVNY[Y] = -source_data->modified_box->UVY[Y];
	source_data->modified_box->UVNY[Z] = -source_data->modified_box->UVY[Z];
	source_data->modified_box->UVNZ[X] = -source_data->modified_box->UVZ[X];
	source_data->modified_box->UVNZ[Y] = -source_data->modified_box->UVZ[Y];
	source_data->modified_box->UVNZ[Z] = -source_data->modified_box->UVZ[Z];
	//axis given: Y (on Z axis and does not change Y axis) circle going only right/left, forward/backward
	source_data->modified_box->Xneg[X] = -fxm((source_data->x_radius), source_data->modified_box->UVX[X]);
	source_data->modified_box->Xneg[Y] = -fxm((source_data->x_radius), source_data->modified_box->UVX[Y]);
	source_data->modified_box->Xneg[Z] = -fxm((source_data->x_radius), source_data->modified_box->UVX[Z]);
	//axis given: X (on Y axis and does not change X axis) circle going only up/down, forward/backward
	source_data->modified_box->Yneg[X] = -fxm((source_data->y_radius), source_data->modified_box->UVY[X]);
	source_data->modified_box->Yneg[Y] = -fxm((source_data->y_radius), source_data->modified_box->UVY[Y]);
	source_data->modified_box->Yneg[Z] = -fxm((source_data->y_radius), source_data->modified_box->UVY[Z]);
	//axis given: Z (on X axis and does not change Z axis) Circle going only up/down, left/right
	source_data->modified_box->Zneg[X] = -fxm((source_data->z_radius), source_data->modified_box->UVZ[X]);
	source_data->modified_box->Zneg[Y] = -fxm((source_data->z_radius), source_data->modified_box->UVZ[Y]);
	source_data->modified_box->Zneg[Z] = -fxm((source_data->z_radius), source_data->modified_box->UVZ[Z]);
	//end of negative
	
	//Sort and assign X, Y, and Z maximum normals. (For macros)
	//Warning: Sorting is GONE. :(

	//Determine a velocity from the difference of current and last position
	segment_to_vector(source_data->modified_box->prevPos, source_data->modified_box->pos, source_data->modified_box->velocity);
}

void	flush_boxes(int start)
{
	
////////////////////////////////////////////////////
//Flush boxes
//This is somewhat important.
//It's kind of funny, the engine mostly works if you don't do this.
//Since every box was in a rational location to begin with, it will just render stuff off-screen.
//But for the light source data, it'll get confused as it will start using old box data
// that has a nonsensical 3D location.
////////////////////////////////////////////////////
	for(int f = start; f < MAX_PHYS_PROXY; f++)
	{
		RBBs[f].status[0] = 0;
		RBBs[f].status[1] = 0;
		RBBs[f].status[2] = 0;
	}
////////////////////////////////////////////////////
	
}

void	initBoxes(void){
		//The bound box / matrix parameter struct has to be uncached, since it is shared between master & slave.
		// But don't be stupid like I was for a long time. See, I used to put it in LWRAM. LWRAM is SLOW!!!!
		RBBs = (_boundBox *)((unsigned int)(&BoundBoxHost[0]) | UNCACHE);
	for(int x = 0; x<MAX_PHYS_PROXY; x++){
		RBBs[x].status[0] = 'N';
		RBBs[x].status[1] = 'N';
		RBBs[x].status[2] = 'N';
		RBBs[x].velocity[X] = 0;
		RBBs[x].velocity[Y] = 0;
		RBBs[x].velocity[Z] = 0;
		//Initialize as unit matrix
		RBBs[x].UVX[X] = 65535;	RBBs[x].UVY[X] = 0;		RBBs[x].UVZ[X] = 0;
		RBBs[x].UVX[Y] = 0;		RBBs[x].UVY[Y] = 65535;	RBBs[x].UVZ[Y] = 0;
		RBBs[x].UVX[Z] = 0;		RBBs[x].UVY[Z] = 0;		RBBs[x].UVZ[Z] = 65535;
		
		RBBs[x].pos[X] = 0;	RBBs[x].rot[X] = 0;	RBBs[x].radius[X] = 0;	 
        RBBs[x].pos[Y] = 0;	RBBs[x].rot[Y] = 0;	RBBs[x].radius[Y] = 0;	
		RBBs[x].pos[Z] = 0;	RBBs[x].rot[Z] = 0;	RBBs[x].radius[Z] = 0;	
	}
}
