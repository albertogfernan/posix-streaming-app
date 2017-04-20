/*
 * main.cpp
 *
 *  Created on: Aug 6, 2016
 *      Author: garci
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "debug_header.h"
#include "utilities.h"
#include "ipc_communications.h"
#include "execute_child.h"
#include "program_coordinator.h"

static int num_procs;
pid_t * app_pids;
FILE * app_desc;


struct timeval timestamp_zero;
size_t frame_size = 0;
int width = 0;
int height = 0;
int bpp = 0;
int fps = 0;

void collect_childs()
{
	int i;
	int status;

	for(i = 0; i< num_procs; i++)
	{
		waitpid(app_pids[i], &status, 0);
		printf("main: child %d collected with status %d\n", app_pids[i], status);
	}

	free(app_pids);
}

void free_resources()
{
	fclose(app_desc);
	close_queues();
}

void ctrlc_handler(int s){
	int i;
	for (i = 0; i < num_procs; i++) {
		if (app_pids[i] > 0)
			kill (app_pids[i], SIGKILL);
	}

	//exit(EXIT_FAILURE);
}

void set_ctrlc_handler()
{
	struct sigaction ctrc_handler;

	ctrc_handler.sa_handler = ctrlc_handler;
	sigemptyset(&ctrc_handler.sa_mask);
	ctrc_handler.sa_flags = 0;

	sigaction(SIGINT, &ctrc_handler, NULL);
}

void check_configuration_main(int num_ops, program_opt_t* options)
{
	int i;

	for(i=0; i<num_ops; i++)
	{
		if(strncmp(options[i].key, "width", OPT_MAX_LENGTH) == 0)
		{
			width = atoi(options[i].value);
		}
		else if(strncmp(options[i].key, "height", OPT_MAX_LENGTH) == 0)
		{
			height = atol(options[i].value);
		}
		else if (strncmp(options[i].key, "bpp", OPT_MAX_LENGTH) == 0)
		{
			bpp = atol(options[i].value);
		}
		else if (strncmp(options[i].key, "fps", OPT_MAX_LENGTH) == 0)
		{
			fps = atoi(options[i].value);
		}
	}

	if (width <= 0 || height <= 0 || bpp <= 0 || fps <= 0) {
		printf("filesrc: width or height or bpp or fps not specified\n");
		exit(EXIT_FAILURE);
	}

	frame_size = width * height * bpp;

	stamp(&timestamp_zero);
}

int main(int argc, char *argv[])
{
	int i;
	char* text_line = NULL;
	size_t zero = 0;
	int num_ops;
	char* program;
	program_opt_t* options;

	if (argc != 2)
	{
		printf("usage: %s <app description file>\n", argv[0]);
		return EXIT_FAILURE;
	}

	srand(time(NULL));

	set_ctrlc_handler();

	app_desc = fopen(argv[1], "r");
	if (app_desc == NULL) {
		perror("Error opening app file");
		exit(EXIT_FAILURE);
	}

	num_procs = count_lines(app_desc) - 1;

	init_queues(num_procs);

	app_pids = (pid_t*)malloc(sizeof(pid_t) * num_procs);

	//GLOBAL OPS
	if (getline(&text_line, &zero, app_desc) > 0) {
		//Check main and read pars
		num_ops = parse_options(text_line, &program, &options);
		if(strcmp(program, "main") == 0)
		{
			check_configuration_main(num_ops, options);
		}
		else
		{
			perror("Error reading global pars.");
			exit(EXIT_FAILURE);
		}
		free(text_line);
		text_line = NULL;
	}
	else
	{
		perror("Error reading global pars.");
		exit(EXIT_FAILURE);
	}


	//CHILD CREATION
	for (i = 0; i < num_procs; i++) {
		if (getline(&text_line, &zero, app_desc) > 0) {
			app_pids[i] = fork();
			switch (app_pids[i]) {
			case -1:
				perror("Error creating child.\n");
				break;
			case 0:
				//Function
				execute_child(i, trim_whitespace(text_line));

				//Free resources
				free_resources();

				free(text_line);
				text_line = NULL;

				printf("child %d finishing\n", getpid());

				exit(EXIT_SUCCESS);
				break;
			default:
				free(text_line);
				text_line = NULL;
				break;
			}
		}
	}

	coordinate(num_procs);

	free_resources();

	collect_childs();

	unlink_queues();

	return EXIT_SUCCESS;
}
