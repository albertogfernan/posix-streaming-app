/*
 * kernel_movementfilter.h
 *
 *  Created on: Aug 23, 2016
 *      Author: agarcia
 */

#ifndef KERNEL_MOVEMENTFILTER_H_
#define KERNEL_MOVEMENTFILTER_H_

#include <thread>
#include <omp.h>

#include "utilities.h"
#include "ipc_communications.h"
#include "program_kernel.h"
#include "papi_management.h"
#include "cuda_measure_management.h"


void init_movementfilter(int id);
void configure_opt_movementfilter(int num_ops, program_opt_t* options);
void process_frame_movementfilter();
void send_metrics_movementfilter(int frame_num);
void free_child_resources_movementfilter();


#endif /* KERNEL_MOVEMENTFILTER_H_ */
