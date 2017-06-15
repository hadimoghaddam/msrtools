// Hadi Asghari-Moghaddam asghari2@illinois.edu
// University of Illinois at Urbana-Champaign
// Please don't distribute the code

#include "msrdrv.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */


static int loadDriver()
{
    int fd;
    fd = open("/dev/" DEV_NAME, O_RDWR);
    if (fd == -1) {
        perror("Failed to open /dev/" DEV_NAME);
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

/*
 * Reference:
 * Intel Software Developer's Manual Vol 3B "253669.pdf" August 2012
 * Intel Software Developer's Manual Vol 3C "326019.pdf" August 2012
 */

/*timespec diff(timespec start, timespec end)
{
	timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}
*/
int main(void)
{
    int i;
    int fd;
//    timespec start, end;
//    int timeElapsed;

    FILE *file;
    //file = fopen("output_msr.txt","w");
    struct MsrInOut msr_start[] = {
        { MSR_WRITE, 0x38f, 0x00, 0x00 },       // ia32_perf_global_ctrl: disable 4 PMCs & 3 FFCs
        { MSR_WRITE, 0xc1, 0x00, 0x00 },        // ia32_pmc0: zero value (35-5)
        { MSR_WRITE, 0xc2, 0x00, 0x00 },        // ia32_pmc1: zero value (35-5)
        { MSR_WRITE, 0xc3, 0x00, 0x00 },        // ia32_pmc2: zero value (35-5)
        { MSR_WRITE, 0xc4, 0x00, 0x00 },        // ia32_pmc3: zero value (35-5)
        { MSR_WRITE, 0x309, 0x00, 0x00 },       // ia32_fixed_ctr0: zero value (35-17)
        { MSR_WRITE, 0x30a, 0x00, 0x00 },       // ia32_fixed_ctr1: zero value (35-17)
        { MSR_WRITE, 0x30b, 0x00, 0x00 },       // ia32_fixed_ctr2: zero value (35-17)
        { MSR_WRITE, 0x186, 0x004101D3, 0x00 }, // ia32_perfevtsel0, MEM_LOAD_UOPS_L3_MISS_RETIRED.LOCAL_DRAM (19-28)
        { MSR_WRITE, 0x187, 0x00410280, 0x00 }, // ia32_perfevtsel1, ICACHE.MISSES (19.22)
//        { MSR_WRITE, 0x187, 0x0041024E, 0x00 }, // ia32_perfevtsel1,  L1D_PREFETCH.MISS (19-22)
        { MSR_WRITE, 0x188, 0x00410185, 0x00 }, // ia32_perfevtsel2, ITLB_MISSES.MISS_CAUSES_A_WALK (19-22)
        { MSR_WRITE, 0x189, 0x004101a2, 0x00 }, // ia32_perfevtsel3, RESOURCE_STALLS.ANY (19-27)
        { MSR_WRITE, 0x38d, 0x222, 0x00 },      // ia32_perf_fixed_ctr_ctrl: ensure 3 FFCs enabled
        { MSR_WRITE, 0x38f, 0x0f, 0x07 },       // ia32_perf_global_ctrl: enable 4 PMCs & 3 FFCs
        { MSR_STOP, 0x00, 0x00 }
    };

    struct MsrInOut msr_stop[] = {
        { MSR_WRITE, 0x38f, 0x00, 0x00 },       // ia32_perf_global_ctrl: disable 4 PMCs & 3 FFCs
        { MSR_WRITE, 0x38d, 0x00, 0x00 },       // ia32_perf_fixed_ctr_ctrl: clean up FFC ctrls
        { MSR_READ, 0xc1, 0x00 },               // ia32_pmc0: read value (35-5)
        { MSR_READ, 0xc2, 0x00 },               // ia32_pmc1: read value (35-5)
        { MSR_READ, 0xc3, 0x00 },               // ia32_pmc2: read value (35-5)
        { MSR_READ, 0xc4, 0x00 },               // ia32_pmc3: read value (35-5)
        { MSR_READ, 0x309, 0x00 },              // ia32_fixed_ctr0: read value (35-17)
        { MSR_READ, 0x30a, 0x00 },              // ia32_fixed_ctr1: read value (35-17)
        { MSR_READ, 0x30b, 0x00 },              // ia32_fixed_ctr2: read value (35-17)
        { MSR_READ, 0x606, 0x00 },              // MSR_RAPL_POWER_UNIT
        { MSR_READ, 0x610, 0x00 },              // MSR_PKG_POWER_LIMIT
        { MSR_READ, 0x611, 0x00 },              // MSR_PKG_ENERGY_STATUS
        { MSR_READ, 0x614, 0x00 },              // MSR_PKG_POWER_INFO
        { MSR_STOP, 0x00, 0x00 }
    };


    fd = loadDriver();
    for (i=0;i<10000;i++){
       //printf("--------------------------------------------\n");
//       clock_gettime(CLOCK_MONOTONIC, &start);
       ioctl(fd, IOCTL_MSR_CMDS, (long long)msr_start);
//       clock_gettime(CLOCK_MONOTONIC, &end);
//       timeElapsed = timespecDiff(&end, &start);
//       printf ("It took %d seconds for start.\n",timeElapsed);
       usleep(10000); //sleep in miliseconds
//       clock_gettime(CLOCK_MONOTONIC, &start);
       ioctl(fd, IOCTL_MSR_CMDS, (long long)msr_stop);
//       clock_gettime(CLOCK_MONOTONIC, &end);

//       timeElapsed = timespecDiff(&end, &start);
//       printf ("It took %d seconds for stop.\n",timeElapsed);
       //
       fprintf(stdout,"--------------------------------------------\n");
       fprintf(stdout,"L3 miss:         %7lld\n", msr_stop[2].value);
       fprintf(stdout,"iCache miss:     %7lld\n", msr_stop[3].value);
//       fprintf(file,"DCache miss:     %7lld\n", msr_stop[3].value);
       fprintf(stdout,"iTLB miss:       %7lld\n", msr_stop[4].value);
       fprintf(stdout,"resource stalls: %7lld\n", msr_stop[5].value);
       fprintf(stdout,"instr retired:   %7lld\n", msr_stop[6].value);
       fprintf(stdout,"core cycles:     %7lld\n", msr_stop[7].value);
       fprintf(stdout,"ref cycles:      %7lld\n", msr_stop[8].value);
       fprintf(stdout,"RAPL Power:      %7lld\n", msr_stop[9].value);
       fprintf(stdout,"Power limit:     %7lld\n", msr_stop[10].value);
       fprintf(stdout,"Energy status:   %7lld\n", msr_stop[11].value);
       fprintf(stdout,"Power info:      %7lld\n", msr_stop[12].value);
//       printf("uops retired:    %7lld\n", msr_stop[2].value);
//      printf("uops issued:     %7lld\n", msr_stop[3].value);
//       printf("stalled cycles:  %7lld\n", msr_stop[4].value);
//       printf("resource stalls: %7lld\n", msr_stop[5].value);
//       printf("instr retired:   %7lld\n", msr_stop[6].value);
//       printf("core cycles:     %7lld\n", msr_stop[7].value);
//       printf("ref cycles:      %7lld\n", msr_stop[8].value);

    }
    closeDriver(fd);
    fclose(file);
    return 0;
}
