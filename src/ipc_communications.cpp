/*
 * ipc_communications.cpp
 *
 *  Created on: Aug 7, 2016
 *      Author: garci
 */
#include "ipc_communications.h"

static int num_procs;
mqd_t* frame_mqueues;
mqd_t* conf_mqueues;
mqd_t coord_queue;
char **frame_q_names;
char **conf_q_names;
char coord_name[100];

void init_queues(int num_proc)
{
	init_frame_queues(num_proc);
	init_conf_queues(num_proc);
	init_coord_queue();
}


void init_conf_queues(int num_proc)
{
	int i;
	struct mq_attr attr;
	int random;

	num_procs = num_proc;

	/* initialize the queue attributes */
	attr.mq_flags = 0;
	attr.mq_maxmsg = 2;
	attr.mq_msgsize = sizeof(app_msg_conf_t);
	attr.mq_curmsgs = 0;

	conf_q_names = (char**)malloc(sizeof(char*) * (num_proc - 1));
	conf_mqueues = (mqd_t*)malloc(sizeof(mqd_t) * (num_procs - 1));

	for(i=0; i<num_procs -1; i++)
	{
		random = rand();
		conf_q_names[i] = (char*)malloc(sizeof(char) * QNAME_MAX_LENGTH);
		snprintf(conf_q_names[i], QNAME_MAX_LENGTH, "/%s-qos-%d-%d", QUEUE_NAMES, i, random);
		conf_mqueues[i] = mq_open(conf_q_names[i], O_CREAT | O_RDWR | O_NONBLOCK, 0644, &attr);
		if(conf_mqueues[i] < 0)
		{
			perror("Error creating the conf queue");
			exit(EXIT_FAILURE);
		}
	}
}


void init_frame_queues(int num_proc)
{
	int i;
	struct mq_attr attr;
	int random;

	num_procs = num_proc;

	/* initialize the queue attributes */
	attr.mq_flags = 0;
	attr.mq_maxmsg = 5;
	attr.mq_msgsize = sizeof(app_msg_frame_t);
	attr.mq_curmsgs = 0;

	frame_q_names = (char**)malloc(sizeof(char*) * (num_proc - 1));
	frame_mqueues = (mqd_t*)malloc(sizeof(mqd_t) * (num_procs - 1));

	for(i=0; i<num_procs -1; i++)
	{
		random = rand();
		frame_q_names[i] = (char*)malloc(sizeof(char) * QNAME_MAX_LENGTH);
		snprintf(frame_q_names[i], QNAME_MAX_LENGTH, "/%s-fram-%d-%d", QUEUE_NAMES, i, random);
		frame_mqueues[i] = mq_open(frame_q_names[i], O_CREAT | O_RDWR, 0644, &attr);
		if(frame_mqueues[i] < 0)
		{
			perror("Error creating the frame queue");
			exit(EXIT_FAILURE);
		}
	}
}

void init_coord_queue()
{
	int random;
	struct mq_attr attr_coord;

	/* initialize the queue attributes */
	attr_coord.mq_flags = 0;
	attr_coord.mq_maxmsg = 10;
	attr_coord.mq_msgsize = sizeof(app_msg_qos_t);
	attr_coord.mq_curmsgs = 0;

	random = rand();
	snprintf(coord_name, QNAME_MAX_LENGTH, "/%s-coord-%d", QUEUE_NAMES, random);
	coord_queue = mq_open(coord_name, O_CREAT | O_RDWR, 0644, &attr_coord);
	if (coord_queue < 0) {
		perror("Error creating the coord queue");
		exit(EXIT_FAILURE);
	}
}


void send_frame_msg(int queue_no, int frame_number, char* shared_mem_name)
{
	app_msg_frame_t message;

	message.frame_number = frame_number;
	strncpy(message.shmem_name, shared_mem_name, SHMEM_MAX_LENGTH);

	//printf("Sending %d %d %d %s\n", queue_no, frame_mqueues[queue_no], message.frame_number, message.shmem_name);

	if(mq_send(frame_mqueues[queue_no], (char *)&message, sizeof(app_msg_frame_t), 0) < 0)
	{
		perror("Error sending the frame message");
	}
}

void receive_frame_msg(int queue_no, int* frame_number, char* shared_mem_name)
{
	app_msg_frame_t message;

	if(mq_receive(frame_mqueues[queue_no], (char*)&message, sizeof(app_msg_frame_t), NULL) < 0)
	{
		perror("Error receiving the frame message");
	}

	//printf("Receiving %d %d %d %lu %s\n", queue_no, mqueues[queue_no], message.frame_number, message.size, message.shmem_name);

	*frame_number = message.frame_number;
	strncpy(shared_mem_name, message.shmem_name, SHMEM_MAX_LENGTH);
}

void send_qos_msg(int frame_number, int field_type, double field_content_1, unsigned long long field_content_2, struct timeval* timestamp)
{
	app_msg_qos_t message;

	message.frame_number = frame_number;
	message.pid_sender = getpid();
	message.field_type = field_type;
	message.field_content_1 = field_content_1;
	message.field_content_2 = field_content_2;
	memcpy(&(message.timestamp),timestamp, sizeof(struct timeval));

	if (mq_send(coord_queue, (char *) &message, sizeof(app_msg_qos_t), 0) < 0) {
		perror("Error sending the qos message");
	}
}


void send_qos_msg_end()
{
	app_msg_qos_t message;

	message.frame_number = -1;
	message.pid_sender = getpid();

	if (mq_send(coord_queue, (char *) &message, sizeof(app_msg_qos_t), 0) < 0) {
		perror("Error sending the qos end message");
	}
}

void receive_qos_msg(int* frame_number, pid_t* pid_sender, int* field_type, double* field_content_1, unsigned long long* field_content_2, struct timeval* timestamp)
{
	app_msg_qos_t message;

	if(mq_receive(coord_queue, (char*)&message, sizeof(app_msg_qos_t), NULL) < 0)
	{
		perror("Error receiving the qos message");
		*frame_number = -2;
	}
	else
	{
		*frame_number = message.frame_number;
		*pid_sender = message.pid_sender;
		*field_type = message.field_type;
		*field_content_1 = message.field_content_1;
		*field_content_2 = message.field_content_2;
		memcpy(timestamp, &(message.timestamp), sizeof(struct timeval));
	}
}


void send_conf_msg(int queue_no, const char* key, const char* value)
{
	app_msg_conf_t message;

	strncpy(message.key, key, OPT_MAX_LENGTH);
	strncpy(message.value, value, OPT_MAX_LENGTH);

	if (mq_send(conf_mqueues[queue_no], (char *) &message, sizeof(app_msg_conf_t), 0) < 0) {
		perror("Error sending the conf message");
	}
}


void receive_conf_msg(int queue_no, int* num_conf, program_opt_t* options)
{
	if(mq_receive(conf_mqueues[queue_no], (char*)options, sizeof(app_msg_conf_t), NULL) < 0)
	{
		if(errno != EAGAIN)
		{
			perror("Error receiving the conf message");
			*num_conf = -1;
		}
		else
			*num_conf = 0;
	}
	else
	{
		*num_conf = 1;
	}
}


void close_queues()
{
	int i;
	for (i = 0; i < num_procs - 1; i++) {
		mq_close(frame_mqueues[i]);
	}

	for (i = 0; i < num_procs - 1; i++) {
		mq_close(conf_mqueues[i]);
	}

	mq_close(coord_queue);
}


void unlink_queues()
{
	int i;
	for (i = 0; i < num_procs - 1; i++) {
		mq_unlink(frame_q_names[i]);
		free(frame_q_names[i]);
	}

	for (i = 0; i < num_procs - 1; i++) {
		mq_unlink(conf_q_names[i]);
		free(conf_q_names[i]);
	}

	mq_unlink(coord_name);

	free(frame_q_names);
	free(conf_q_names);
}


