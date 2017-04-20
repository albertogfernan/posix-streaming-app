/*
 * program_videosink.cpp
 *
 *  Created on: Aug 9, 2016
 *      Author: agarcia
 */
#include "program_videosink.h"

using namespace cv;

static int process_id = -1;
static int receive_from = -1;
static unsigned char* buffer = NULL;
static char shmem_name[SHMEM_MAX_LENGTH];
static double prevtime = 0, second = 0;
static unsigned int framerate_counter = 0;
static double delay = 0;

void check_configuration_videosink(int id, int num_ops, program_opt_t* options)
{
	process_id = id;
	receive_from = process_id - 1;

	namedWindow("POSIX streaming app", WINDOW_NORMAL);
}

void create_shared_memory_videosink()
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

void display_frame()
{
	Mat img;

	Mat frame(height, width, CV_MAKETYPE(CV_8U, bpp), buffer);

	cvtColor(frame, img, CV_RGBA2BGR);

	imshow("POSIX streaming app", img);
	waitKey(1);
}

void calculate_statistics(int current_frame)
{
	unsigned long long int zerotime, nowtime;
	double ztime;

	zerotime = then(&timestamp_zero);
	nowtime = now();

	ztime = ((double) (nowtime - zerotime)) / 1e6;
	second += ((double) (nowtime - prevtime)) / 1e6;
	prevtime = nowtime;
	framerate_counter++;

	delay = ztime - ((double) current_frame / fps);
}

void print_and_send_statistics(int current_frame)
{
	struct timeval rubbish;

	if (second > 1.0) {
		send_qos_msg(current_frame, FTYPE_DEL_FPS, delay, framerate_counter, &rubbish);

		//printf("videosink: framerate %d fps\n", framerate_counter);
		framerate_counter = 0;
		second = 0;
	}

	//printf("videosink: frame %d delay at %d nominal fps: %lf\n", current_frame, fps, delay);
}

void free_frame()
{
	munmap(buffer, frame_size);

	shm_unlink(shmem_name);
}

void free_child_resources_videosink()
{
	destroyAllWindows();
	waitKey(1);
}


void execute_videosink(int id, int num_ops, program_opt_t* options)
{
	int frames = 0;
	int curr_frame = 0;
	//Check conditions
	check_configuration_videosink(id, num_ops, options);

	do
	{
		//Receive frame
		receive_frame_msg(receive_from, &curr_frame, shmem_name);

		if(frames != curr_frame && curr_frame != -1)
		{
			printf("videosink: frame out of order %d %d\n", curr_frame, frames);
			break;
		}

		//Create shared memory
		create_shared_memory_videosink();

		//If common frame display
		if(curr_frame != -1)
		{
			//display
			display_frame();

			//caculate
			calculate_statistics(curr_frame);

			//print
			print_and_send_statistics(curr_frame);
		}

		free_frame();

		frames++;

	}while(curr_frame != -1);

	send_qos_msg_end();

	free_child_resources_videosink();
}
