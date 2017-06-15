/* Hadi has changed this code to meet his goals. */

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



int SampleNumbers = 200000; //total number of samples
int samplingInterval = 100; //sleep time, default 1ms


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

int main(int argc, char *argv[])
{
    uint64_t data;
//	uint32_t reg;
	int cpu = 0;
    int i;
 
	char buffer[50];

	unsigned long long * freqs = (unsigned long long *) malloc(SampleNumbers*sizeof(unsigned long long));
	unsigned long long * energy = (unsigned long long *) malloc(SampleNumbers*sizeof(unsigned long long));

    sprintf(buffer,"%s.txt",argv[1]);
    FILE *out_file;
    out_file = fopen(buffer,"w");
       
    int fd = loadDriver(cpu);


	for (i=0;i<SampleNumbers;i++){
		usleep(samplingInterval); //sleep 1 milisecond
		data = rdmsr_on_cpu(0x198,fd);
		freqs[i] = data & 0xFFFF;
		data = rdmsr_on_cpu(0x611,fd);
		energy[i] = data; //- prev_energy;
    }

    for(i=0;i<SampleNumbers;i++){
      fprintf(out_file,"%7lld  ", freqs[i]);
      fprintf(out_file,"%7lld  \n", energy[i]);
    }

/*  reg = 0x611;
	data = rdmsr_on_cpu(reg,fd);
    printf("%llu\n",data);
*/
    closeDriver(fd);
    fclose(out_file);

    free(freqs);
	free(energy);
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
