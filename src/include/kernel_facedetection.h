/*
 * kernel_facedetection.h
 *
 *  Created on: Aug 31, 2016
 *      Author: agarcia
 */

#ifndef KERNEL_FACEDETECTION_H_
#define KERNEL_FACEDETECTION_H_

#include "utilities.h"
#include "ipc_communications.h"
#include "program_kernel.h"


void init_facedetection(int id);
void configure_opt_facedetection(int num_ops, program_opt_t* options);
void process_frame_facedetection();
void send_metrics_facedetection(int frame_num);
void free_child_resources_facedetection();

#endif /* KERNEL_FACEDETECTION_H_ */
