#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

enum {travel_time = 8};
pid_t elevator;
int condition = 0; 		// 0 - стоит на этаже, 1 - движется 
int movement = 0;		// 0 - движение с 1 на 0 этаж, 1 - движение с 0 на 1 этаж

void handler_sigterm(int s){
	while (condition == 1){
	}
	printf("==%d== Operator: Terminated\n", getpid());
	exit(0);
}

void handler_alarm(int s){
	condition = 0;
	kill(elevator, SIGCHLD);
	return;
}

void handler_movement_0(int s){
	condition = 1;
	movement = 0;
	alarm(travel_time);
	return;
}

void handler_movement_1(int s){
	condition = 1;
	movement = 1;
	alarm(travel_time);
	return;
}

void handler_sigint(int s){
	if (condition == 1){
		int second = alarm(0);
		alarm(second);
		if (movement == 0){
			puts("Elevator goes from the 1 to the 0 floor");
			printf("Time to finish = %d\n", second);
		} else{
			puts("Elevator goes from the 0 to the 1 floor");
			printf("Time to finish = %d\n", second);
		}
	}
	return;
}

int main(void){
	read(0, &elevator, sizeof(pid_t));
	signal(SIGTERM, handler_sigterm);
	signal(SIGINT, handler_sigint);
	signal(SIGUSR1, handler_movement_1);
	signal(SIGUSR2, handler_movement_0);
	signal(SIGALRM, handler_alarm);
	while(1){
		pause();
	}
	return 0;
}