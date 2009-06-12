#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#ifndef SOCK_NAME
#define SOCK_NAME    "/dev/ttyS0"
#endif

int
main(int argc, char *argv[])
{
	struct termios		saved_termios_stdin, termios_stdin;
	struct termios		saved_termios_pipe, termios_pipe;
	struct sockaddr_un	addr;
	int    fd;
	char   buf[1024];
	fd_set  fds;
	int	nlen;
	char *	sock_name;

	if (argc == 2) {
	    sock_name = argv[1];
	}
	else {
	    sock_name = SOCK_NAME;
	}

	if (tcgetattr(0, &termios_stdin) < 0) {
		perror("tcgetattr");
		exit(1);
	}
	saved_termios_stdin = termios_stdin;
	termios_stdin.c_lflag &= (~ECHO);
	if (tcsetattr(0, TCSANOW, &termios_stdin) < 0) {
		perror("tcsetattr");
		exit(1);
	}

start:

	/*
	 * Make a socket of UNIX domain stream type socket.
	 */
	if ((fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		exit(1);
	}

	/* 
	 * Make sure addr is clean.
	 */
	bzero((char *)&addr, sizeof(addr));

	/*
	 * Copy the name of socket.
	 */
	addr.sun_family = AF_UNIX;

	if (argc == 2) {
		printf("Using %s\n", argv[1]);
		strcpy(addr.sun_path, argv[1]);
	} else {
		printf("Using %s\n", sock_name);
		strcpy(addr.sun_path, sock_name);
	}

	/*
	 *  Try to connect to the server.
	 */
	printf("Wait until %s is available...", addr.sun_path);
	fflush(stdout);
	while (connect(fd, (struct sockaddr *)&addr,
		    sizeof(addr.sun_family) + strlen(sock_name)) < 0) {
		sleep(1);
	}
	printf("connected.\n");

	if (tcgetattr(0, &termios_pipe) < 0) {
		perror("tcgetattr");
		exit(1);
	}
	saved_termios_pipe = termios_pipe;
	termios_pipe.c_lflag &= (~ICANON);
	termios_pipe.c_cc[VTIME] = 0;
	termios_pipe.c_cc[VMIN] = 1;
	if (tcsetattr(0, TCSANOW, &termios_pipe) < 0) {
		perror("tcsetattr");
		exit(1);
	}

	/*
	 * Do I/O.
	 */
	for (;;) {
		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(fd, &fds);
		select(fd + 1, &fds, NULL, NULL, NULL);

		if (FD_ISSET(0, &fds)) {
			nlen = read(0, buf, 1024);
			send(fd, buf, nlen, 0);
		}
		if (FD_ISSET(fd, &fds)) {
			nlen = recv(fd, buf, 1024, 0);
			if(nlen == 0) {
				/* Remote Server closed the connection. */
				printf("connection closed.\n");
				break;
			}
			write(1, buf, nlen);
		}
	}

	close(fd);

	goto start;
}
