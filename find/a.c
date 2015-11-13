#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

char *join_dir(const char *dir, const char *path) {
	size_t len = strlen(dir) + strlen(path) + 1;
	full_path = malloc(len);
	snprintf(&full_path, "%s/%s", dir, path);

	return full_path;
}

void list_dir(const char *dir) {
	DIR *dp;
	struct dirent *ep;
	struct stat statbuf;


	if(!(dp = opendir(dir))) {
		goto errors;
	}

	while (ep = readdir(dp)) {
		char *full_path = join_dir()
		

		printf("ep->d_name: %s\n", ep->d_name);

		if (stat(ep->d_name, &statbuf) == -1) {
			continue;
		}

		if (S_ISDIR(statbuf.st_mode)) {
			list_dir(strcpy(dp->d_name));
		}

		if(S_ISREG(statbuf.st_mode)) {
			puts(full_path);
		}
	}

	if(closedir (dp)) {
		goto errors;
	}

	return;

	errors:
	perror ("Couldn't read the directory");
}

int main(int argc, char const *argv[]) {
	if(argc>1) {
		list_dir(argv[1]);
	}
	return (0);
}