#ifndef SHELL_H
#define SHELL_H

#define CMD_INFO "INFO"
#define CMD_DIR "DIR"
#define CMD_CD "CD"
#define CMD_GET "GET"
#define CMD_PUT "PUT"

void shellLoop(int fd, char* filename);
void storeVolID();
void init(char* filename, int fd);

#endif
