/*
 * program_kernel.h
 *
 *  Created on: Aug 23, 2016
 *      Author: agarcia
 */

#ifndef PROGRAM_KERNEL_H_
#define PROGRAM_KERNEL_H_

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "utilities.h"
#include "ipc_communications.h"
#include "debug_header.h"
#include "global_params.h"
#include "kernel_movementfilter.h"
#include "kernel_facedetection.h"
#include "kernel_platedetection.h"

#define COORD_EXCHANGE		10
#define KER_MAX_LENGTH		255

#define DEVICE_UCONF		-1
#define DEVICE_CPUST		0
#define DEVICE_CPUMT		1
#define DEVICE_GPGPU		2
#define DEVICE_NONE			3
#define DEVICE_MIXED		4


static const char DEVICE_UCONF_S[] = "-1";
static const char DEVICE_CPUST_S[] = "0";
static const char DEVICE_CPUMT_S[] = "1";
static const char DEVICE_GPGPU_S[] = "2";
static const char DEVICE_NONE_S[] = "3";

void configure_kernel(const char* kernel);
void execute_kernel(int id, int num_ops, program_opt_t* options);

extern char kernel_name[KER_MAX_LENGTH];
extern int process_id;
extern int receive_from;
extern unsigned char* buffer;
extern size_t size;
extern int width;
extern int height;
extern int bpp;

#endif /* PROGRAM_KERNEL_H_ */
