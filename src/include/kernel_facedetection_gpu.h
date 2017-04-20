/*
 * kernel_facedetection_gpu.h
 *
 *  Created on: Sep 2, 2016
 *      Author: agarcia
 */

#ifndef KERNEL_FACEDETECTION_GPU_H_
#define KERNEL_FACEDETECTION_GPU_H_

#include "program_kernel.h"

bool initCUDA_FD();
void confCUDA_FD(char* file_face, char* file_eyes);
void freeCUDA_FD();
long long facedetection_gpu(unsigned char *data, double scale_factor, int height_parm);

#endif /* KERNEL_FACEDETECTION_GPU_H_ */
