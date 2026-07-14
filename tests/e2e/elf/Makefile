CXX      = g++
CXXFLAGS = -g -std=c++17 -Wall -Wextra

BASE = ../../..

INCLUDE  = -I$(BASE)/inc/elf/
INCLUDE += -I$(BASE)/inc/util/
INCLUDE += -I$(BASE)/inc/

SOURCES  = $(BASE)/src/elf/Elf.cpp
SOURCES += $(BASE)/src/elf/ElfHelpers.cpp
SOURCES += $(BASE)/src/Utility.cpp

OUT = elf_runner

build:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SOURCES) elf_runner.cpp -o $(OUT)

clean:
	rm -f $(OUT)

.PHONY: build clean
