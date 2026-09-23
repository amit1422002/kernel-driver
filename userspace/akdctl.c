/*
 * akdctl - userspace test tool for /dev/akd
 *
 * Build (host, for a matching ABI):
 *   aarch64-linux-gnu-gcc -O2 -Wall -I../kernel -o akdctl akdctl.c
 *
 * Usage:
 *   akdctl version
 *   akdctl status
 *   akdctl set-status <0|1|2>
 *   akdctl write <string>
 *   akdctl read
 *   akdctl clear
 *   akdctl len
 */

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "../kernel/akd.h"

static int open_dev(int flags)
{
	int fd = open(AKD_DEVICE_PATH, flags);
	if (fd < 0) {
		fprintf(stderr, "open %s: %s\n", AKD_DEVICE_PATH, strerror(errno));
		exit(1);
	}
	return fd;
}

static void usage(const char *argv0)
{
	fprintf(stderr,
		"Usage: %s <command> [args]\n"
		"  version\n"
		"  status\n"
		"  set-status <0|1|2>   idle/busy/error\n"
		"  write <string>\n"
		"  read\n"
		"  clear\n"
		"  len\n",
		argv0);
}

static const char *status_name(uint32_t s)
{
	switch (s) {
	case AKD_STATUS_IDLE:
		return "idle";
	case AKD_STATUS_BUSY:
		return "busy";
	case AKD_STATUS_ERROR:
		return "error";
	default:
		return "unknown";
	}
}

int main(int argc, char **argv)
{
	int fd;
	uint32_t val;
	ssize_t n;
	char buf[AKD_BUF_SIZE + 1];

	if (argc < 2) {
		usage(argv[0]);
		return 1;
	}

	if (strcmp(argv[1], "version") == 0) {
		fd = open_dev(O_RDONLY);
		if (ioctl(fd, AKD_IOC_GET_VERSION, &val) < 0) {
			perror("ioctl GET_VERSION");
			return 1;
		}
		printf("akd version %u.%u.%u (0x%08x)\n",
		       (val >> 16) & 0xff, (val >> 8) & 0xff, val & 0xff, val);
		close(fd);
		return 0;
	}

	if (strcmp(argv[1], "status") == 0) {
		fd = open_dev(O_RDONLY);
		if (ioctl(fd, AKD_IOC_GET_STATUS, &val) < 0) {
			perror("ioctl GET_STATUS");
			return 1;
		}
		printf("status %u (%s)\n", val, status_name(val));
		close(fd);
		return 0;
	}

	if (strcmp(argv[1], "set-status") == 0) {
		if (argc < 3) {
			usage(argv[0]);
			return 1;
		}
		val = (uint32_t)strtoul(argv[2], NULL, 0);
		fd = open_dev(O_RDWR);
		if (ioctl(fd, AKD_IOC_SET_STATUS, &val) < 0) {
			perror("ioctl SET_STATUS");
			return 1;
		}
		printf("status set to %u (%s)\n", val, status_name(val));
		close(fd);
		return 0;
	}

	if (strcmp(argv[1], "write") == 0) {
		if (argc < 3) {
			usage(argv[0]);
			return 1;
		}
		fd = open_dev(O_WRONLY);
		n = write(fd, argv[2], strlen(argv[2]));
		if (n < 0) {
			perror("write");
			return 1;
		}
		printf("wrote %zd bytes\n", n);
		close(fd);
		return 0;
	}

	if (strcmp(argv[1], "read") == 0) {
		fd = open_dev(O_RDONLY);
		n = read(fd, buf, AKD_BUF_SIZE);
		if (n < 0) {
			perror("read");
			return 1;
		}
		buf[n] = '\0';
		printf("read %zd bytes: %s\n", n, buf);
		close(fd);
		return 0;
	}

	if (strcmp(argv[1], "clear") == 0) {
		fd = open_dev(O_RDWR);
		if (ioctl(fd, AKD_IOC_CLEAR) < 0) {
			perror("ioctl CLEAR");
			return 1;
		}
		printf("cleared\n");
		close(fd);
		return 0;
	}

	if (strcmp(argv[1], "len") == 0) {
		fd = open_dev(O_RDONLY);
		if (ioctl(fd, AKD_IOC_GET_LEN, &val) < 0) {
			perror("ioctl GET_LEN");
			return 1;
		}
		printf("len %u\n", val);
		close(fd);
		return 0;
	}

	usage(argv[0]);
	return 1;
}
