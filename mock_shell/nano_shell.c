#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int char_detect(char chr, char *string);
extern char **environ;

//picoshell_main
int main(int argc, char **argv)
{
    int buff_size = 1024; 
    int args_num = 64;
    char *buffer =(char *) malloc(buff_size);
    char **args = (char **) malloc(args_num * sizeof(char *));
    char **loc_vars = (char **) malloc(3 * sizeof(char *));
    int i = 0;
    int status = 0; // 0 is success

    if (loc_vars == NULL) {
        fprintf(stderr, "Memory allocation failed for local variables\n");
        return 1;
    }
    if (args == NULL) {
        fprintf(stderr, "Memory allocation failed for arguments\n");
        return 1;
    }

    do {
        printf("nano-mennux > ");
        fflush(stdout);
        if (fgets(buffer, buff_size, stdin) == NULL)
        {
            break; 
        }

        while (strchr(buffer, '\n') == NULL && !feof(stdin))
        {
            buff_size *= 2;
            buffer = (char *) realloc(buffer, buff_size);
            if (!fgets(buffer + strlen(buffer), buff_size - strlen(buffer), stdin))
            {
                break;
            }
        }

        buffer[strlen(buffer) - 1] = 0; // Remove newline
        if (strlen(buffer) == 0)
        {
            continue; // Skip empty input
        }

        // Parse input into arguments
        int i = 0;
        char *token = strtok(buffer, " ");
        while (token != NULL) {
            if (i >= args_num - 1)
            {
                args_num *= 2;
                args = (char **) realloc(args, args_num * sizeof(char *));
            }
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // Null-terminate args

        if (strcmp(args[0], "echo") == 0)
        {
            for (int j = 1; args[j] != NULL; j++)
            {
                printf("%s", args[j]);
                if (args[j + 1] != NULL)
                    putchar(' ');
            }
            printf("\n");
            status = 0;
            continue;
        }
        else if (strcmp(args[0], "exit") == 0)
        {
            printf("Good Bye\n");
            break;
            status = 0;
        }
        else if (strcmp(args[0], "cd") == 0)
        {
            if (args[1] == NULL || strcmp(args[1], "~") == 0)
                chdir(getenv("HOME"));
            else if (strcmp(args[1], "..") == 0)
                chdir("..");
            else if (strcmp(args[0], "export") == 0)
                getenv(args[1]);
            else
            {
                if (chdir(args[1]) != 0)
                {
                    printf("cd: %s: No such file or directory\n", args[1]);
                    status = -2;
                    continue;
                }
            }
            status = 0;
            continue;
        }
        else if (char_detect('=', args[0]) == 0)
        {
            loc_vars[i] = malloc(strlen(args[0]) * sizeof(char));
            if (loc_vars[i] == NULL)
            {
                fprintf(stderr, "Memory allocation failed for the current local variables\n");
                return 1;
            }
            strcpy(loc_vars[i], args[0]);
            status = 0;
            for (int j = 0; j < i; j++)
            {
                printf("%s ", loc_vars[j]);
            }
            continue;

        }
        pid_t pid = fork();

        if (pid < 0)
        {
            // Fork failed
            perror("fork failed\n");
            exit(1);
        }
        else if (pid == 0)
        {
            // Child process
            if (execvp(args[0], args) == -1)
            {
                // execvp failed
                printf("%s: command not found\n", args[0]);
                status = -1;
                break;
            }
        }
        else if (pid > 0)
        {
            // Parent process: wait for child if not a background command
            waitpid(pid, &status, 0);
            if (WIFEXITED(status))
            {
                status = WEXITSTATUS(status);
            }
        }
    } while (1);

    //free allocated memory for arguments
    for (int j = 0; j < i - 1; j++)
    {
        free(loc_vars[j]);
    }
    free(loc_vars);
    free(buffer);
    free(args);

    return status;
}


/**
 * char_detect: checks whether a character exists in a string
 * 
 * chr: the character we are looking for in the string
 * string: the string we are searching in
 * 
 * return: 0 in case of success (character found in string), otherwise it returns -1 
 */
int char_detect(char chr, char *string)
{
    int detect_status = -1;
    for (int i = 0; i < strlen(string); i++)
    {
        if (*(string + i) == chr)
        {
            detect_status = 0;
            break;
        }
    }   
    return detect_status;
}
