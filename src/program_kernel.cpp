/*
 * program_kernel.cpp
 *
 *  Created on: Aug 23, 2016
 *      Author: agarcia
 */
#include "program_kernel.h"

//Add here compute time, fops, and watts

//Add pointers here
void (*init_kernel)(int id) = NULL;
void (*configure_opt_kernel)(int num_ops, program_opt_t* options) = NULL;
void (*process_frame_kernel)() = NULL;
void (*send_metrics_kernel)(int frame_num) = NULL;
void (*free_child_resources_kernel)() = NULL;

int process_id = -1;
int receive_from = -1;
unsigned char* buffer = NULL;
char kernel_name[KER_MAX_LENGTH];
char shmem_name[SHMEM_MAX_LENGTH];

void create_shared_memory_kernel()
{
	int fd = shm_open(shmem_name, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
	if(fd == -1)
	{
		printf("filesrc: error creating shared memory buffer\n");
		exit(EXIT_FAILURE);
	}

	if (ftruncate(fd, frame_size) == -1) {
		printf("filesrc: error allocating shared memory buffer\n");
		exit(EXIT_FAILURE);
	}

	buffer = (unsigned char*)mmap(NULL, frame_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (buffer == MAP_FAILED) {
		printf("filesrc: error mapping shared memory buffer\n");
		exit(EXIT_FAILURE);
	}

	close(fd);
}

void coordinator_exchange(int curr_frame)
{
	int num_ops;
	program_opt_t dyn_options;

	if(curr_frame % COORD_EXCHANGE == 0 || curr_frame == -1)
	{
		send_metrics_kernel(curr_frame);

		receive_conf_msg(process_id, &num_ops, &dyn_options);

		if(num_ops > 0)
			configure_opt_kernel(num_ops, &dyn_options);
	}
}

void send_frame(int frame_num)
{
	if(msync(buffer, frame_size, MS_SYNC) == -1)
	{
		printf("filesrc: error synchronising the buffer.\n");
	}

	send_frame_msg(process_id, frame_num, shmem_name);

	munmap(buffer, frame_size);
}

void execute_kernel(int id, int num_ops, program_opt_t* init_options)
{
	int frames = 0;
	int curr_frame = 0;

	//Check conditions
	init_kernel(id);

	configure_opt_kernel(num_ops, init_options);

	do {
		//Receive frame
		receive_frame_msg(receive_from, &curr_frame, shmem_name);

		//Create shared memory
		create_shared_memory_kernel();

		//If common frame process
		if (curr_frame != -1) {
			process_frame_kernel();
		}

		coordinator_exchange(curr_frame);

		send_frame(curr_frame);

		frames++;

	} while (curr_frame != -1);

	free_child_resources_kernel();
}

void configure_kernel(const char* kernel)
{
	strncpy(kernel_name, kernel, KER_MAX_LENGTH);

	if(strcmp(kernel_name, "movementfilter") == 0)
	{
		init_kernel = &init_movementfilter;
		configure_opt_kernel = &configure_opt_movementfilter;
		process_frame_kernel = &process_frame_movementfilter;
		send_metrics_kernel = &send_metrics_movementfilter;
		free_child_resources_kernel = &free_child_resources_movementfilter;
	}
	else if(strcmp(kernel_name, "facedetection") == 0)
	{
		init_kernel = &init_facedetection;
		configure_opt_kernel = &configure_opt_facedetection;
		process_frame_kernel = &process_frame_facedetection;
		send_metrics_kernel = &send_metrics_facedetection;
		free_child_resources_kernel = &free_child_resources_facedetection;
	}
	else if(strcmp(kernel_name, "platedetection") == 0)
	{
		init_kernel = &init_platedetection;
		configure_opt_kernel = &configure_opt_platedetection;
		process_frame_kernel = &process_frame_platedetection;
		send_metrics_kernel = &send_metrics_platedetection;
		free_child_resources_kernel = &free_child_resources_platedetection;
	}
}
