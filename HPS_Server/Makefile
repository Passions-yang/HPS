TARGET=Server
CC=g++
INCLUDE=-I./
CFLAGS=-g -Wall
LDFLAGS=-lpthread

SRC=$(wildcard *.cpp)
OBJ=$(patsubst %.cpp,%.o,$(SRC))


$(TARGET) : $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ):$(SRC)
	$(CC) -c $(INCLUDE) $(CFLAGS) $<  -o $@



.PHONY:clean

clean:
	-rm -fr *.o
	-rm -fr $(TARGET)

