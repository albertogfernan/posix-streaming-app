/*
 * cuda_measure_management.h
 *
 *  Created on: Aug 30, 2016
 *      Author: agarcia
 */

#ifndef CUDA_MEASURE_MANAGEMENT_H_
#define CUDA_MEASURE_MANAGEMENT_H_

#include <stdio.h>

extern unsigned long long int time_gpgpu;

void initCUDAMeasures();
void startCUDAMeasures();
void stopCUDAMeasures();
void printCUDAMeasures(char* kernel_name);
void freeCUDAMeasures();

#endif /* CUDA_MEASURE_MANAGEMENT_H_ */
