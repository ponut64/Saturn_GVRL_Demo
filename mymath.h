#ifndef __MYMATH_H__
# define __MYMATH_H__

//I'm not renaming these because Jo Engine *is* where I got them. ... but its pretty basic stuff.

# define JO_ABS(X)                  ((X) < 0 ? -(X) : (X))

# define JO_MIN(A, B)               (((A) < (B)) ? (A) : (B))

# define JO_MAX(A, B)               (((A) > (B)) ? (A) : (B))

FIXED		fxm(FIXED d1, FIXED d2);
FIXED		fxdot(VECTOR ptA, VECTOR ptB);
FIXED		fxdiv(FIXED dividend, FIXED divisor);

FIXED		approximate_distance(FIXED * p0, FIXED * p1);

void	segment_to_vector(FIXED * start, FIXED * end, FIXED * out);

void	normalize(FIXED vector_in[XYZ], FIXED vector_out[XYZ]);
void	double_normalize(FIXED vector_in[XYZ], FIXED vector_out[XYZ]);
void	project_to_segment(POINT tgt, POINT p1, POINT p2, POINT outPt, VECTOR outV);
Bool	isPointonSegment(FIXED point[XYZ], FIXED start[XYZ], FIXED end[XYZ]);
void	cross_fixed(FIXED vector1[XYZ], FIXED vector2[XYZ], FIXED output[XYZ]);
Uint8	solve_domain(FIXED normal[XYZ]);
FIXED	pt_col_plane(FIXED planept[XYZ], FIXED ptoffset[XYZ], FIXED normal[XYZ], FIXED unitNormal[XYZ], FIXED offset[XYZ]);
int		ptalt_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ]);
FIXED	realpt_to_plane(FIXED ptreal[XYZ], FIXED normal[XYZ], FIXED offset[XYZ]);
Bool	line_hit_plane_here(FIXED p0[XYZ], FIXED p1[XYZ], FIXED centreFace[XYZ], FIXED unitNormal[XYZ], FIXED offset[XYZ], FIXED output[XYZ]);
void	print_from_id(Uint8 normid, Uint8 spotX, Uint8 spotY);

void *	align_4(void * ptr);

#endif

