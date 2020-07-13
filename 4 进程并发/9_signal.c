#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static void handler(int s){
	signal(SIGINT, handler);
	puts("Handler Start ... ");
	sleep(10);
}


int main(int argc, char **argv){
	signal(SIGINT, handler);
	puts("Main Start ...");
	sleep(10);

	puts("Main End");
	
	exit(0);
}
