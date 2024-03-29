#include <SL_DEF.H>
#include "def.h"
#include "mymath.h"
/*
	Deflection
	//d - 2 * DotProduct(d,n) * n
	d = direction vector
	n = normal vector
*/

#define MATH_TOLERANCE (16384)

	static int realNormal[XYZ] = {0, 0, 0};
	static int realpt[XYZ] = {0, 0, 0};
	static FIXED pFNn[XYZ] = {0, 0, 0};


inline FIXED		fxm(FIXED d1, FIXED d2) //Fixed Point Multiplication
{
	register volatile FIXED rtval;
	asm(
	"dmuls.l %[d1],%[d2];"
	"sts MACH,r1;"		// Store system register [sts] , high of 64-bit register MAC to r1
	"sts MACL,%[out];"	// Low of 64-bit register MAC to the register of output param "out"
	"xtrct r1,%[out];" 	//This whole procress gets the middle 32-bits of 32 * 32 -> (2x32 bit registers)
    :    [out] "=r" (rtval)				//OUT
    :    [d1] "r" (d1), [d2] "r" (d2)	//IN
	:	"r1", "mach", "macl"		//CLOBBERS
	);
	return rtval;
}


inline FIXED	fxdot(VECTOR ptA, VECTOR ptB) //Fixed-point dot product
{
	register volatile FIXED rtval;
	asm(
		"clrmac;"
		"mac.l @%[ptr1]+,@%[ptr2]+;"
		"mac.l @%[ptr1]+,@%[ptr2]+;"
		"mac.l @%[ptr1]+,@%[ptr2]+;"
		"sts MACH,r1;"
		"sts MACL,%[ox];"
		"xtrct r1,%[ox];"
		: 	[ox] "=r" (rtval), [ptr1] "+p" (ptA) , [ptr2] "+p" (ptB)	//OUT
		:																//IN
		:	"r1", "mach", "macl"										//CLOBBERS
	);
	return rtval;
}



inline FIXED	fxdiv(FIXED dividend, FIXED divisor) //Fixed-point division
{
	
	volatile int * DVSR = ( int*)0xFFFFFF00;
	volatile int * DVDNTH = ( int*)0xFFFFFF10;
	volatile int * DVDNTL = ( int*)0xFFFFFF14;

	*DVSR = divisor;
	*DVDNTH = (dividend>>16);
	*DVDNTL = (dividend<<16);
	return *DVDNTL;

	// register volatile FIXED quotient;
	// asm(
	// "mov.l %[dvs], @%[dvsr];"
	// "mov %[dvd], r1;" //Move the dividend to a general-purpose register, to prevent weird misreading of data.
	// "shlr16 r1;"
	// "exts.w r1, r1;" //Sign extension in case value is negative
	// "mov.l r1, @%[nth];" //Expresses "*DVDNTH = dividend>>16"
	// "mov %[dvd], r1;" 
	// "shll16 r1;"
	// "mov.l r1, @%[ntl];" //Expresses *DVDNTL = dividend<<16";
	// "mov.l @%[ntl], %[out];" //Get result.
		// : 	[out] "=r" (quotient)											//OUT
		// :	[dvs] "r" (divisor) , [dvd] "r" (dividend) ,					//IN
			// [dvsr] "r" (DVSR) ,	[nth] "r" (DVDNTH) , [ntl] "r" (DVDNTL)		//IN
		// :	"r1"															//CLOBBERS
	// );
	// return quotient;
}

//////////////////////////////////
// Shorthand to turn two points (to represent a segment) into a vector
//////////////////////////////////
void	segment_to_vector(FIXED * start, FIXED * end, FIXED * out)
{
	out[X] = (start[X] - end[X]);
	out[Y] = (start[Y] - end[Y]);
	out[Z] = (start[Z] - end[Z]);
}

//////////////////////////////////
// Manhattan
//
// Cube root scalar.
// 1.25992104989ish / 3 = 0.2467ish * 65536 = 16168
//
//////////////////////////////////
/* FIXED		approximate_distance(FIXED * p0, FIXED * p1)
{
	// POINT difference;
	// segment_to_vector(p0, p1, difference);
	// int max = JO_MAX(JO_ABS(difference[X]), JO_MAX(JO_ABS(difference[Y]), JO_ABS(difference[Z])));
	// if(max == JO_ABS(difference[X]))
	// {
		// return JO_ABS(p0[X] - p1[X]) + fxm(JO_ABS(p0[Y] - p1[Y]), 16168) + fxm(JO_ABS(p0[Z] - p1[Z]), 16168);
	// } else if(max == JO_ABS(difference[Y]))
	// {
		// return JO_ABS(p0[Y] - p1[Y]) + fxm(JO_ABS(p0[X] - p1[X]), 16168) + fxm(JO_ABS(p0[Z] - p1[Z]), 16168);
	// } else {
		// return JO_ABS(p0[Z] - p1[Z]) + fxm(JO_ABS(p0[Y] - p1[Y]), 16168) + fxm(JO_ABS(p0[X] - p1[X]), 16168);
	// }
	return (JO_ABS(p0[X] - p1[X]) + JO_ABS(p0[Y] - p1[Y]) + JO_ABS(p0[Z] - p1[Z]));
}
 */

//////////////////////////////////
// "fast inverse square root", but fixed-point
//////////////////////////////////
FIXED		fxisqrt(FIXED input){
	
	FIXED xSR = 0;
	FIXED pushrsamp = 0;
	FIXED msb = 0;
	FIXED shoffset = 0;
	FIXED yIsqr = 0;
	
	if(input <= 65536){
		return 65536;
	}
	
	xSR = input>>1;
	pushrsamp = input;
	
	while(pushrsamp >= 65536){
		pushrsamp >>=1;
		msb++;
	}

	shoffset = (16 - ((msb)>>1));
	yIsqr = 1<<shoffset;
	//y = (y * (98304 - ( ( (x>>1) * ((y * y)>>16 ) )>>16 ) ) )>>16;   x2
	return (fxm(yIsqr, (98304 - fxm(xSR, fxm(yIsqr, yIsqr)))));
}

//////////////////////////////////
// "fast inverse square root x2", but fixed-point
//////////////////////////////////
FIXED		double_fxisqrt(FIXED input){
	
	FIXED xSR = 0;
	FIXED pushrsamp = 0;
	FIXED msb = 0;
	FIXED shoffset = 0;
	FIXED yIsqr = 0;
	
	if(input <= 65536){
		return 65536;
	}
	
	xSR = input>>1;
	pushrsamp = input;
	
	while(pushrsamp >= 65536){
		pushrsamp >>=1;
		msb++;
	}

	shoffset = (16 - ((msb)>>1));
	yIsqr = 1<<shoffset;
	//y = (y * (98304 - ( ( (x>>1) * ((y * y)>>16 ) )>>16 ) ) )>>16;   x2
	yIsqr = (fxm(yIsqr, (98304 - fxm(xSR, fxm(yIsqr, yIsqr)))));
	return (fxm(yIsqr, (98304 - fxm(xSR, fxm(yIsqr, yIsqr)))));
}


void	normalize(FIXED vector_in[XYZ], FIXED vector_out[XYZ])
{
	//Shift inputs rsamp by 8, to prevent overflow.
	static FIXED vmag = 0;
	vmag = fxisqrt(fxm(vector_in[X],vector_in[X]) + fxm(vector_in[Y],vector_in[Y]) + fxm(vector_in[Z],vector_in[Z]));
	vector_out[X] = fxm(vmag, vector_in[X]);
	vector_out[Y] = fxm(vmag, vector_in[Y]);
	vector_out[Z] = fxm(vmag, vector_in[Z]);
}

void	double_normalize(FIXED vector_in[XYZ], FIXED vector_out[XYZ])
{
	//Shift inputs rsamp by 8, to prevent overflow.
	static FIXED vmag = 0;
	vmag = double_fxisqrt(fxm(vector_in[X],vector_in[X]) + fxm(vector_in[Y],vector_in[Y]) + fxm(vector_in[Z],vector_in[Z]));
	vector_out[X] = fxm(vmag, vector_in[X]);
	vector_out[Y] = fxm(vmag, vector_in[Y]);
	vector_out[Z] = fxm(vmag, vector_in[Z]);
}

//////////////////////////////////
// Checks if "point" is between "start" and "end".
//////////////////////////////////
Bool	isPointonSegment(FIXED point[XYZ], FIXED start[XYZ], FIXED end[XYZ])
{
	FIXED max[XYZ];
	FIXED min[XYZ];
	
	max[X] = JO_MAX(start[X], end[X]);
	max[Y] = JO_MAX(start[Y], end[Y]);
	max[Z] = JO_MAX(start[Z], end[Z]);

	min[X] = JO_MIN(start[X], end[X]);
	min[Y] = JO_MIN(start[Y], end[Y]);
	min[Z] = JO_MIN(start[Z], end[Z]);
	
	if(point[X] >= (min[X] - MATH_TOLERANCE) && point[X] <= (max[X] + MATH_TOLERANCE) &&
		point[Y] >= (min[Y] - MATH_TOLERANCE) && point[Y] <= (max[Y] + MATH_TOLERANCE) &&
		point[Z] >= (min[Z] - MATH_TOLERANCE) && point[Z] <= (max[Z] + MATH_TOLERANCE)){
				return 1;
	} else {
		return 0;
	}
}
 
//////////////////////////////////
// Given a segment represented by two points "p1" and "p2,
// project a third point "tgt" onto the the same segment.
// Method is: Unit vector "p1->p2", dot product that unit with tgt, multiply dot result with unit.
//////////////////////////////////
void	project_to_segment(POINT tgt, POINT p1, POINT p2, POINT outPt, VECTOR outV)
{
	
	VECTOR vectorized_pt;
	//Following makes a vector from the left/right centers.
	outV[X] = (p1[X] - p2[X]);
	outV[Y] = (p1[Y] - p2[Y]);
	outV[Z] = (p1[Z] - p2[Z]);
	
	if(JO_ABS(outV[X]) >= (SQUARE_MAX) || JO_ABS(outV[Y]) >= (SQUARE_MAX) || JO_ABS(outV[Z]) >= (SQUARE_MAX))
	{
	//Normalization that covers values >SQUARE_MAX
	int vmag = slSquart( ((outV[X]>>16) * (outV[X]>>16)) + ((outV[Y]>>16) * (outV[Y]>>16)) + ((outV[Z]>>16) * (outV[Z]>>16)) )<<16;
	vmag = slDivFX(vmag, 65536);
	//
	vectorized_pt[X] = fxm(outV[X], vmag);
	vectorized_pt[Y] = fxm(outV[Y], vmag);
	vectorized_pt[Z] = fxm(outV[Z], vmag);
	} else {
	normalize(outV, vectorized_pt);
	}
	//
	//Generatr a scalar for the source vector
	int scaler = slInnerProduct(vectorized_pt, tgt);
	//Scale the vector to project onto ; using scalar as a represenation of how far along the vector it is
	outPt[X] = fxm(scaler, vectorized_pt[X]);
	outPt[Y] = fxm(scaler, vectorized_pt[Y]);
	outPt[Z] = fxm(scaler, vectorized_pt[Z]);
	
	
}

void	cross_fixed(FIXED vector1[XYZ], FIXED vector2[XYZ], FIXED output[XYZ])
{
	output[X] = fxm(vector1[Y], vector2[Z]) - fxm(vector1[Z], vector2[Y]);
	output[Y] = fxm(vector1[Z], vector2[X]) - fxm(vector1[X], vector2[Z]);
	output[Z] = fxm(vector1[X], vector2[Y]) - fxm(vector1[Y], vector2[X]);
}

//////////////////////////////////
//A helper function which checks the X and Z signs of a vector to find its domain.
//////////////////////////////////
Uint8	solve_domain(FIXED normal[XYZ]){
	if(normal[X] >= 0 && normal[Z] >= 0){
		//PP
		return 1;
	} else if(normal[X] >= 0 && normal[Z] < 0){
		//PN
		return 2;
	} else if(normal[X] < 0 && normal[Z] >= 0){
		//NP
		return 3;
	} else if(normal[X] < 0 && normal[Z] < 0){
		//NN
		return 4;
	};
	/*
	3	-	1
	4	-	2
	*/
	return 0;
}

FIXED	pt_col_plane(FIXED planept[XYZ], FIXED ptoffset[XYZ], FIXED normal[XYZ], FIXED unitNormal[XYZ], FIXED offset[XYZ])
{
	//Using a NORMAL OF A PLANE which is also a POINT ON THE PLANE and checking IF A POINT IS ON THAT PLANE
	//the REAL POSITION of the normal, which is also a POINT ON THE PLANE, needs an actual position. WE FIND IT HERE.
	realNormal[X] = normal[X] + (offset[X]);
	realNormal[Y] = normal[Y] + (offset[Y]);
	realNormal[Z] = normal[Z] + (offset[Z]);
	realpt[X] = planept[X] + (ptoffset[X]);
	realpt[Y] = planept[Y] + (ptoffset[Y]);
	realpt[Z] = planept[Z] + (ptoffset[Z]);
	//the DIFFERENCE between a POSSIBLE POINT ON THE PLANE, and a KNOWN POINT ON THE PLANE, must use the REAL POSITION of the NORMAL POINT.
	pFNn[X] = realNormal[X] - realpt[X];
	pFNn[Y] = realNormal[Y] - realpt[Y];
	pFNn[Z] = realNormal[Z] - realpt[Z];
	//The NORMAL of the plane has NO REAL POSITION. it is FROM ORIGIN. We use the normal here.
	//If the dot product here is zero, the point lies on the plane.
	return fxdot(pFNn, unitNormal);
}

int	ptalt_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ]) //Shifts down the pFNn to suppress overflows
{
	realNormal[X] = normal[X] + (offset[X]);
	realNormal[Y] = normal[Y] + (offset[Y]);
	realNormal[Z] = normal[Z] + (offset[Z]);
	pFNn[X] = (realNormal[X] - ptreal[X])>>8;
	pFNn[Y] = (realNormal[Y] - ptreal[Y])>>8;
	pFNn[Z] = (realNormal[Z] - ptreal[Z])>>8;
	return fxdot(pFNn, normal);
}

FIXED	realpt_to_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ])
{
	realNormal[X] = normal[X] + (offset[X]);
	realNormal[Y] = normal[Y] + (offset[Y]);
	realNormal[Z] = normal[Z] + (offset[Z]);
	pFNn[X] = realNormal[X] - ptreal[X];
	pFNn[Y] = realNormal[Y] - ptreal[Y];
	pFNn[Z] = realNormal[Z] - ptreal[Z];
	return fxdot(pFNn, normal);
}

//////////////////////////////////
// Line-to-plane projection function
// Line: p0->p1
// point_on_plane : a point on the plane
// unitNormal : the unit vector normal of the plane
// offset : world-space position of the point_on_plane. If point_on_plane is already, substitute with "zPt". (do not leave blank)
// output : the point at which the line intersects the plane
// return value : whether or not the output point is between p0 and p1
//////////////////////////////////
Bool	line_hit_plane_here(FIXED p0[XYZ], FIXED p1[XYZ], FIXED point_on_plane[XYZ], FIXED unitNormal[XYZ], FIXED offset[XYZ], FIXED output[XYZ])
{

	FIXED line_scalar;
	FIXED vector_of_line[XYZ];
	FIXED vector_to_plane[XYZ];
	
	vector_of_line[X] = p0[X] - p1[X];
	vector_of_line[Y] = p0[Y] - p1[Y];
	vector_of_line[Z] = p0[Z] - p1[Z];

	vector_to_plane[X] = (point_on_plane[X] + (offset[X])) - p0[X];
	vector_to_plane[Y] = (point_on_plane[Y] + (offset[Y])) - p0[Y];
	vector_to_plane[Z] = (point_on_plane[Z] + (offset[Z])) - p0[Z];

	line_scalar = slDivFX(slInnerProduct(vector_of_line, unitNormal), slInnerProduct(vector_to_plane, unitNormal));
	if(line_scalar > (1000<<16) || line_scalar < -(1000<<16)){
		return 0;
	}
	
	output[X] = (p0[X] + fxm(vector_of_line[X], line_scalar));
	output[Y] = (p0[Y] + fxm(vector_of_line[Y], line_scalar));
	output[Z] = (p0[Z] + fxm(vector_of_line[Z], line_scalar));

	return isPointonSegment(output, p0, p1);
}


void * align_4(void * ptr)
{
	ptr += (((unsigned int)ptr) & 1) ? 1 : 0;
	ptr += (((unsigned int)ptr) & 2) ? 1 : 0;
	return ptr;
}

