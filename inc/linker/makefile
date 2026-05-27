CXXFLAGS += -g -pthread

BASE_DIR = ../..

INCLUDE = -I$(BASE_DIR)/inc/linker/

SOURCE = $(wildcard $(BASE_DIR)/src/linker/*.cpp)

OUT = $(BASE_DIR)/linker

lnk:
	g++ $(CXXFLAGS) $(INCLUDE) $(SOURCE) -o $(OUT)