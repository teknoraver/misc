#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define BLOCK 65536

/*
 * findnull - find files with all content of binary 0
 * example usage: find / -xdev -type f -print0 |xargs -0 ./findnull
 */

void findnull(char *path)
{
	int i;
	char *file;
	struct stat sb;
	int zero = 1;

	int fd = open(path, O_RDONLY);
	fstat(fd, &sb);
	if(!sb.st_size) {
		close(fd);
		return;
	}

	file = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
	for(i = 0; i < sb.st_size; i++)
		if(file[i]) {
			zero = 0;
			break;
		}
	munmap(file, sb.st_size);
	close(fd);
	if(zero)
		printf("%s: 0\n", path);
}

int main(int argc, char *argv[])
{
	int i;
	for(i = 1; i < argc; i++)
		findnull(argv[i]);

	return 0;
}
