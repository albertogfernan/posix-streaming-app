# Copyright (C) THALES 2012 All rights reserved
#
# THE PROGRAM IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED INCLUDING,
# WITHOUT LIMITATION, ANY WARRANTIES ON ITS, NON-INFRINGEMENT, MERCHANTABILITY, SECURED, INNOVATIVE OR RELEVANT NATURE, FITNESS
# FOR A PARTICULAR PURPOSE OR COMPATIBILITY WITH ANY EQUIPMENT OR SOFTWARE.
	
CC			= g++
NVCC		= nvcc
EXE			= posix-streaming-app
FLAGS		= -O3 -Wall -fopenmp -g
CPU     	= x86_64
CUDA            = /usr/local/cuda-7.5
OPENCV          = /opt/opencv-2.4.13/install
PAPI      	= /opt/papi-5.5.1/install

SRC_DIR	= ./src
OBJ_DIR	= ./obj/$(CPU)

DEFINES 	= -DDEBUG=0

INC_OCV		= -I$(OPENCV)/include/opencv -I$(OPENCV)/include
INCLUDES  	= -I$(SRC_DIR)/include $(INC_OCV) -I$(CUDA)/include/ -I$(PAPI)/include -I/opt/lp_solve_5.5 

LIB_CV		= -lopencv_core -lopencv_gpu -lopencv_imgproc -lopencv_objdetect -lopencv_highgui 
LIBRARIES 	= $(LIB_CV) -lrt -lgomp -lcuda -lcudart $(PAPI)/lib/libpapi.a -ldl 
#/opt/lp_solve_5.5/lpsolve55/bin/ux64/liblpsolve55.a  

LIBDIRS		= -L$(CUDA)/lib64 -Wl,-rpath=$(CUDA)/lib64 -L$(PAPI)/lib -Wl,-rpath=$(PAPI)/lib -L$(OPENCV)/lib -Wl,-rpath=$(OPENCV)/lib 

CPP_FILES := $(wildcard $(SRC_DIR)/*.cpp)
CUD_FILES := $(wildcard $(SRC_DIR)/*.cu)	

OBJ_FILES := $(addprefix $(OBJ_DIR)/,$(notdir $(CPP_FILES:.cpp=.o)))
OBJ_FILES += $(addprefix $(OBJ_DIR)/,$(notdir $(CUD_FILES:.cu=.o)))


all : $(OBJ_DIR) $(OBJ_FILES)
	$(CC) $(FLAGS) -o $(EXE) $(OBJ_FILES) $(LIBDIRS) $(LIBRARIES)
	#sudo -E modprobe msr
	#sudo -E setcap cap_sys_rawio=ep $(EXE)
	#sudo -E chmod 666 /dev/cpu/*/msr
	

clean: 
	rm $(OBJ_FILES)
	rmdir $(OBJ_DIR)
	rm $(EXE)
	
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CC) $(FLAGS) -std=c++11  $(DEFINES) $(INCLUDES) -c $< -o $@
	
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cu	
	$(NVCC) -gencode arch=compute_20,code=compute_20 -std=c++11  --compiler-options "$(FLAGS)" $(DEFINES) $(INCLUDES) -c $< -o $@
	
$(OBJ_DIR) :
	mkdir -p $(OBJ_DIR)
	
