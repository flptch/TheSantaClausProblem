CC=gcc
CFLAGS=-std=gnu99 -Wall -Wextra -pedantic -lrt -pthread
proj2: proj2.c
	$(CC) $(CFLAGS) -o proj2 proj2.c
