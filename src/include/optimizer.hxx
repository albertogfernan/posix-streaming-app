#ifndef OPTIMIZER_H_INCLUDED
#define OPTIMIZER_H_INCLUDED

#include "lp_lib.h"

void pixel_to_qos(long pixel_diff, long thrs, int *QoS_int);

REAL *optimizer(int QoS);

#endif
