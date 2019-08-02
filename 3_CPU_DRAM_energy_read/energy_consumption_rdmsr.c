/* Hadi Asghari-Moghaddam
 * asghari2@illinois.edu
 * Summer 2019
 *
 * Do not distribute the code
 */

/* Copyright for msr-tools*/
/* ----------------------------------------------------------------------- *
 *
 *   Copyright 2000 Transmeta Corporation - All Rights Reserved
 *   Copyright 2004-2008 H. Peter Anvin - All Rights Reserved
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 *   Boston MA 02110-1301, USA; either version 2 of the License, or
 *   (at your option) any later version; incorporated herein by reference.
 *
 * ----------------------------------------------------------------------- */

/*
 * rdmsr.c
 *
 * Utility to read an MSR.
 */

/* How to use this code:
 * 1- $ sudo modprobe msr
 *			runs the msr driver in Linux
 * 2- $ g++ energy_consumption_rdmsr.c -o energy_consumption_rdmsr
 *			compiles the C code
 * 3- $ sudo ./energy_consumption_rdmsr
 *			runs the code
 * 4- When you are done with measurement press "return" key and it will terminate
 *			terminates the code
 * 5- reports CPU and DRAM energy consumption and runtime
 * 6- $ sudo rmmod msr
 *			removes the driver
 */


#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <inttypes.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <math.h>

#define BILLION 1000000000L

static int loadDriver(int cpu)
{
	int fd;
	char msr_file_name[64];

	sprintf(msr_file_name, "/dev/cpu/%d/msr", cpu);
	fd = open(msr_file_name, O_RDONLY);
	if (fd < 0) {
		if (errno == ENXIO) {
			fprintf(stderr, "rdmsr: No CPU %d\n", cpu);
			exit(2);
		} else if (errno == EIO) {
			fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n",
				cpu);
			exit(3);
		} else {
			perror("rdmsr: open");
			exit(127);
		}
	}
    return fd;
}

static void closeDriver(int fd)
{
	int e;
	e = close(fd);
	if (e == -1) {
		perror("Failed to close fd");
	}
}

uint64_t rdmsr_on_cpu(uint32_t reg, int fd);

int main(int argc, char *argv[])
{

	struct timespec start_time, end_time;

	int cpu = 0;

	uint64_t start_energy_cpu = 0 , end_energy_cpu= 0,
					 start_energy_dram = 0, end_energy_dram = 0;
	uint64_t energy_scale;

	int fd = loadDriver(cpu);

	// Find the energy scaling factor
	//  On highwind it is 0b1110 = 14. So the value obtained from 0x611 and 0x619
	//  should be divided by 2^14 to get the energy consumption Joules
	energy_scale = rdmsr_on_cpu(0x606,fd);
	energy_scale = (energy_scale & 0x00000F00) >> 8;

	// Read current (start) counter values for CPU and DRAM energy counter
	start_energy_cpu = rdmsr_on_cpu(0x611,fd);
	start_energy_dram = rdmsr_on_cpu(0x619,fd);
	// Find start time
	clock_gettime(CLOCK_MONOTONIC, & start_time);

	// Wait to receive "return"
	//	The purpose is to wait until some one presses the return button
	int a = getc(stdin);

	// Read current RAPL counter values; The start_energy_xxx should be
	//  deducted from this to get the energy consumption between two points
	end_energy_cpu = rdmsr_on_cpu(0x611,fd);
	end_energy_dram = rdmsr_on_cpu(0x619,fd);
	// Find end time
	clock_gettime(CLOCK_MONOTONIC, & end_time);

	// Find total runtime
	uint64_t time =  BILLION * (end_time.tv_sec - start_time.tv_sec) +
		(end_time.tv_nsec - start_time.tv_nsec);

	// Print out the result
	printf("Time: %f, CPU Energy: %f\nDRAM Energy: %f\n",
			(double) time / BILLION,
			((double)end_energy_cpu-start_energy_cpu)/ pow(2,energy_scale),
			((double)end_energy_dram-start_energy_dram)/ pow(2,energy_scale));

	closeDriver(fd);
  exit(0);
}

uint64_t rdmsr_on_cpu(uint32_t reg, int fd)
{
	uint64_t data;
	int cpu = 0;
	if (pread(fd, &data, sizeof data, reg) != sizeof data) {
		if (errno == EIO) {
			fprintf(stderr, "rdmsr: CPU %d cannot read "
				"MSR 0x%08"PRIx32"\n",
				cpu, reg);
			exit(4);
		} else {
			perror("rdmsr: pread");
			exit(127);
		}
	}
	return data;
}


/* filter out ".", "..", "microcode" in /dev/cpu */

/*
int dir_filter(const struct dirent *dirp) {
	if (isdigit(dirp->d_name[0]))
		return 1;
	else
		return 0;
}


void rdmsr_on_all_cpus(uint32_t reg)
{
	struct dirent **namelist;
	int dir_entries;

	dir_entries = scandir("/dev/cpu", &namelist, dir_filter, 0);
	while (dir_entries--) {
		rdmsr_on_cpu(reg, atoi(namelist[dir_entries]->d_name));
		free(namelist[dir_entries]);
	}
	free(namelist);
}
*/


