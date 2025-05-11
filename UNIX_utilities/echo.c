#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>


int main(int argc, char **argv)
{
    int newline = 1;

    if (strcmp(argv[1], "-n") == 0)
    {
        newline = 0;
        argc--;
        argv++; // increments pointer to next token
    }
    for (int i = 1; i < argc; i++)
    {
        write(1, argv[i], strlen(argv[i]));
        if (i != argc - 1)
        {
            write(1, " ", 1);
        }
    }
    if (newline)
        putchar('\n');
    return 0;
}