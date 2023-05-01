here : 
SHARED_LIBRARIES= libCodec.so

.PHONY: all
all: task stdinExample coder

task: codec.h basic_main.c
	gcc -g basic_main.c ./libCodec.so -o encoder

stdinExample: stdin_main.c
	gcc -g stdin_main.c ./libCodec.so -o tester

coder: codec.h coder.c
	gcc -g coder.c ./libCodec.so -o coder -pthread


.PHONY: clean
clean:
	-rm encoder tester coder 2>/dev/null
