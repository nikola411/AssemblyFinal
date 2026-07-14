CXXFLAGS += -g -pthread

BASE_DIR = ../..

INCLUDE = -I$(BASE_DIR)/inc/linker/
INCLUDE += -I$(BASE_DIR)/inc/
INCLUDE += -I$(BASE_DIR)/inc/elf/
INCLUDE += -I$(BASE_DIR)/inc/data_types/
INCLUDE += -I$(BASE_DIR)/inc/assembly/
INCLUDE += -I$(BASE_DIR)/inc/instruction/

SOURCE = $(wildcard $(BASE_DIR)/src/linker/*.cpp)

OUT = $(BASE_DIR)/linker

lnk:
	g++ $(CXXFLAGS) $(INCLUDE) $(SOURCE) -o $(OUT)