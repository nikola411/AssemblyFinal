GTEST_DIR=./googletest/googletest
USER_DIR=.

CPPFLAGS += -isystem $(GTEST_DIR)/include
CXXFLAGS += -g -Wall -Wextra -pthread

TESTS = $(wildcard ./*-tests.cpp)

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

all : $(TESTS)

clean :
	rm -f ./out/tests/.*

GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GTEST_OUT = ./out/gtest

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc -o ${GTEST_OUT}/$@

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest_main.cc -o ${GTEST_OUT}/$@

gtest.a : ${GTEST_OUT}/gtest-all.o
	$(AR) $(ARFLAGS) ${GTEST_OUT}/$@ $^

gtest_main.a : ${GTEST_OUT}/gtest-all.o ${GTEST_OUT}/gtest_main.o
	$(AR) $(ARFLAGS) ${GTEST_OUT}/$@ $^

UTILITY_INCLUDE = ./../../inc/
UTILITY_SOURCE = ../../src/Utility.cpp
UTILITY_OUT = $(OTHER_OUT)/utility.o

OTHER_OUT = ./out/other

utility.o : $(UTILITY_INCLUDE) $(UTILITY_SOURCE)
	$(CXX) $(CXXFLAGS) -I$(UTILITY_INCLUDE) -c $(UTILITY_SOURCE) -o $(OTHER_OUT)/utility.o

base.o : ./main.cpp
	$(CXX) $(CXXFLAGS) -I$(GTEST_DIR) -c ./main.cpp -o $(OTHER_OUT)/main.o

TEST_OUT = ./out/tests
OBJ = $(object)

DEPENDENCIES = $(GTEST_HEADERS) ./$(GTEST_OUT)/gtest.a

$(OBJ).o : $(USER_DIR)/$(OBJ).cpp $(DEPENDENCIES)
	$(CXX) $(CPPFLAGS) -I$(UTILITY_INCLUDE) $(CXXFLAGS) -c $(USER_DIR)/$(OBJ).cpp -o $(OTHER_OUT)/$(OBJ).o

tests-run :
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(wildcard ./out/other/*) ./out/gtest/gtest.a -o ./out/tests/run
