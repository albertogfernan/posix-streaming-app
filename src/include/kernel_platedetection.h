/*
 * kernel_facedetection.h
 *
 *  Created on: Aug 31, 2016
 *      Author: agarcia
 */

#ifndef KERNEL_PLATEDETECTION_H_
#define KERNEL_PLATEDETECTION_H_

#include "utilities.h"
#include "ipc_communications.h"
#include "program_kernel.h"


void init_platedetection(int id);
void configure_opt_platedetection(int num_ops, program_opt_t* options);
void process_frame_platedetection();
void send_metrics_platedetection(int frame_num);
void free_child_resources_platedetection();

#endif /* KERNEL_FACEDETECTION_H_ */
