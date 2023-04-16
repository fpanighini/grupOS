CC = gcc
FLAG_INC =-pthread -g -Wall -Wextra -Werror -std=gnu11

all: app worker viewer

app:
	mkdir -p bin/
	$(CC) src/application/*.c src/application/*.h lib/*.h lib/*.c -o bin/app $(FLAG_INC)


worker:
	mkdir -p bin/
	$(CC) src/worker/*.c src/worker/*.h lib/*.h lib/*.c -o bin/worker $(FLAG_INC)


viewer:
	mkdir -p bin/
	$(CC) src/viewer/*.c src/viewer/*.h lib/*.h lib/*.c -o bin/viewer $(FLAG_INC)

clean:
	rm -rf bin/*
