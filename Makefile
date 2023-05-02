.PHONY: all
all: coder

coder: codec.h main.cpp
	g++ main.cpp -ldl -pthread ./libCodec.so -o coder

.PHONY: clean
clean:
	-rm coder 2>/dev/null
