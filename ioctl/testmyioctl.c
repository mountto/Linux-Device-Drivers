/***************************************************************************//**
*  \file       testmyioctl.c
*
*  \details    Userspace application to test the Device driver
*
*  \author     Murt Meiv
*  \date	Thu 4, ngay 7 thang 7 nam 2021
*  \Tested with Linux murt-VPCEH28FJ 5.8.0-59-generic #66~20.04.1-Ubuntu SMP Thu Jun 17 11:14:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
 
#define WR_VALUE _IOW('a','a',int32_t*)
#define RD_VALUE _IOR('a','b',int32_t*)
 
int main()
{
        int fd;
        int32_t value, number;
        printf("*********************************\n");
        printf("*******xxxxxxxxxxxxxxxxxxx*******\n");
 
        printf("\nOpening Driver\n");
        fd = open("/dev/ioctl", O_RDWR);
        if(fd < 0) {
                printf("Cannot open device file...\n");
                return 0;
        }
 
        printf("Enter the Value to send\n");
        scanf("%d",&number);
        printf("Writing Value to Driver\n");
        ioctl(fd, WR_VALUE, (int32_t*) &number); 
 
        printf("\tReading Value from Driver\n");
        ioctl(fd, RD_VALUE, (int32_t*) &value);
        printf("Value of Driver sent to user: %d\n", value);
 
        printf("Closing Driver\n");
        close(fd);
}
