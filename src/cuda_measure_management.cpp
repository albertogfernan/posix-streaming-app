#include "cuda_measure_management.h"

#include <cuda_runtime_api.h>
#include <cuda_runtime.h>
#include <cuda.h>

cudaEvent_t start;
cudaEvent_t stop;

unsigned long long int time_gpgpu = 0;
unsigned long long int acctime_gpgpu = 0;

void initCUDAMeasures()
{
	cudaEventCreate(&start);
	cudaEventCreate(&stop);
}

void startCUDAMeasures()
{
	cudaEventRecord(start);
}

void stopCUDAMeasures()
{
	float milliseconds = 0;

	cudaEventRecord(stop);
	cudaEventSynchronize(stop);
	cudaEventElapsedTime(&milliseconds, start, stop);

	time_gpgpu = (unsigned long long int)(milliseconds * 1000.0f);
	acctime_gpgpu += time_gpgpu;
}

void printCUDAMeasures(char* kernel_name)
{
	printf("kernel %s: accumulated gpgpu time %lf secs.\n", kernel_name, ((double) (acctime_gpgpu)) / 1e6);
}


void freeCUDAMeasures()
{
	cudaEventDestroy(start);
	cudaEventDestroy(stop);
}
