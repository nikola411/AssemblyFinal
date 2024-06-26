INC_DIR=../../inc

INCLUDE=-I$(INC_DIR)
INCLUDE+= -I$(INC_DIR)/instruction/
INCLUDE+= -I$(INC_DIR)/assembly/
INCLUDE+= -I../../misc/parser/
INCLUDE+= -I../../misc/scanner/

PROJECT_BINARY=../binary/Assembly.a

tests.o:
	g++ $(INCLUDE) -c end-to-end.cpp -o ./out/tests.o

test-run: tests.o
	g++ ./out/tests.o $(PROJECT_BINARY) -o test-run.o