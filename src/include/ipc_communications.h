/*
 * ipc_communications.h
 *
 *  Created on: Aug 7, 2016
 *      Author: garci
 */

#ifndef IPC_COMMUNICATIONS_H_
#define IPC_COMMUNICATIONS_H_

#include <stdlib.h>
#include <stdio.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include "utilities.h"

#define QUEUE_NAMES 		"posix-app-queue"
#define QNAME_MAX_LENGTH	100
#define SHMEM_MAX_LENGTH	255

typedef struct{
	int frame_number;
	char shmem_name[SHMEM_MAX_LENGTH];
} app_msg_frame_t;


#define FTYPE_CPU_PIX		1
#define FTYPE_CPU_FAC		2
#define FTYPE_DEL_FPS		3
#define FTYPE_CPU_PLA		4

typedef struct{
	int frame_number;
	pid_t pid_sender;
	int field_type;
	double field_content_1;
	unsigned long long field_content_2;
	struct timeval timestamp;
} app_msg_qos_t;

typedef program_opt_t app_msg_conf_t;

void init_queues(int num_proc);

void init_conf_queues(int num_procs);

void init_frame_queues(int num_procs);

void init_coord_queue();

void send_frame_msg(int receiver, int frame_number, char* shared_mem_name);

void receive_frame_msg(int sender, int* frame_number, char* shared_mem_name);

void send_qos_msg(int frame_number, int field_type, double field_content_1, unsigned long long field_content_2, struct timeval* timestamp);

void send_qos_msg_end();

void receive_qos_msg(int* frame_number, pid_t* pid_sender, int* field_type, double* field_content_1, unsigned long long* field_content_2, struct timeval* timestamp);

void send_conf_msg(int queue_no, const char* key, const char* value);

void receive_conf_msg(int queue_no, int* num_conf, program_opt_t* options);

void close_queues();

void unlink_queues();

#endif /* SRC_INCLUDE_IPC_COMMUNICATIONS_H_ */
