#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>

int condition = 0;
pid_t elevator;

void handler_sigint(int s){
	if (condition == 1){
		puts("The button on the 1 floor is pressed");
	} else{
		puts("The button on the 1 floor is not pressed");
	}
	signal(SIGINT, handler_sigint);
	return;
}

void handler_pressing_of_button(int s){
	if (condition == 0){
		condition = 1;
		puts("The button on the 1 floor is pressed");
		kill(elevator, SIGUSR2);
	}
	return;
}

void handler_elevator_arrival(int s){
	if (condition == 1){
		condition = 0;
		puts("The button on the 1 floor is not pressed, elevator is here");
	}
	return;
}

void handler_sigterm(int s){
	printf("==%d== Operator: Terminated\n", getpid());
	exit(0);
}

int main(void){
	read(0, &elevator, sizeof(pid_t));
	signal(SIGINT, handler_sigint);
	signal(SIGUSR1, handler_pressing_of_button);
	signal(SIGUSR2, handler_elevator_arrival);
	signal(SIGTERM, handler_sigterm);
	while(1){
		pause();
	}
	return 0;
}