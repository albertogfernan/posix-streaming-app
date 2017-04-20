/*
 * kernel_facedetection.cpp
 *
 *  Created on: Aug 31, 2016
 *      Author: agarcia
 */
#include "kernel_platedetection.h"
#include "kernel_platedetection_gpu.h"

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

static int device_type = DEVICE_UCONF;
static char* file_plate = NULL;
static double factor = 1.15;
CascadeClassifier* plate_cascade = NULL;
long long plates_detected = 0;
static bool cuda_support = true;


void init_platedetection(int id)
{
	process_id = id;
	receive_from = process_id - 1;

	plate_cascade = new CascadeClassifier();

	cuda_support = initCUDA_PD();

	initPapi();
}


void configure_opt_platedetection(int num_ops, program_opt_t* options)
{
	int i;

	for (i = 0; i < num_ops; i++) {
		if (strncmp(options[i].key, "file_plate", OPT_MAX_LENGTH) == 0) {
			file_plate = options[i].value;
		} else if (strncmp(options[i].key, "device", OPT_MAX_LENGTH) == 0) {
			device_type = atoi(options[i].value);
		} else if (strncmp(options[i].key, "scale", OPT_MAX_LENGTH) == 0) {
			factor = atof(options[i].value);
		}
	}

	if (file_plate == NULL) {
		printf("kernel %s: file_plate not specified\n", kernel_name);
		exit (EXIT_FAILURE);
	}

	if(plate_cascade->empty() && !plate_cascade->load(file_plate)){
		printf("kernel %s: error loading %s file\n", kernel_name, file_plate);
		exit(EXIT_FAILURE);
	}

	//Add defaults if necessary in else part
	if (device_type == DEVICE_UCONF)
		device_type = DEVICE_CPUMT;

	if(cuda_support)
		confCUDA_PD(file_plate);
	else if(device_type == DEVICE_GPGPU)
		device_type = DEVICE_NONE;
}


long long platedetection(unsigned char *data, double scale_factor, int height_parm)
{
	std::vector<Rect> plates;
	unsigned int row_stride = width * bpp;
	long long plates_found = 0;

	Mat frame(height_parm, width, CV_MAKETYPE(CV_8U, bpp), buffer, row_stride);
	Mat frame_gray;

	cvtColor(frame, frame_gray, CV_RGB2GRAY);
	//equalizeHist(frame_gray, frame_gray);

	plate_cascade->detectMultiScale(frame_gray, plates, scale_factor, 3, 0|CASCADE_SCALE_IMAGE);
	for (size_t i = 0; i < plates.size(); i++) {
	//	Point center(plates[i].x + plates[i].width * 0.5, plates[i].y + plates[i].height * 0.5);
	//	ellipse(frame, center, Size(plates[i].width * 0.5, plates[i].height * 0.5), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);
		rectangle(frame, plates[i], Scalar(255, 0, 255), 4, 8, 0);
		
	}

	plates_found = plates.size();

	return plates_found;
}


void process_frame_platedetection() {
	plates_detected = 0;

	switch (device_type) {
	case DEVICE_UCONF:

		printf("kernel %s: device not configured\n", kernel_name);

		break;
	case DEVICE_CPUST:
		omp_set_num_threads(1);

		startMeasures();

		plates_detected = platedetection(buffer, factor, height);

		stopMeasures();
		break;
	case DEVICE_CPUMT:
		omp_set_num_threads(std::thread::hardware_concurrency());

		startMeasures();

		plates_detected = platedetection(buffer, factor, height);

		stopMeasures();
		break;
	case DEVICE_GPGPU:
		startCUDAMeasures();

		plates_detected = platedetection_gpu(buffer, factor, height);

		stopCUDAMeasures();
		break;
	case DEVICE_NONE:
		break;
	case DEVICE_MIXED:
		omp_set_num_threads(std::thread::hardware_concurrency());

		startMeasures();

		plates_detected = platedetection(buffer, factor, height / 2);

		stopMeasures();

		startCUDAMeasures();

		plates_detected = platedetection_gpu(&buffer[width * bpp * height / 2], factor, height / 2);

		stopCUDAMeasures();

		break;
	default:
		printf("kernel %s: device unknown\n", kernel_name);
		break;
	}
}

void send_metrics_platedetection(int frame_num) {
	struct timeval rubbish;
	if (frame_num == -1)
		send_qos_msg_end();
	else {
		switch (device_type) {
		case DEVICE_UCONF:
			printf("kernel %s: device not configured\n", kernel_name);
			break;
		case DEVICE_CPUST:
		case DEVICE_CPUMT:
			send_qos_msg(frame_num, FTYPE_CPU_PLA, ((double) (time_proc)) / 1e6, plates_detected, &rubbish);
			break;
		case DEVICE_NONE:
			break;
		case DEVICE_GPGPU:
			send_qos_msg(frame_num, FTYPE_CPU_PLA, ((double) (time_gpgpu)) / 1e6, plates_detected, &rubbish);
			break;
		case DEVICE_MIXED:
			send_qos_msg(frame_num, FTYPE_CPU_PLA, ((double) (time_proc + time_gpgpu)) / 1e6, plates_detected, &rubbish);
			break;
		default:
			printf("kernel %s: device unknown\n", kernel_name);
			break;
		}
	}
}

void free_child_resources_platedetection()
{
	delete plate_cascade;

	if(cuda_support)
		freeCUDA_PD();

	freePapi();

	printMeasures(kernel_name);

	printCUDAMeasures(kernel_name);
}

