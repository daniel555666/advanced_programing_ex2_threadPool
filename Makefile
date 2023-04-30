here : 
SHARED_LIBRARIES= libCodec.so

.PHONY: all
all: task stdinExample coder

task: codec.h basic_main.c
	gcc basic_main.c ./libCodec.so -o encoder

stdinExample: stdin_main.c
	gcc stdin_main.c ./libCodec.so -o tester

coder: codec.h coder.o
	g++ coder.o ./libCodec.so -o coder


.PHONY: clean
clean:
	-rm encoder tester 2>/dev/null
