CXX      = g++
CXXFLAGS = -g -std=c++17 -Wall -Wextra

BASE = ../..

INCLUDE  = -I$(BASE)/inc/elf/
INCLUDE += -I$(BASE)/inc/data_types/
INCLUDE += -I$(BASE)/inc/assembly/
INCLUDE += -I$(BASE)/inc/instruction/
INCLUDE += -I$(BASE)/inc/

SOURCES  = $(BASE)/src/elf/Elf.cpp
SOURCES += $(BASE)/src/elf/ElfHelpers.cpp
SOURCES += $(BASE)/src/Utility.cpp

LINKER_SOURCES  = $(BASE)/src/data_types/Section.cpp
LINKER_SOURCES += $(BASE)/src/Utility.cpp

OUT       = elf_test
OUT_UNLOAD = unload_test
OUT_LINKER = linker_test

build:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SOURCES) ElfTest.cpp -o $(OUT)

build-unload:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(SOURCES) UnloadLinkableTest.cpp -o $(OUT_UNLOAD)

build-linker:
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(LINKER_SOURCES) LinkerTest.cpp -o $(OUT_LINKER)

run: build
	./$(OUT)

run-unload: build-unload
	./$(OUT_UNLOAD)

run-linker: build-linker
	./$(OUT_LINKER)

clean:
	rm -f $(OUT) $(OUT_UNLOAD) $(OUT_LINKER)

.PHONY: build run build-unload run-unload build-linker run-linker clean
