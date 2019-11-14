#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

static pid_t button_0, button_1, cabin_button, elevator, lift;

void handler_sigterm(int s){
	if (button_0 > 0) kill(button_0, SIGTERM);
	if (button_1 > 0) kill(button_1 ,SIGTERM);
	if (cabin_button > 0) kill(cabin_button, SIGTERM);
	if (elevator > 0) kill(elevator, SIGTERM);
	if (lift > 0) kill(lift, SIGTERM);
	while (wait(NULL) != -1){
	}
	exit(0);
	return;
}

void handler_sigint(int s){
	signal(SIGINT, handler_sigint);
	return;
}

void check_pipe(int fd[2]){
	if (pipe(fd) == -1){
		write(2, strerror(errno), strlen(strerror(errno)));
		exit(1);
	}
}

int main(void){
	int fd1[2], fd2[2], fd3[2], fd4[2], fd5[2];
	check_pipe(fd1);
	check_pipe(fd2);
	check_pipe(fd3);
	check_pipe(fd4);
	check_pipe(fd5);

	if ((button_0 = fork()) == 0){
		dup2(fd1[0], 0);
		close(fd1[0]);
		close(fd1[1]);
		execl("button_0", "button_0", NULL);
		write(2, strerror(errno), strlen(strerror(errno)));
		kill(getppid(), SIGTERM);
		return 1;
	}

	if ((button_1 = fork()) == 0){
		dup2(fd2[0], 0);
		close(fd2[0]);
		close(fd2[1]);
		execl("button_1", "button_1", NULL);
		write(2, strerror(errno), strlen(strerror(errno)));
		kill(getppid(), SIGTERM);
		return 1;
	}

	if ((cabin_button = fork()) == 0){
		dup2(fd3[0], 0);
		close(fd3[0]);
		close(fd3[1]);
		execl("cabin_button", "cabin_button", NULL);
		write(2, strerror(errno), strlen(strerror(errno)));
		kill(getppid(), SIGTERM);
		return 1;
	}

	if ((elevator = fork()) == 0){
		dup2(fd4[0], 0);
		close(fd4[0]);
		close(fd4[1]);
		execl("elevator", "elevator", NULL);
		write(2, strerror(errno), strlen(strerror(errno)));
		kill(getppid(), SIGTERM);
		return 1;
	}

	if ((lift = fork()) == 0){
		dup2(fd5[0], 0);
		close(fd5[0]);
		close(fd5[1]);
		execl("lift", "lift", NULL);
		write(2, strerror(errno), strlen(strerror(errno)));
		kill(getppid(), SIGTERM);
		return 1;
	}

	write(fd1[1], &elevator, sizeof(pid_t));

	write(fd2[1], &elevator, sizeof(pid_t));

	write(fd3[1], &elevator, sizeof(pid_t));

	write(fd4[1], &button_0, sizeof(pid_t));
	write(fd4[1], &button_1, sizeof(pid_t));
	write(fd4[1], &cabin_button, sizeof(pid_t));
	write(fd4[1], &lift, sizeof(pid_t));

	write(fd5[1], &elevator, sizeof(pid_t));

	signal(SIGTERM, handler_sigterm);
	signal(SIGINT, handler_sigint);

	puts("Press 0 to call the elevator to floor 0");
	puts("Press 1 to call the elevator to floor 1");
	puts("Press 2 to go the elevator");
	puts("Press 3 to exit the program");
	puts("The elevator is on the 1 floor");
	puts("The doors are closed");

	while(1){
		int button = getchar();
		if (button == '0'){
			kill(button_0, SIGUSR1);
			continue;
		}
		if (button == '1'){
			kill(button_1, SIGUSR1);
			continue;
		}
		if (button == '2'){
			kill(cabin_button, SIGUSR1);
			continue;
		}
		if (button == '3'){
			raise(SIGTERM);
			continue;
		}
	}
	return 0;
}