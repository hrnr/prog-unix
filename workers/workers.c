#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <err.h>
#include <string.h>
#include "arg.h"

size_t thread_count = 0;
pthread_mutex_t thread_count_m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t thread_count_c = PTHREAD_COND_INITIALIZER;

void * worker(void *data) {
	sleep(rand() % 10);

	pthread_mutex_lock(&thread_count_m);
	--thread_count;
	pthread_mutex_unlock(&thread_count_m);
	pthread_cond_signal(&thread_count_c);

	printf("thread ended\n");

	return (NULL);
}

void respawn_workers(size_t worker_count) {
	pthread_t thread;
	int error;

	for(;;) {
		pthread_mutex_lock(&thread_count_m);
		while(!(thread_count != worker_count)) {
			pthread_cond_wait(&thread_count_c, &thread_count_m);
		}

		error = pthread_create(&thread, NULL, worker, NULL);
		if (error != 0) {
			/* strerror is not reentrant */
			warnx("pthread_create: %s", strerror(error));
			continue;
		}
		pthread_detach(thread);
		++thread_count;
		printf("thread created\n");

		pthread_mutex_unlock(&thread_count_m);
	}
}

void
usage(char *argv0)
{
 fprintf(stderr,
     "usage: %s command [-n num_workers]\n",
     argv0);

 exit(1);
}


int main(int argc, char *argv[]) {
	size_t num_workers = 0;
	char *argv0;

	/* command line args */
	ARGBEGIN {
	case 'n':
		/* strtoll + arr handling */
		num_workers = atoll(EARGF(usage(argv0)));
		break;
	default:
		usage(argv0);
	} ARGEND;

	printf("will setup %llu workers\n", num_workers);

	respawn_workers(num_workers);

	return (EXIT_SUCCESS);
}