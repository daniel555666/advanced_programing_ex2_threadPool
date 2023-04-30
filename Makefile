.PHONY: all
all: task stdinExample

task:	codec.h basic_main.c
	gcc basic_main.c ./libCodec.so -o encoder

stdinExample:	stdin_main.c
		gcc stdin_main.c ./libCodec.so -o tester

.PHONY: clean
clean:
	-rm encoder tester 2>/dev/null
