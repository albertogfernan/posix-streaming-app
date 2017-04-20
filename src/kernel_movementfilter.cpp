/*
 * kernel_movementfilter.cpp
 *
 *  Created on: Aug 24, 2016
 *      Author: agarcia
 */
#include "kernel_movementfilter.h"
#include "kernel_movementfilter_gpu.h"

static int device_type = DEVICE_UCONF;
unsigned char* buffer_ref = NULL;
long long pixel_diff = 0;

void init_movementfilter(int id)
{
	process_id = id;
	receive_from = process_id - 1;

	buffer_ref = (unsigned char*)malloc(MAX_BUFFER);
	memset(buffer_ref, '\0', MAX_BUFFER);

	initCUDA_MF();

	initPapi();
}


void configure_opt_movementfilter(int num_ops, program_opt_t* options)
{
	int i;

	for (i = 0; i < num_ops; i++) {
		if (strncmp(options[i].key, "device", OPT_MAX_LENGTH) == 0) {
			device_type = atoi(options[i].value);
			printf("kernel %s: setting configuration %s to %s\n", kernel_name, options[i].key, options[i].value);
		}
	}

	//Add defaults if necessary in else part
	if(device_type == DEVICE_UCONF)
		device_type = DEVICE_CPUST;
}

void convert2greyscale_cpu_kernel(unsigned char *dataIn, unsigned char *dataOut, unsigned char *dataRef, int height_parm) {
	int i, j;
	unsigned int r, g, b;
	unsigned int luma;
	unsigned char *dataIn_th, *dataOut_th, *dataRef_th;
	unsigned int row_stride = width * bpp;

#pragma omp parallel private(i, j, r, g, b, luma, dataIn_th, dataOut_th, dataRef_th) shared(dataIn, dataOut, dataRef, width, height_parm, bpp, row_stride)
	{
#pragma omp for schedule(static)
		for (i = 0; i < height_parm; i++) {
			dataIn_th = dataIn + (row_stride * i);
			dataOut_th = dataOut + (row_stride * i);
			dataRef_th = dataRef + (row_stride * i);
			for (j = 0; j < width; j++) {
				r = dataIn_th[0];
				g = dataIn_th[1];
				b = dataIn_th[2];

				/* BT. 709 coefficients in B8 fixed point */
				/* 0.2126 R + 0.7152 G + 0.0722 B */
				/*luma = ((r << 8) * 54) + ((g << 8) * 183) + ((b << 8) * 19);
				 luma >>= 16;*//* get integer part */

				luma = (r + g + b) / 3;

				dataOut_th[0] = abs(luma - dataRef_th[0]);
				dataOut_th[1] = abs(luma - dataRef_th[1]);
				dataOut_th[2] = abs(luma - dataRef_th[2]);

				dataRef_th[0] = luma;
				dataRef_th[1] = luma;
				dataRef_th[2] = luma;

				dataIn_th += bpp;
				dataOut_th += bpp;
				dataRef_th += bpp;
			}
		}
	}

	return;
}


long long reduce_cpu_kernel(unsigned char *dataOut, int height_parm) {
	int i, j;
	unsigned char *dataOut_th;
	long long diff_pix = 0;
	unsigned int row_stride = width * bpp;

#pragma omp parallel private(i, j, dataOut_th) shared(dataOut, width, height_parm, bpp, row_stride) reduction(+:diff_pix)
	{
#pragma omp for schedule(static)
		for (i = 0; i < height_parm; i++) {
			dataOut_th = dataOut + (row_stride * i);
			for (j = 0; j < width; j++) {
				diff_pix += dataOut_th[0];
				dataOut_th += bpp;
			}
		}
	}

	return diff_pix;
}


void process_frame_movementfilter()
{
	switch(device_type){
	case DEVICE_UCONF:

		printf("kernel %s: device not configured\n", kernel_name);

		break;
	case DEVICE_CPUST:
		omp_set_num_threads(1);

		startMeasures();

		convert2greyscale_cpu_kernel(buffer, buffer, buffer_ref, height);

		pixel_diff = reduce_cpu_kernel(buffer, height);

		stopMeasures();

		break;
	case DEVICE_CPUMT:
		omp_set_num_threads(std::thread::hardware_concurrency());

		startMeasures();

		convert2greyscale_cpu_kernel(buffer, buffer, buffer_ref, height);

		pixel_diff = reduce_cpu_kernel(buffer, height);

		stopMeasures();

		break;
	case DEVICE_GPGPU:

		startCUDAMeasures();

		pixel_diff = convert2greyscale_cuda_kernel(buffer, buffer_ref, height);

		stopCUDAMeasures();

		break;
	case DEVICE_NONE:
		pixel_diff = 0;
		break;
	case DEVICE_MIXED:
		omp_set_num_threads(std::thread::hardware_concurrency());

		startMeasures();

		convert2greyscale_cpu_kernel(buffer, buffer, buffer_ref, height / 2);

		pixel_diff = reduce_cpu_kernel(buffer, height / 2);

		stopMeasures();
		
		startCUDAMeasures();

		pixel_diff += convert2greyscale_cuda_kernel(&buffer[width * bpp * height / 2], &buffer_ref[width * bpp * height / 2], height / 2);

		stopCUDAMeasures();

		break;
	default:
		printf("kernel %s: device unknown\n", kernel_name);
		break;
	}
}

void send_metrics_movementfilter(int frame_num) {
	struct timeval rubbish;
	if (frame_num == -1)
		send_qos_msg_end();
	else {
		switch (device_type) {
		case DEVICE_UCONF:
			printf("kernel %s: device not configured\n", kernel_name);
			break;
		case DEVICE_CPUST:
			send_qos_msg(frame_num, FTYPE_CPU_PIX, ((double) (time_proc)) / 1e6, pixel_diff, &rubbish);
			break;
		case DEVICE_CPUMT:
			send_qos_msg(frame_num, FTYPE_CPU_PIX, ((double) (time_proc)) / 1e6, pixel_diff, &rubbish);
			break;
		case DEVICE_GPGPU:
			send_qos_msg(frame_num, FTYPE_CPU_PIX, ((double) (time_gpgpu)) / 1e6, pixel_diff, &rubbish);
			break;
		case DEVICE_MIXED:
			send_qos_msg(frame_num, FTYPE_CPU_PIX, ((double) (time_proc + time_gpgpu)) / 1e6, pixel_diff, &rubbish);
			break;
		case DEVICE_NONE:
			break;
		default:
			printf("kernel %s: device unknown\n", kernel_name);
			break;
		}
	}
}

void free_child_resources_movementfilter()
{
	free(buffer_ref);

	freeCUDA_MF();

	freePapi();

	printMeasures(kernel_name);

	printCUDAMeasures(kernel_name);
}
