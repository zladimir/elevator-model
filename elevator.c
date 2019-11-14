#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

enum {time = 7};
int elevator_condition = 0;		// 0 - стоит на этаже, 1 - движется 
int movement = 0;				// 0 - движение с 1 на 0 этаж, 1 - движение с 0 на 1 этаж
int door_status = 0;			// 0 - закрыты, 1 - открыты
int location = 1; 				// 1 - первый этаж, 0 - нулевой 
int button_queue_0 = 0;			// поставить кновку в очередь 
int button_queue_1 = 0;	
int button_queue_cab = 0;	

pid_t button_0, button_1, cabin_button, lift;

void alrm(int s){
	door_status = 0;
	puts("Doors are closed");
	printf("==%d== Operator: Terminated\n", getpid());
	exit(0);
}

void handler_sigterm(int s){
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);
	signal(SIGFPE, SIG_IGN);
	signal(SIGCHLD, handler_sigterm);
	signal(SIGALRM, alrm);
	if (elevator_condition == 0){
		puts("Elevator is on the floor");
		if (door_status == 1){
			puts("Doors are open");
			alarm(time);
		}else{
			printf("==%d== Operator: Terminated\n", getpid());
			exit(0);
		}
	}
	else elevator_condition = 0;
}

void handler_sigint(int s){
	if (elevator_condition == 0){
		int second = alarm(0);
		alarm(second);
		if (location == 1){
			if (door_status == 0){
				puts("Elevator is on the 1 floor. doors are closed");
			} else {
				puts("Elevator is on the 1 floor. doors are open");
				printf("Time to close the door = %d\n", second);
			}
		} else{
			if (door_status == 0){
				puts("Elevator is on the 0 floor. doors are closed");
			} else {
				puts("Elevator is on the 0 floor. doors are open");
				printf("Time to close the door = %d\n", second);
			}
		}
	}
	return;
}

void handler_(int s){
	if(location == 0){
		if (s == SIGUSR1){
			alarm(time);
			kill(button_0, SIGUSR2);
		}
		if (s == SIGUSR2) button_queue_1 = 1;
	}else{
		if (s == SIGUSR2){
			alarm(time);
			kill(button_1, SIGUSR2);
		}
		if (s == SIGUSR1) button_queue_0 = 1;
	}
	if (s == SIGFPE) button_queue_cab = 1;
	return;
}

void queue(){
	signal(SIGUSR1, handler_);
	signal(SIGUSR2, handler_);
	signal(SIGFPE, handler_);
	return;
}

void handler_button_0(int s){
	if (elevator_condition == 0){		// Лифт стоит на этаже
		if (location == 0){
			if (door_status == 0){
				door_status = 1;
				puts("Doors are open");
			}
			queue();
			alarm(time);
			kill(button_0, SIGUSR2);
		} 
		else{
			if (door_status == 1){
				door_status = 0;
				puts("Doors are closed");
			}	
			elevator_condition = 1;
			movement = 0;
			puts("Elevator goes from 1 to 0 floor");
			kill(lift, SIGUSR2);	// с 1 на 0
		}
	}
	else{
		if (movement == 1){
			button_queue_0 = 1;	// с 0 на 1
		}
	}
	return;
}


void handler_button_1(int s){
	if (elevator_condition == 0){		// Лифт стоит на этаже
		if (location == 1){
			if (door_status == 0){
				door_status = 1;
				puts("Doors are open");
			}
			queue();
			alarm(time);
			kill(button_1, SIGUSR2);
		} 
		else{
			if (door_status == 1){
				door_status = 0;		
				puts("Doors are closed");
			}	
			elevator_condition = 1;
			movement = 1;
			puts("Elevator goes from 0 to 1 floor");
			kill(lift, SIGUSR1);	// с 0 на 1
		}
	}
	else{
		if (movement == 0){
			button_queue_1 = 1;		// с 0 на 1
		}
	}
	return;
}

void handler_cabin_button(int s){
	if (elevator_condition == 0){		// Лифт стоит на этаже
		if (door_status == 1){
			door_status = 0;
			puts("Doors are closed");
		}
		elevator_condition = 1;
		if (location == 0){
			movement = 1;
			puts("Elevator goes from 0 to 1 floor");
			kill(lift, SIGUSR1);
		}else{
			movement = 0;
			puts("Elevator goes from 1 to 0 floor");
			kill(lift, SIGUSR2);
		}
	}
	return;
}

void handler_alarm(int s){
	door_status = 0;
	puts("Doors are closed");
	signal(SIGUSR1, handler_button_0);
	signal(SIGUSR2, handler_button_1);
	signal(SIGFPE, handler_cabin_button);
	if (location == 1){
		if (button_queue_0 == 1) raise(SIGUSR1);
		else{
			if (button_queue_cab == 1) raise(SIGFPE);
		}
	}else{
		if (button_queue_1 == 1) raise(SIGUSR2);
		else{
			if (button_queue_cab == 1) raise(SIGFPE);
		}
	}
	return;
}

void handler_lift(int s){
	elevator_condition = 0;
	if (location == 0){
		location = 1;
		button_queue_1 = 0;
		kill(button_1, SIGUSR2);
		kill(cabin_button, SIGUSR2);
		puts("Elevator is on the 1 floor");
	}else{
		location = 0;
		button_queue_0 = 0;
		kill(button_0, SIGUSR2);
		kill(cabin_button, SIGUSR2);
		puts("Elevator is on the 0 floor");
	}
	door_status = 1;
	puts("Doors are open");
	button_queue_cab = 0;
	queue();
	alarm(time);
	return;
}

int main(void){
	read(0, &button_0, sizeof(pid_t));
	read(0, &button_1, sizeof(pid_t));
	read(0, &cabin_button, sizeof(pid_t));
	read(0, &lift, sizeof(pid_t));
	signal(SIGINT, handler_sigint);
	signal(SIGTERM, handler_sigterm);
	signal(SIGUSR1, handler_button_0);
	signal(SIGUSR2, handler_button_1);
	signal(SIGFPE, handler_cabin_button);
	signal(SIGCHLD, handler_lift);
	signal(SIGALRM, handler_alarm);

	while(1){
		pause();
	}
	return 0;
}