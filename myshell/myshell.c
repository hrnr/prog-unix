#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "parse.h"

// closes fd, if it is not "standard" fd - stdin, stderr, stdout
int close_nonsd(int fd) {
	if(fd != 0 && fd != 1 && fd != 2) {
		return (close(fd));
	}
	return (0);
}

int exec_cmds(struct command *cmd) {
	int read = 0; /* stdin */
	int pipefd[2] = { 0, 1 };
	pid_t pid = -1;

	for (struct command *it = cmd; it != NULL; it = it->next) {
		if(it->next) {
			if(pipe(pipefd) == -1) {
				return (-1);
			}
			it->fd[1] = pipefd[1];
		} else {
			it->fd[1] = 1; /* stdout */
		}
		it->fd[0] = read;
		read = pipefd[0];

		switch(pid = fork()) {
			case -1:
				perror("couldn't fork");
				return (-1);
				break;
			case 0:
				/* in child */
				// printf("%s in:%d out:%d\n", it->argv[0], it->fd[0], it->fd[1]);
				// TODO: dup err handling
				dup2(it->fd[0], 0); /* stdin */
				dup2(it->fd[1], 1); /* stdout */

				close_nonsd(it->fd[0]);
				close_nonsd(it->fd[1]);
				close_nonsd(read);
				execvp(it->argv[0], it->argv);
				break;
			default:
				/* in parent */
				close_nonsd(it->fd[0]);
				close_nonsd(it->fd[1]);
				break;
		}
	}
	close_nonsd(read);

	/* join last command in batch */
	int status;
	if(pid != -1) {
		waitpid(pid, &status, 0);
	}
	
	return (0);
}

int main(int argc, char **argv) {
	char *line;
	struct command *cmd;
	int quit = 0;

	while (!quit) {
		line = readline("myshell> ");
		if (!line)
			return (0);
		add_history(line);
		cmd = parse_line(line);
		if (exec_cmds(cmd)) {
			fprintf(stderr, "Line execution failed.\n");
		}
		free_cmds(cmd);
		free(line);
	}

	return (0);
}
