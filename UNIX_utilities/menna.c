#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define COUNT 100
#define READ_ERROR -1
#define OPEN_FAIL -2
#define WRONG_INPUT -3

int main(int argc, char **argv)
{
    int fd1, fd2;
    char buff[COUNT];
    int count_r, count_w;

    if (argc != 3)
    {
        printf("Not enough arguments given\n");
        exit(WRONG_INPUT);
    }
    
    if ((fd1 = open(argv[1], O_RDONLY)) < 0)
    {
        printf("file <%s> failed to open\n", argv[1]);
        exit(OPEN_FAIL);
    }

    if ((fd2 = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 644)) < 0)
    {
        printf("file <%s> failed to open\n", argv[2]);
        exit(OPEN_FAIL);
    }
    while((count_r = read(fd1, buff, COUNT)) > 0)
    {
        if ((count_w = write(fd2, buff, count_r)) < 0)
        {
            
            printf("Writing failed %d\n", count_w);
            close(fd1);
            close(fd2);
            exit(EXIT_FAILURE);
        }
    }
    
    if (count_r == -1)
    {
        printf("Problem in reading");
        close(fd1);
        close(fd2);
        exit(READ_ERROR);
    }
    close(fd1);
    close(fd2);
    return 0;
}