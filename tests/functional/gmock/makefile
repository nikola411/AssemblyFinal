GTEST_DIR=../../unit-tests/googletest/googletest
GMOCK_DIR=../../unit-tests/googletest/googlemock
USER_DIR=../

CPPFLAGS += -isystem $(GTEST_DIR)/include  -isystem $(GMOCK_DIR)/include
CXXFLAGS += -g -Wall -Wextra -pthread

GTEST_HEADERS = $(GTEST_DIR)/include/gtest/*.h \
                $(GTEST_DIR)/include/gtest/internal/*.h

GMOCK_HEADERS = $(GMOCK_DIR)/include/gmock/*.h \
                $(GMOCK_DIR)/include/gmock/internal/*.h \
                $(GTEST_HEADERS)

all : $(TESTS)

clean :

GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)
GTEST_OUT = ./

gtest-all.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest-all.cc -o ${GMOCK_OUT}/$@

gtest_main.o : $(GTEST_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GTEST_DIR)/src/gtest_main.cc -o ${GTEST_OUT}/$@

gtest.a : ${GTEST_OUT}/gtest-all.o
	$(AR) $(ARFLAGS) ${GTEST_OUT}/$@ $^

gtest_main.a : ${GTEST_OUT}/gtest-all.o ${GTEST_OUT}/gtest_main.o
	$(AR) $(ARFLAGS) ${GTEST_OUT}/$@ $^

GMOCK_SRCS_ = $(GMOCK_DIR)/src/*.cc $(GMOCK_HEADERS)
GMOCK_OUT = ./

gmock-all.o : $(GMOCK_SRCS_)
	$(CXX) $(CPPFLAGS)  -I$(GMOCK_DIR) -I$(GTEST_DIR) $(CXXFLAGS) -c $(GMOCK_DIR)/src/gmock-all.cc -o $(GMOCK_OUT)/$@

gmock_main.o : $(GMOCK_SRCS_)
	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) -I$(GMOCK_DIR) $(CXXFLAGS) -c $(GMOCK_DIR)/src/gmock_main.cc -o $(GMOCK_OUT)/$@

gmock.a : $(GMOCK_OUT)/gmock-all.o $(GMOCK_OUT)/gtest-all.o
	$(AR) $(ARFLAGS) $(GMOCK_OUT)/$@ $^

gmock_main.a : $(GMOCK_OUT)/gmock-all.o gtest-all.o gmock_main.o
	$(AR) $(ARFLAGS) $(GMOCK_OUT)/$@ $^