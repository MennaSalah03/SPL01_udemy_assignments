#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        perror("Incorrect number of arguments\n");
        exit(-1);
    }
    if (rename(argv[1], argv[2]) < 0)
    {
        perror("Moving file failed\n");
        exit(-2);
    }
    return 0;
}