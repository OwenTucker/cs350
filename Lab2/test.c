#include "inodemap.h"
#include "inodemap.c"
#include <stdio.h>
#include <stdlib.h>
#include "getopt.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

int find_links(char* dir_name, char* file_name) {
	static int w = 0;
	struct stat buf, tuf;
	struct dirent* de;
	int exists, e;
	DIR* d;
	d = opendir(dir_name);
	

	for (de = readdir(d); de != NULL; de = readdir(d)) {
		char path[1024];
		char filepath[1024];
		sprintf(path, "%s/%s", dir_name, de->d_name);
		while (w != 0) {
			strcat(filepath, "/../");
			w--;
		}
		sprintf(filepath, "%s/%s", dir_name, file_name);
		e = stat(filepath, &tuf);
		exists = stat(path, &buf);
		//printf("Current path %s, current inode: %ld\n", path, buf.st_ino);
		if ((exists) < 0) {
			fprintf(stderr, "%s not found\n", path);
			perror("stat()");
		}
		else if (buf.st_ino == tuf.st_ino) printf("%s is a hard link to %s\n", de->d_name, file_name);
		else if (S_ISLNK(buf.st_mode)) {
			printf("%s is a symbolic link to %s\n", de->d_name, file_name);
		}
		if (S_ISDIR(buf.st_mode) && strcmp(de->d_name, ".") != 0 && strcmp(de->d_name, "..") != 0) {
			w++;
			find_links(path, file_name);
		}
	}
	return 1; 
}

int main(int argc, char* argv[]) {
	char* dir_name = argv[1]; 
	char* file_name = argv[2];
	find_links(dir_name, file_name);
	
}