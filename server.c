#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "db.h"
#include <pthread.h>
#include <sys/time.h>
#include <signal.h>
#include <time.h>

Node_t *head;

int handle_command(char *, char *, int len);


void* RunClient() {
	int i = 0; char c;
	char command[256] = {0};
	char response[256] = {0};
	while(1){
		i = 0;
		while(1) {
			read(0,&c,1);
			if ( c == '\n')
				break;
			command[i]=c;
			i++;
		}
		char *aux = (char*)malloc(sizeof(char)*i);
		handle_command(command, response, sizeof(response));
		strcpy(aux,command);
		memset(command, 0, 256*(sizeof(char)));
		printf("%s\n", response );
	}
}

int handle_command(char *command, char *response, int len) {
	if (command[0] == EOF) {
		strncpy(response, "all done", len-1);
		return(0);
	} else {
		interpret_command(command, response, len);
	}
	return(1);
}

int main(int argc, char *argv[]) {
	if (argc != 1) {
		fprintf(stderr, "Usage: server\n");
		exit(1);
	}
	RunClient();
	return(0);
}
