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

void aabox_sphere_insc(const struct aabox *box, struct sphere *sph);
void aabox_sphere_circ(const struct aabox *box, struct sphere *sph);

#endif	/* GEOM_H_ */
