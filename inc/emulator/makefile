CXXFLAGS += -g -pthread

BASE_DIR = ../..

INCLUDE = -I$(BASE_DIR)/inc/emulator/
INCLUDE += -I$(BASE_DIR)/inc/instruction/

SOURCE = $(wildcard $(BASE_DIR)/src/emulator/*.cpp)
SOURCE += $(wildcard $(BASE_DIR)/src/instruction/*.cpp)

OUT = $(BASE_DIR)/emulator

emu:
	g++ $(CXXFLAGS) $(INCLUDE) $(SOURCE) -o $(OUT)