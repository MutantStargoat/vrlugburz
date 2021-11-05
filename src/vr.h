#ifndef VR_H_
#define VR_H_

#include <goatvr.h>

#ifdef BUILD_VR
#define vr_active()	goatvr_invr()
#else
#define vr_active()	0
#endif

int should_swap;
int vrbn_a, vrbn_b, vrbn_x, vrbn_y;

int init_vr(void);
void shutdown_vr(void);
void update_vr_input(void);

#endif	/* VR_H_ */
