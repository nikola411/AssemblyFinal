CXXFLAGS += -g -pthread

BASE_DIR = ../..

INCLUDE = -I$(BASE_DIR)/inc/assembly/
INCLUDE += -I$(BASE_DIR)/inc/instruction/
INCLUDE += -I$(BASE_DIR)/inc/
INCLUDE += -I$(BASE_DIR)/inc/util/
INCLUDE += -I$(BASE_DIR)/misc/parser/
INCLUDE += -I$(BASE_DIR)/misc/scanner/
INCLUDE += -I$(BASE_DIR)/inc/data_types/

SOURCE = $(wildcard $(BASE_DIR)/src/assembly/*.cpp)
SOURCE += $(wildcard $(BASE_DIR)/src/instruction/*.cpp)
SOURCE += $(wildcard $(BASE_DIR)/src/*.cpp)
SOURCE += $(wildcard $(BASE_DIR)/misc/parser/*.cpp)
SOURCE += $(wildcard $(BASE_DIR)/misc/scanner/*.cpp)
SOURCE += $(wildcard $(BASE_DIR)/src/data_types/*.cpp)

OUT = $(BASE_DIR)/asembler

asm:
	g++ $(CXXFLAGS) $(INCLUDE) $(SOURCE) -o $(OUT)