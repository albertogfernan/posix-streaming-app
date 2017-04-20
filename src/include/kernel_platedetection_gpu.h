/*
 * kernel_facedetection_gpu.h
 *
 *  Created on: Sep 2, 2016
 *      Author: agarcia
 */

#ifndef KERNEL_PLATEDETECTION_GPU_H_
#define KERNEL_PLATEDETECTION_GPU_H_

#include "program_kernel.h"

bool initCUDA_PD();
void confCUDA_PD(char* file_face);
void freeCUDA_PD();
long long platedetection_gpu(unsigned char *data, double scale_factor, int height_parm);

#endif /* KERNEL_FACEDETECTION_GPU_H_ */
