#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>


#define MAX_PATH 1024


int main(int argc, char** argv)
{
    char cwd[MAX_PATH];
    if (argc > 2)
    {
        exit(-1);
    }
    if ((getcwd(cwd, MAX_PATH)) == NULL)
    {
        printf("Error");
        exit(-2);
    }
    write(1, cwd, strlen(cwd));
    return 0;
}