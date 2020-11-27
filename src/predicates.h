#ifndef _PREDICATES_H
#define _PREDICATES_H


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

void exactinit();
float incircle(float *pa, float *pb, float *pc, float *pd);
float orient2d(float *pa, float *pb, float *pc);

#endif
