#ifndef GEOM_H_
#define GEOM_H_

#include "cgmath/cgmath.h"

struct sphere {
	cgm_vec3 pos;
	float rad;
};

struct aabox {
	cgm_vec3 vmin, vmax;
};

#endif	/* GEOM_H_ */
