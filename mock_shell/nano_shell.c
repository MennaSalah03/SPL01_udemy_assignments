#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>

int assign_detect(char *string);
extern char **environ;

//nanoshell_main
int main(int argc, char **argv)
{
    int buff_size = 1024; 
    int args_num = 64;
    int max_vars = 3;
    char *buffer =(char *) malloc(buff_size);
    char **args = (char **) malloc(args_num * sizeof(char *));
    char **loc_vars = (char **) malloc(max_vars * sizeof(char *));
    int i = 0;
    int loc_var_count = 0;
    int var_exists = 0;
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
            break; 

        while (strchr(buffer, '\n') == NULL && !feof(stdin))
        {
            buff_size *= 2;
            buffer = (char *) realloc(buffer, buff_size);
            if (!fgets(buffer + strlen(buffer), buff_size - strlen(buffer), stdin))
                break;
        }

        buffer[strlen(buffer) - 1] = 0; // Remove newline
        if (strlen(buffer) == 0)
        {
            continue; // Skip empty input
        }

        // Parse input into arguments
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
            for (int arg = 1; args[arg] != NULL; arg++)
            {
                if (*(args[arg]) == '$')
                {
                    var_exists = 0;
                    //local variables search
                    for (int loc_var = 0; loc_var < loc_var_count; loc_var++)
                    {
                        if (strcmp(loc_vars[loc_var], (args[1] + 1)) == 0)
                        {
                            puts("here");
                            printf("%s\n", loc_vars[loc_var]);
                            var_exists = 1;
                            break;
                        }
                    }

                    // enviroment variables search
                    for (int env_var = 0; environ[env_var] != NULL; env_var++)
                    {
                        if (strcmp(environ[env_var], (args[1] + 1)) == 0)
                        {
                            puts("here");
                            printf("%s\n", environ[env_var]);
                            var_exists = 1;
                        }
                    }
                    if (var_exists == 0)
                        putchar('\n');
                }
                printf("%s", args[arg]);
                if (args[arg + 1] != NULL)
                    putchar(' ');
            }
            putchar('\n');
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
        else if (assign_detect(args[0]) == 0)
        {
            if (loc_var_count >= max_vars)
            {
                max_vars *= 2;
                char **temp = (char **) realloc(loc_vars, max_vars * sizeof(char *)); 
                loc_vars = temp;
                if (loc_vars == NULL)
                {
                    fprintf(stderr, "Memory allocation failed for the local variables array\n");
                    return 1;
                }
            }
            loc_vars[loc_var_count] = (char *) malloc(strlen(args[0]) * sizeof(char));
            if (loc_vars[loc_var_count] == NULL)
            {
                fprintf(stderr, "Memory allocation failed for the current local variables\n");
                return 1;
            }
            strcpy(loc_vars[loc_var_count], args[0]);
           
            loc_var_count++;
            status = 0;
            continue;
        }
        else if ((strcmp(args[0], "export") == 0) && args[1])
        {
            if (putenv(strdup(args[1])) != 0)
            {
                perror("Enviroment variable not added\n");
            }
            status = 0;
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
 * assign_detect: checks whether a character exists in a string
 * 
 * chr: the character we are looking for in the string
 * string: the string we are searching in
 * 
 * return: 0 in case of success (character found in string), otherwise it returns -1 
 */
int assign_detect(char *string)
{
    int str_len = strlen(string);
    int detect_status = -1;
    for (int i = 0; i < str_len; i++)
    {
        if (*(string + i) == '=')
        {
            if (*(string + i + 1) == '\0')
                break;
            detect_status = 0;
            break;
        }
    }   
    return detect_status;
}
