/*
 * kernel_movementfilter_gpu.h
 *
 *  Created on: Aug 30, 2016
 *      Author: agarcia
 */

#ifndef KERNEL_MOVEMENTFILTER_GPU_H_
#define KERNEL_MOVEMENTFILTER_GPU_H_

#include "program_kernel.h"

#define MAX_BUFFER 4096*4096*4

void initCUDA_MF();
void freeCUDA_MF();

long long convert2greyscale_cuda_kernel(unsigned char *data, unsigned char *dataRef, int height_parm);

#endif /* KERNEL_MOVEMENTFILTER_GPU_H_ */
