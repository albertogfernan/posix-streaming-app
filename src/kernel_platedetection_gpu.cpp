/*
 * kernel_facedetection_gpu.cpp
 *
 *  Created on: Sep 2, 2016
 *      Author: agarcia
 */
#include "kernel_platedetection_gpu.h"
#include "opencv2/core/version.hpp"

#if CV_VERSION_MAJOR == 3

Code not updated or maintained, should not compile

#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/cudaobjdetect.hpp"
#include "opencv2/cudaimgproc.hpp"
#include "opencv2/cudawarping.hpp"

using namespace std;
using namespace cv;
using namespace cv::cuda;

Ptr<cuda::CascadeClassifier> face_cascade_gpu;
Ptr<cuda::CascadeClassifier> eyes_cascade_gpu;


void initCUDA_PD()
{
	int cudaDevs;

	try
	{
		cudaDevs = getCudaEnabledDeviceCount();
	}catch(Exception)
	{
		cudaDevs = 0;
	}
	if(cudaDevs < 1)
		printf("kernel facedetect: no CUDA support\n");
	else if(cudaDevs > 1)
		setDevice(1);
	else
		setDevice(0);

	cout << "kernel facedetect: ";
	cv::gpu::printShortCudaDeviceInfo(cv::gpu::getDevice());
	cout << "kernel facedetect: using OpenCV version " << CV_VERSION << endl;

	initCUDAMeasures();
}


void confCUDA_PD(char* file_face, char* file_eyes)
{
	face_cascade_gpu = cuda::CascadeClassifier::create(file_face);
	eyes_cascade_gpu = cuda::CascadeClassifier::create(file_eyes);
}


void freeCUDA_PD()
{
	face_cascade_gpu.release();
	eyes_cascade_gpu.release();

	freeCUDAMeasures();
}


long long platedetection_gpu(unsigned char *data) {
	unsigned int row_stride = width * bpp;
	GpuMat gpu_frame, frame_gray, faces, eyes;
	Mat obj_host, obj_host2;
	int radius;
	bool findLargestObject = false;
	bool filterRects = true;
	vector<Rect> faces_rect, eyes_rect;


	Mat frame(height, width, CV_MAKETYPE(CV_8U, bpp), buffer, row_stride);
	Mat frame2 = frame;
	gpu_frame.upload(frame2);

	cv::cuda::cvtColor(gpu_frame, frame_gray, CV_RGBA2GRAY);
	//cv::cuda::equalizeHist(frame_gray, frame_gray);
/*
	face_cascade_gpu->setFindLargestObject(findLargestObject);
	face_cascade_gpu->setScaleFactor(1.2);
	face_cascade_gpu->setMinNeighbors((filterRects || findLargestObject) ? 4 : 0);

	eyes_cascade_gpu->setFindLargestObject(findLargestObject);
	eyes_cascade_gpu->setMinNeighbors((filterRects || findLargestObject) ? 4 : 0);


	face_cascade_gpu->detectMultiScale(frame_gray, faces);
	face_cascade_gpu->convert(faces, faces_rect);

	for (size_t i = 0; i < faces_rect.size(); i++)
	{
		Point center(faces_rect[i].x + faces_rect[i].width * 0.5, faces_rect[i].y + faces_rect[i].height * 0.5);
		ellipse(frame, center, Size(faces_rect[i].width * 0.5, faces_rect[i].height * 0.5), 0, 0, 360, Scalar(255, 0, 255), 4, 8, 0);

		GpuMat faceROI(frame_gray(faces_rect[i]));
		eyes_cascade_gpu->detectMultiScale(faceROI, eyes);
		eyes_cascade_gpu->convert(eyes, eyes_rect);

		for(size_t j = 0; j < eyes_rect.size(); j++)
		{
			Point center(faces_rect[i].x + eyes_rect[j].x + eyes_rect[j].width * 0.5, faces_rect[i].y + eyes_rect[j].y + eyes_rect[j].height * 0.5);
			radius = cvRound((eyes_rect[j].width + eyes_rect[j].height) * 0.25);
			circle(frame, center, radius, Scalar(255, 0, 0), 4, 8, 0);
		}
	}
*/
	return faces_rect.size();
}

#else

#include <iostream>
#include <iomanip>
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/gpu/gpu.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

CascadeClassifier_GPU* plate_cascade_gpu = NULL;

bool initCUDA_PD()
{
	int cudaDevs = 0;

	try {
		cudaDevs = getCudaEnabledDeviceCount();
	} catch (cv::Exception e) {
		cudaDevs = 0;
	}

	if(cudaDevs < 1)
	{
		printf("kernel platedetect: no CUDA support\n");
		return false;
	}
	else if(cudaDevs > 1)
		setDevice(1);
	else
		setDevice(0);

	cout << "kernel platedetect: ";
	cv::gpu::printShortCudaDeviceInfo(cv::gpu::getDevice());
	cout << "kernel platedetect: using OpenCV version " << CV_VERSION << endl;
	plate_cascade_gpu = new CascadeClassifier_GPU();

	initCUDAMeasures();

	return true;
}


void confCUDA_PD(char* file_plate)
{
	if (plate_cascade_gpu->empty() && !plate_cascade_gpu->load(file_plate)) {
		printf("kernel platedetect: error loading %s file\n", file_plate);
		exit(EXIT_FAILURE);
	}
}


void freeCUDA_PD()
{
	delete plate_cascade_gpu;

	freeCUDAMeasures();
}

long long platedetection_gpu(unsigned char *data, double scale_factor, int height_parm) {
	unsigned int row_stride = width * bpp;
	unsigned int plates_found = 0;
	GpuMat gpu_frame, frame_gray, plates;
	Mat obj_host, obj_host2;

	Mat frame(height_parm, width, CV_MAKETYPE(CV_8U, bpp), buffer, row_stride);
	gpu_frame.upload(frame);

	cvtColor(gpu_frame, frame_gray, CV_RGBA2GRAY);
	//equalizeHist(frame_gray, frame_gray);

	plates_found = plate_cascade_gpu->detectMultiScale(frame_gray, plates, scale_factor, 15);
	plates.colRange(0, plates_found).download(obj_host);  // retrieve results from GPU
	
	Rect* cplates = obj_host.ptr<Rect>();
	for (size_t i = 0; i < plates_found; i++) {
		rectangle(frame, cplates[i], Scalar(255, 0, 255), 4, 8, 0);
	}
	

	return plates_found;
}

#endif
