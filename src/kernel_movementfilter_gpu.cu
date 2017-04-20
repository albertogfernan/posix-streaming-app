#include "kernel_movementfilter_gpu.h"

#include <cuda_runtime_api.h>
#include <cuda.h>

#define BLOCK_SIZE_1D 256

unsigned char *frame_data_device, *ref_data_device;
int * reduction;
int * devReduction;

void initCUDA_MF()
{
	int size;

	cudaMalloc((void**) &ref_data_device, MAX_BUFFER);
	cudaMemset(ref_data_device, 0, MAX_BUFFER);
	cudaMalloc((void**) &frame_data_device, MAX_BUFFER);
	cudaMemset(frame_data_device, 0, MAX_BUFFER);

	size = (frame_size / 4) / BLOCK_SIZE_1D;
	reduction = (int*) malloc(size);
	cudaMalloc(&devReduction, size);
	cudaMemset(devReduction, 0, size);
	cudaHostRegister(reduction, size, cudaHostRegisterDefault);

	initCUDAMeasures();
}


void freeCUDA_MF()
{
	cudaFree(ref_data_device);
	cudaFree(frame_data_device);

	cudaFree(devReduction);
	cudaHostUnregister(reduction);
	free(reduction);

	freeCUDAMeasures();

	cudaDeviceReset();
}


template<unsigned int blockSize>
__device__ void warpReduce(volatile int *sdata, unsigned int tid) {
	if (blockSize >= 64)
		sdata[tid] += sdata[tid + 32];
	if (blockSize >= 32)
		sdata[tid] += sdata[tid + 16];
	if (blockSize >= 16)
		sdata[tid] += sdata[tid + 8];
	if (blockSize >= 8)
		sdata[tid] += sdata[tid + 4];
	if (blockSize >= 4)
		sdata[tid] += sdata[tid + 2];
	if (blockSize >= 2)
		sdata[tid] += sdata[tid + 1];
}

template<unsigned int blockSize>
__global__ void reduce6_RGBx(unsigned char *g_idata, int *g_odata, int n) {
	extern __shared__ int sdata[];
	unsigned int tid = threadIdx.x;
	unsigned int i = blockIdx.x * (blockSize * 2) + tid;
	unsigned int gridSize = blockSize * 2 * gridDim.x;
	sdata[tid] = 0;

	while (i < n) {
		sdata[tid] += ((uchar4*)g_idata)[i].x + ((uchar4*)g_idata)[i + blockSize].x;
		i += gridSize;
	}

	__syncthreads();

	if (blockSize >= 512) {
		if (tid < 256) {
			sdata[tid] += sdata[tid + 256];
		}
		__syncthreads();
	}
	if (blockSize >= 256) {
		if (tid < 128) {
			sdata[tid] += sdata[tid + 128];
		}
		__syncthreads();
	}
	if (blockSize >= 128) {
		if (tid < 64) {
			sdata[tid] += sdata[tid + 64];
		}
		__syncthreads();
	}
	if (tid < 32)
		warpReduce<blockSize>(sdata, tid);

	if (tid == 0)
		g_odata[blockIdx.x] = sdata[0];
}


template<unsigned int blockSize>
__global__ void reduce6_RGB(unsigned char *g_idata, int *g_odata, int n) {
	extern __shared__ int sdata[];
	unsigned int tid = threadIdx.x;
	unsigned int i = blockIdx.x * (blockSize * 2) + tid;
	unsigned int gridSize = blockSize * 2 * gridDim.x;
	sdata[tid] = 0;

	while (i < n) {
		sdata[tid] += ((uchar3*)g_idata)[i].x + ((uchar3*)g_idata)[i + blockSize].x;
		i += gridSize;
	}

	__syncthreads();

	if (blockSize >= 512) {
		if (tid < 256) {
			sdata[tid] += sdata[tid + 256];
		}
		__syncthreads();
	}
	if (blockSize >= 256) {
		if (tid < 128) {
			sdata[tid] += sdata[tid + 128];
		}
		__syncthreads();
	}
	if (blockSize >= 128) {
		if (tid < 64) {
			sdata[tid] += sdata[tid + 64];
		}
		__syncthreads();
	}
	if (tid < 32)
		warpReduce<blockSize>(sdata, tid);

	if (tid == 0)
		g_odata[blockIdx.x] = sdata[0];
}


template<unsigned int blockSize>
__global__ void reduce6_int(int *g_idata, int *g_odata, int n) {
	extern __shared__ int sdata[];
	unsigned int tid = threadIdx.x;
	unsigned int i = blockIdx.x * (blockSize * 2) + tid;
	unsigned int gridSize = blockSize * 2 * gridDim.x;
	sdata[tid] = 0;

	while (i < n) {
		sdata[tid] += g_idata[i] + g_idata[i + blockSize];
		i += gridSize;
	}

	__syncthreads();

	if (blockSize >= 512) {
		if (tid < 256) {
			sdata[tid] += sdata[tid + 256];
		}
		__syncthreads();
	}
	if (blockSize >= 256) {
		if (tid < 128) {
			sdata[tid] += sdata[tid + 128];
		}
		__syncthreads();
	}
	if (blockSize >= 128) {
		if (tid < 64) {
			sdata[tid] += sdata[tid + 64];
		}
		__syncthreads();
	}
	if (tid < 32)
		warpReduce<blockSize>(sdata, tid);

	if (tid == 0)
		g_odata[blockIdx.x] = sdata[0];
}


__global__ void convert2greyscale_cuda_kernel_RGBx(int width, int height, int pixel_stride, int row_stride, unsigned char *dataIn, unsigned char *dataOut, unsigned char *dataRef)
{
	int absolute_position;
	int luma;

	absolute_position = (blockIdx.x * blockDim.x) + threadIdx.x;

	uchar4 a=((uchar4*)dataIn)[absolute_position];
	uchar4 aref=((uchar4*)dataRef)[absolute_position];

	luma = (a.x + a.y + a.z) / 3;

	a.x = abs(luma - aref.x);
	a.y = abs(luma - aref.y);
	a.z = abs(luma - aref.z);

	aref.x = luma;
	aref.y = luma;
	aref.z = luma;

	((uchar4*)dataOut)[absolute_position] = a;
	((uchar4*)dataRef)[absolute_position] = aref;
}


__global__ void convert2greyscale_cuda_kernel_RGB(int width, int height, int pixel_stride, int row_stride, unsigned char *dataIn, unsigned char *dataOut, unsigned char *dataRef)
{
	int absolute_position;
	int luma;

	absolute_position = (blockIdx.x * blockDim.x) + threadIdx.x;

	uchar3 a=((uchar3*)dataIn)[absolute_position];
	uchar3 aref=((uchar3*)dataRef)[absolute_position];

	luma = (a.x + a.y + a.z) / 3;

	a.x = abs(luma - aref.x);
	a.y = abs(luma - aref.y);
	a.z = abs(luma - aref.z);

	aref.x = luma;
	aref.y = luma;
	aref.z = luma;

	((uchar3*)dataOut)[absolute_position] = a;
	((uchar3*)dataRef)[absolute_position] = aref;
}



long long convert2greyscale_cuda_kernel(unsigned char *data, unsigned char *dataRef, int height_parm)
{
	long long pixel_diff_gpgpu = 0;
	unsigned int row_stride = width * bpp;
	int true_size = row_stride * height_parm;
	cudaHostRegister(data, true_size, cudaHostRegisterDefault);
	cudaMemcpy(frame_data_device, data, true_size, cudaMemcpyHostToDevice);

	const dim3 blockSize(BLOCK_SIZE_1D, 1, 1);
	const dim3 gridSize((true_size / bpp) / BLOCK_SIZE_1D, 1, 1);
	const dim3 gridSize2(gridSize.x / BLOCK_SIZE_1D, 1, 1);
	int smemSize = blockSize.x * bpp;

	if (bpp == 3) {
		convert2greyscale_cuda_kernel_RGB<<<gridSize, blockSize>>>(width, height_parm, bpp, row_stride, frame_data_device, frame_data_device, ref_data_device);
		reduce6_RGB<BLOCK_SIZE_1D> <<<gridSize, blockSize, smemSize>>>(frame_data_device, devReduction, true_size / bpp);
		reduce6_int<BLOCK_SIZE_1D> <<<gridSize2, blockSize, smemSize>>>(devReduction, devReduction, gridSize.x);
	} else if (bpp == 4) {
		convert2greyscale_cuda_kernel_RGBx<<<gridSize, blockSize>>>(width, height_parm, bpp, row_stride, frame_data_device, frame_data_device, ref_data_device);
		reduce6_RGBx<BLOCK_SIZE_1D> <<<gridSize, blockSize, smemSize>>>(frame_data_device, devReduction, true_size / bpp);
		reduce6_int<BLOCK_SIZE_1D> <<<gridSize2, blockSize, smemSize>>>(devReduction, devReduction, gridSize.x);
	}

	cudaMemcpy(data, frame_data_device, true_size, cudaMemcpyDeviceToHost);
	cudaMemcpy(reduction, devReduction, gridSize2.x * sizeof(int), cudaMemcpyDeviceToHost);

	pixel_diff_gpgpu = 0;
	for (unsigned int i = 0; i < gridSize2.x; i++)
		pixel_diff_gpgpu += reduction[i];

	cudaHostUnregister(data);

	return pixel_diff_gpgpu;
}
