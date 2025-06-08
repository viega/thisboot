#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>

#if defined(__linux__)
#include <sys/sysinfo.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#else
#error "Unsupported platform"
#endif

#define output(s, ...) \
    if (!quiet) {\
	fprintf(stderr, s __VA_OPT__(,) __VA_ARGS__);	\
    }


int
main(int argc, char *argv[], char *envp[])
{
    int file_arg = 1;
    int quiet    = 0;

    if (argc > 1 && !strcmp(argv[1], "-q")) {
	quiet= 1;
	file_arg++;
    }

    if (--argc != file_arg) {
	output("Must specify one file to check timestamp on.\n");
	exit(-1);
    }

    struct stat     finfo;
    struct timespec now;
    struct timespec boot;

    if (stat(argv[file_arg], &finfo)) {
	output("Could not stat %s\n", argv[file_arg]);
	exit(-2);
    }


#if defined(__linux__)
    struct sysinfo  si;

    clock_gettime(CLOCK_REALTIME, &boot);
    sysinfo(&si);
    boot.tv_sec -= si.uptime;

#elif defined(__APPLE__)
    size_t l = sizeof(struct timespec);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    sysctl(mib, 2, &boot, &l, NULL, 0);
#endif


    // Second resolution only; st_mtim isn't portable.


    if (finfo.st_mtime >= boot.tv_sec) {
	output("%s is current (written post-boot).\n", argv[1]);
	exit(0);
    }
    else {
	output("%s has not been modified since last boot.\n", argv[1]);
	exit(1);
    }

}
