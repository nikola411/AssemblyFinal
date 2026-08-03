CXXFLAGS += -g -pthread

BASE_DIR = ../..

INCLUDE = -I$(BASE_DIR)/inc/emulator/
INCLUDE += -I$(BASE_DIR)/inc/instruction/
INCLUDE += -I$(BASE_DIR)/inc/
INCLUDE += -I$(BASE_DIR)/inc/data_types/
INCLUDE += -I$(BASE_DIR)/inc/assembly/

SOURCE = $(wildcard $(BASE_DIR)/src/emulator/*.cpp)
SOURCE += $(wildcard $(BASE_DIR)/src/instruction/*.cpp)
SOURCE += $(BASE_DIR)/src/Utility.cpp
SOURCE += $(BASE_DIR)/src/ErrorHandling.cpp

OUT = $(BASE_DIR)/emulator

emu:
	g++ $(CXXFLAGS) $(INCLUDE) $(SOURCE) -o $(OUT)