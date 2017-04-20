/*
 * kernel_facedetection.cpp
 *
 *  Created on: Aug 31, 2016
 *      Author: agarcia
 */
#include "kernel_facedetection.h"
#include "kernel_facedetection_gpu.h"

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace cv;

static int device_type = DEVICE_UCONF;
static char* file_face = NULL;
static char* file_eyes = NULL;
static double factor = 1.15;
CascadeClassifier* face_cascade = NULL;
CascadeClassifier* eyes_cascade = NULL;
long long faces_detected = 0;
static bool cuda_support = true;


void init_facedetection(int id)
{
	process_id = id;
	receive_from = process_id - 1;

	face_cascade = new CascadeClassifier();
	eyes_cascade = new CascadeClassifier();

	cuda_support = initCUDA_FD();

	initPapi();
}


void configure_opt_facedetection(int num_ops, program_opt_t* options)
{
	int i;

	for (i = 0; i < num_ops; i++) {
		if (strncmp(options[i].key, "file_face", OPT_MAX_LENGTH) == 0) {
			file_face = options[i].value;
		} else if (strncmp(options[i].key, "file_eyes", OPT_MAX_LENGTH) == 0) {
			file_eyes = options[i].value;
		} else if (strncmp(options[i].key, "device", OPT_MAX_LENGTH) == 0) {
			device_type = atoi(options[i].value);
		} else if (strncmp(options[i].key, "scale", OPT_MAX_LENGTH) == 0) {
			factor = atof(options[i].value);
		}
	}

	if (file_face == NULL) {
		printf("kernel %s: file_face not specified\n", kernel_name);
		exit (EXIT_FAILURE);
	}

	if (file_eyes == NULL) {
		printf("kernel %s: file_eyes not specified\n", kernel_name);
		exit (EXIT_FAILURE);
	}

	if(face_cascade->empty() && !face_cascade->load(file_face)){
		printf("kernel %s: error loading %s file\n", kernel_name, file_face);
		exit(EXIT_FAILURE);
	}

	if(eyes_cascade->empty() && !eyes_cascade->load(file_eyes)){
		printf("kernel %s: error loading %s file\n", kernel_name, file_eyes);
		exit(EXIT_FAILURE);
	}

	//Add defaults if necessary in else part
	if (device_type == DEVICE_UCONF)
		device_type = DEVICE_CPUMT;

	if(cuda_support)
		confCUDA_FD(file_face, file_eyes);
	else if(device_type == DEVICE_GPGPU)
		device_type = DEVICE_NONE;
}


long long facedetection(unsigned char *data, double scale_factor, int height_parm)
{
	std::vector<Rect> faces;
	unsigned int row_stride = width * bpp;
	long long faces_found = 0;

	Mat frame(height, width, CV_MAKETYPE(CV_8U, bpp), buffer, row_stride);
	Mat frame_gray;

	cvtColor(frame, frame_gray, CV_RGB2GRAY);
	//equalizeHist(frame_gray, frame_gray);

	face_cascade->detectMultiScale(frame_gray, faces, scale_factor, 30, 0|CASCADE_SCALE_IMAGE);

	faces_found = faces.size();

	for (size_t i = 0; i < faces.size(); i++) {
		Point center(faces[i].x + faces[i].width * 0.5, faces[i].y + faces[i].height * 0.5);
		ellipse(frame, center, Size(faces[i].width * 0.5, faces[i].height * 0.5), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);

		Mat faceROI = frame_gray(faces[i]);
		std::vector<Rect> eyes;

		//-- In each face, detect eyes
		eyes_cascade->detectMultiScale(faceROI, eyes);

		for (size_t j = 0; j < eyes.size(); j++) {
			Point center(faces[i].x + eyes[j].x + eyes[j].width * 0.5, faces[i].y + eyes[j].y + eyes[j].height * 0.5);
			int radius = cvRound((eyes[j].width + eyes[j].height) * 0.25);
			circle(frame, center, radius, Scalar(255, 0, 0), 4, 8, 0);
		}
	}

	return faces_found;
}


void process_frame_facedetection() {
	faces_detected = 0;

	switch (device_type) {
	case DEVICE_UCONF:

		printf("kernel %s: device not configured\n", kernel_name);

		break;
	case DEVICE_CPUST:
		omp_set_num_threads(1);

		startMeasures();

		faces_detected = facedetection(buffer, factor, height);

		stopMeasures();
		break;
	case DEVICE_CPUMT:
		omp_set_num_threads(std::thread::hardware_concurrency());

		startMeasures();

		faces_detected = facedetection(buffer, factor, height);

		stopMeasures();
		break;
	case DEVICE_GPGPU:
		startCUDAMeasures();

		faces_detected = facedetection_gpu(buffer, factor, height);

		stopCUDAMeasures();
		break;
	case DEVICE_NONE:
		break;
	case DEVICE_MIXED:
			omp_set_num_threads(std::thread::hardware_concurrency());

			startMeasures();

			faces_detected = facedetection(buffer, factor, height / 2);

			stopMeasures();

			startCUDAMeasures();

			faces_detected = facedetection_gpu(&buffer[width * bpp * height / 2], factor, height / 2);

			stopCUDAMeasures();

			break;
	default:
		printf("kernel %s: device unknown\n", kernel_name);
		break;
	}
}

void send_metrics_facedetection(int frame_num) {
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
			send_qos_msg(frame_num, FTYPE_CPU_FAC, ((double) (time_proc)) / 1e6, faces_detected, &rubbish);
			break;
		case DEVICE_NONE:
			break;
		case DEVICE_GPGPU:
			send_qos_msg(frame_num, FTYPE_CPU_FAC, ((double) (time_gpgpu)) / 1e6, faces_detected, &rubbish);
			break;
		case DEVICE_MIXED:
			send_qos_msg(frame_num, FTYPE_CPU_FAC, ((double) (time_proc + time_gpgpu)) / 1e6, faces_detected, &rubbish);
			break;
		default:
			printf("kernel %s: device unknown\n", kernel_name);
			break;
		}
	}
}

void free_child_resources_facedetection()
{
	delete face_cascade;
	delete eyes_cascade;

	if(cuda_support)
		freeCUDA_FD();

	freePapi();

	printMeasures(kernel_name);

	printCUDAMeasures(kernel_name);
}

