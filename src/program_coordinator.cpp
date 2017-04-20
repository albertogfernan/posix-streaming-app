/*
 * program_coordinator.cpp
 *
 *  Created on: Aug 29, 2016
 *      Author: agarcia
 */
#include "program_coordinator.h"

//#include "optimizer.h"

void coordinate(int num_procs)
{
	int num_kernels = num_procs;
	int frame_number, field_type;
	pid_t pid_sender;
	double field_content_1;
	unsigned long long field_content_2;
	struct timeval timestamp;

	while(num_kernels > 0)
	{
		receive_qos_msg(&frame_number, &pid_sender, &field_type, &field_content_1, &field_content_2, &timestamp);

		if(frame_number > -1)
		{
			switch (field_type) {
			case FTYPE_CPU_PIX:
				printf("coordinator: frame %d processing time %lf secs, pixels different %llu\n", frame_number, field_content_1, field_content_2);
				break;
			case FTYPE_CPU_FAC:
				printf("coordinator: frame %d processing time %lf secs, faces detected %llu\n", frame_number,field_content_1, field_content_2);
				break;
			case FTYPE_CPU_PLA:
				printf("coordinator: frame %d processing time %lf secs, plates detected %llu\n", frame_number,field_content_1, field_content_2);
				break;
			case FTYPE_DEL_FPS:
				printf("coordinator: frame %d delay to nominal %lf, fps %llu\n", frame_number, field_content_1, field_content_2);
				break;
			default:
				printf("coordinator: unknown message.\n");
				break;
			}



			if (frame_number == 100) {

			send_conf_msg(1, "scale", "2.5");	//To movementfilter
			//	send_conf_msg(2, "device", DEVICE_CPUMT_S);	//To facedetector
			}
			if (frame_number == 400) {
				//send_conf_msg(2, "device", DEVICE_GPGPU_S);	//To facedetector
			}



		}
		else if (frame_number < -1)
		{
			kill(getpid(), SIGINT);
			break;
		}
		else
			num_kernels--;
	}
}
