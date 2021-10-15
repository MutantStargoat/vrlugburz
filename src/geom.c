#include "geom.h"

void aabox_sphere_insc(const struct aabox *box, struct sphere *sph)
{
	float rx = (box->vmax.x - box->vmin.x) / 2.0f;
	float ry = (box->vmax.y - box->vmin.y) / 2.0f;
	float rz = (box->vmax.z - box->vmin.z) / 2.0f;

	sph->pos.x = box->vmin.x + rx;
	sph->pos.y = box->vmin.y + ry;
	sph->pos.z = box->vmin.z + rz;

	sph->rad = fabs(rx) > fabs(ry) ? rx : ry;
	if(fabs(rz) > fabs(sph->rad)) sph->rad = rz;
}

void aabox_sphere_circ(const struct aabox *box, struct sphere *sph)
{
	float rx = (box->vmax.x - box->vmin.x) / 2.0f;
	float ry = (box->vmax.y - box->vmin.y) / 2.0f;
	float rz = (box->vmax.z - box->vmin.z) / 2.0f;

	sph->pos.x = box->vmin.x + rx;
	sph->pos.y = box->vmin.y + ry;
	sph->pos.z = box->vmin.z + rz;

	sph->rad = sqrt(rx * rx + ry * ry + rz * rz);
}
