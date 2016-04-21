/*
 * Stealthily detect if we are observed from the outside.
 * Inspired by http://www.manoharvanga.com/hackme/
 *
 * Works on Linux, on FreeBSD does not.
 *
 * Compile:
 *   gcc prevent.c -ldl
 *
 * Run:
 *   ./a.out
 *   strace ./a.out
 *   ltrace ./a.out
 *   echo "r" | gdb ./a.out
 */

#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/ptrace.h>

/* obfuscated "ptrace" string so that it does not show up in strings(1) */
char pstr[6] = { 0x25, 0x21, 0x27, 0x34, 0x36, 0x30 };

int
main(void)
{
	int i;
	void *dl = dlopen("/usr/lib/libc.so", RTLD_NOW);
	long (*myptrace)(int, int, int, int);
	int request;

	for (i = 0; i < sizeof (pstr); i++)
		pstr[i] = 0x55 ^ pstr[i];

	if ((myptrace = dlsym(dl, pstr)) == NULL)
		return (0);

#if (defined(__APPLE__) && defined(__MACH__)) || defined(__FreeBSD__)
	request = PT_TRACE_ME;
#else
	request = PTRACE_TRACEME;
#endif

	/* Detect we are running under debugger/strace/ltrace. */
	if (myptrace(request, 0, 0, 0) == -1) {
		printf("Traced !\n");
		return (1);
	} else {
		printf("Everything is safe\n");
	}

	sleep(1);

	return (0);
}
