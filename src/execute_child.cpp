#include "execute_child.h"

void execute_child(int id, char* conf_line)
{
	int num_ops;
	char* program;
	program_opt_t* options;

	//Parse options
	num_ops = parse_options(conf_line, &program, &options);

	debug_print("Executing %s\n", program);

	if(strcmp(program, "filesrc") == 0)
	{
		execute_filesrc(id, num_ops, options);
	}
	else if(strcmp(program, "movementfilter") == 0 || strcmp(program, "facedetection") == 0 || strcmp(program, "platedetection") == 0)
	{
		configure_kernel(program);
		execute_kernel(id, num_ops, options);
	}
	else if(strcmp(program, "videosink") == 0)
	{
		execute_videosink(id, num_ops, options);
	}
	else{
		printf("Program not recognized: %s", program);
	}

	free(options);

	return;

}
