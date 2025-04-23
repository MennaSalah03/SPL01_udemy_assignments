#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>


int main(int argc, char **argv)
{
    if (argc == 1)
    {
        exit(-1);
    }
    char nxt_scp = '\0';
    for (int i = 1; i < argc; i++)
    {
        for (int j = 0; j < strlen(argv[i]); j++)
        {
            if(argv[i][j] == '/')
            {
                nxt_scp = argv[i][j + 1];
            }
        }
        write(1, argv[i], strlen(argv[i]));
        if (i != argc - 1)
        {
            write(1, " ", 1);
        }
    }
    return 0;
}