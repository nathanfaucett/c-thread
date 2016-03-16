CC := clang

OPT_LVL := -O2

DEPS = ./deps
SRC = ./src

C_FLAGS := -ansi -pthread $(OPT_LVL) -I$(DEPS) -I$(SRC) -Wall


all: install build_test run_test

run_test:
	./test/main

build_test: build_lib.o
	$(CC) $(C_FLAGS) ./test/lib.o ./test/main.c -o ./test/main

build_lib.o:
	$(CC) $(C_FLAGS) -c ./lib.c -o ./test/lib.o

install:
	clib install

clean:
	$(RM) ./test/main **/*.o **/*~
	$(RM) -rf deps
