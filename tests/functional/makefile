gmock-lib:
	$(MAKE) -s gmock.a -C ./gmock/

BASE_INCLUDE = ../../inc
BASE_SOURCE = ../../src


GTEST_DIR=../unit-tests/googletest/googletest
GMOCK_DIR=../unit-tests/googletest/googlemock
USER_DIR=./

CPPFLAGS += -isystem $(GTEST_DIR)/include  -isystem $(GMOCK_DIR)/include
CXXFLAGS += -g -Wall -Wextra -pthread

CODE_INCLUDE = ./../../inc/
CODE_SOURCE = ../../src/

OTHER_OUT = ./out/tests
OBJ = $(object)

DEPENDENCIES = $(GTEST_HEADERS) ./$(GTEST_OUT)/gtest.a

TARGET_INCLUDE=-I$(CODE_INCLUDE) -I$(CODE_INCLUDE)/instruction -I$(CODE_INCLUDE)/assembly 

assembly-tests.o : $(USER_DIR)/assembly-tests.cpp 
	$(CXX) $(CPPFLAGS) $(TARGET_INCLUDE) $(CXXFLAGS) -c $(USER_DIR)/assembly-tests.cpp -o $(OTHER_OUT)/assembly-tests.o

tests-run: assembly-tests.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(wildcard ./out/tests/*.o)  ./out/Assembly.a ./gmock/gmock_main.a  -o ./out/tests/end-to-end

end-to-end.o:
	$(CXX) $(CPPFLAGS) $(TARGET_INCLUDE) $(CXXFLAGS) -c $(USER_DIR)/end-to-end.cpp -o ./tests/end-to-end.o

tests-eoe: end-to-end.o
	$(CXX) $(CXXFLAGS) ./tests/end-to-end.o ../binary/Assembly.a