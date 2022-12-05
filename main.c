#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "shell.h"

int main(int argc, char *argv[]) {
	int fd;
	if (argc != 2) {
		printf("Usage: %s <file>\n", argv[0]);
		exit(1);
	}
	char *file = argv[1];
 	fd = open(file, O_RDWR);
	if (-1 == fd) {
		perror("opening file: ");
		exit(1);
	}
	shellLoop(fd, argv[1]);
	close(fd);
}
