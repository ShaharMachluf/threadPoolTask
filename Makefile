.PHONY: all
all: task stdinExample coder

coder: codec.h main.cpp
	g++ main.cpp -ldl -pthread ./libCodec.so -o coder

task:	codec.h basic_main.c
	gcc basic_main.c -L. -l ./libCodec.so -o encoder

stdinExample:	stdin_main.c
	gcc stdin_main.c -L. -l ./libCodec.so -o tester

.PHONY: clean
clean:
	-rm encoder tester libCodec.so coder 2>/dev/null
