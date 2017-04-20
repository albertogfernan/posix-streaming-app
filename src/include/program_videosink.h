/*
 * program_videosink.h
 *
 *  Created on: Aug 9, 2016
 *      Author: agarcia
 */

#ifndef PROGRAM_VIDEOSINK_H_
#define PROGRAM_VIDEOSINK_H_

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include <cv.h>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <opencv2/imgproc/imgproc.hpp>

#include "utilities.h"
#include "ipc_communications.h"
#include "debug_header.h"
#include "global_params.h"

void execute_videosink(int id, int num_ops, program_opt_t* options);

#endif /* PROGRAM_VIDEOSINK_H_ */
