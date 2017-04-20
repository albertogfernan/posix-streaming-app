/*
 * program_filesrc.cpp
 *
 *  Created on: Aug 7, 2016
 *      Author: garci
 */
#include "program_filesrc.h"

static int process_id = -1;
static unsigned char* buffer = NULL;
static FILE * fd_src = NULL;
static char shmem_name[SHMEM_MAX_LENGTH];

void check_configuration_filesrc(int id, int num_ops, program_opt_t* options)
{
	int i;
	char* file_name = NULL;

	process_id = id;

	if(process_id != 0)
	{
		printf("filesrc: filesrc must be the first program\n");
		exit(EXIT_FAILURE);
	}

	for(i=0; i<num_ops; i++)
	{
		if(strncmp(options[i].key, "location", OPT_MAX_LENGTH) == 0)
		{
			file_name = options[i].value;
		}
	}

	if(file_name == NULL)
	{
		printf("filesrc: file_name not specified\n");
		exit(EXIT_FAILURE);
	}

	fd_src = fopen(file_name, "rb");
	if (fd_src == NULL) {
		printf("filesrc: error opening the file\n");
		exit(EXIT_FAILURE);
	}
}

void create_shared_memory_filesrc(int frame_num)
{
	int random;
	int fd;

	random = rand();
	snprintf(shmem_name, SHMEM_MAX_LENGTH, "/shared-mem-%d-%d", frame_num, random);

	fd = shm_open(shmem_name, O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR);
	if(fd == -1)
	{
		printf("filesrc: error creating shared memory buffer\n");
		exit(EXIT_FAILURE);
	}

	if (ftruncate(fd, frame_size) != 0) {
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


int read_frame()
{
	unsigned int rb = 0, total = 0;
	unsigned int total_bytes = frame_size;

	do
	{
		rb = fread(&buffer[total], 1, total_bytes - total, fd_src);
		total += rb;
	}
	while(rb != 0 && total != total_bytes);

	if (total != total_bytes && total != 0) {
		printf("filesrc: error reading %d bytes from the file. Returned %d \n", total_bytes, rb);
		return -1;
	}
	else if(total == 0)
	{
		printf("filesrc: EOF.\n");
	}

	return total;
}

void send_new_frame(int frame_num)
{
	if(msync(buffer, frame_size, MS_SYNC) == -1)
	{
		printf("filesrc: error synchronising the buffer.\n");
	}

	send_frame_msg(process_id, frame_num, shmem_name);

	munmap(buffer, frame_size);
}

void free_child_resources_filesrc()
{
	fclose(fd_src);
}

void execute_filesrc(int id, int num_ops, program_opt_t* options)
{
	size_t bytes_read = 0;
	int frames = 0;
	//Check conditions
	check_configuration_filesrc(id, num_ops, options);

	do
	{
		//Create shared memory
		create_shared_memory_filesrc(frames);

		//Read from file
		bytes_read = read_frame();

		//Send to
		if(bytes_read == frame_size)
		{
			//Send frame
			send_new_frame(frames);
		}
		else if (bytes_read == 0)
		{
			//Send closure
			send_new_frame(-1);
			send_qos_msg_end();
			break;
		}
		else
		{
			break;
		}

		frames++;

	}while(bytes_read > 0);

	free_child_resources_filesrc();
}
