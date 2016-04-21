/*
 * Skeleton of strace program.
 */

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>

int
wait_for_syscall(pid_t child_pid) {
	int status;
	while(1) {
		ptrace(PTRACE_SYSCALL, child_pid, 0, 0); // start again child with tracing
		waitpid(child_pid, &status, 0);
		if (WIFSTOPPED(status) && WSTOPSIG(status) & 0x80) {
        	return 0;
		}
		if (WIFEXITED(status)) {
			// child has exited or terminated
			return 1;
		}
	}
}

int
tracer(pid_t child_pid) {
	int status;
	struct user_regs_struct regs;
	// wait for child to call TRACEME
	waitpid(child_pid, &status, 0);
	// to distinguis syscalls from others
	ptrace(PTRACE_SETOPTIONS, child_pid, 0, PTRACE_O_TRACESYSGOOD);
	printf("child should be resumed again with pid %d\n", child_pid);
	while(1) {
        if (wait_for_syscall(child_pid) != 0) break;
        ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);
        fprintf(stderr, "syscall(%llu) = ", regs.orig_rax);
        if (wait_for_syscall(child_pid) != 0) break;
        ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);
        fprintf(stderr, "%lld\n", regs.rax);
    }
    return (0);
}

int
main(int argc, char *argv[])
{
	pid_t pid;
	char *filename;
	int i;

	if (argc == 1)
		errx(1, "usage: %s <program> [arg1 .. argn]", argv[0]);
	argv++;
	argc--;
	filename = argv[0];

	switch (pid = fork()) {
		case -1:
			err(1, "fork");
			break;
		case 0:
			printf("execve\n");
			fflush(stdout);
			ptrace(PTRACE_TRACEME, 0, NULL, NULL);
			kill(getpid(), SIGSTOP);
			return execve(filename, argv, NULL);
		default:
			;
	}

	return tracer(pid);
}
