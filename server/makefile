CXX = g++
OUTPUT = -o bin/main
INCLUDES = -I../common/ -I./src/vendor/
FLAGS = -std=c++17 -g -Wall $(INCLUDES) -pthread
OBJS = main.o instance.o map.o
BIN_OBJS = $(addprefix bin/objs/, $(OBJS))

# FLAGS THAT DEPEND ON THE OPERATING SYSTEM
OSFLAG :=
ifeq ($(OS),Windows_NT)
	OSFLAG += -lws2_32 -m32
# else
endif

VPATH = src src/game ../socket ../common bin bin/objs
TARGET = bin/server

all: $(TARGET)

$(TARGET): bin bin_objs $(OBJS) main.cpp
	$(CXX) $(FLAGS) $(BIN_OBJS) -o $(TARGET) $(OSFLAG)

run: $(TARGET)
	./bin/server

bin:
	mkdir -p bin

bin_objs:
	mkdir -p bin/objs

main.o : main.cpp util.hpp
instance.o: instance.cpp instance.hpp
map.o: map.cpp map.hpp

.cpp.o:
	$(CXX) $(FLAGS) -c $< -o bin/objs/$@

clean:
	@\rm -rf bin/objs/*.o $(TARGET)