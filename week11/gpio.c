#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    unsigned int data;
    int fd = -1;
    int count = 0;

    fd = open("/dev/sensor", O_RDWR);

    if (0 >= fd)
    {
        printf("Open fail!\n");
        exit(0);
    }

    while (count < 5) {
        ioctl(fd, NULL, &data);
        printf("distance is %dcm\n", data);
        count++;
        sleep(2);
    }

    return 0;
}