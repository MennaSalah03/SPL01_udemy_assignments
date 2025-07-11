#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

int assign_detect(char *string);
void handle_reirection(char **args, char **redirecion_array, int redirect_count);
int valid_redirect(char *arg);
extern char **environ;

//nanoshell_main
int main(int argc, char **argv)
{
    int buff_size = 1024; 
    int args_num = 64;
    int max_vars = 3;
    int max_redirects = 3;
    char *buffer =(char *) malloc(buff_size);
    char **args = (char **) malloc(args_num * sizeof(char *));
    char **loc_vars = (char **) malloc(max_vars * sizeof(char *));
    char **redirection_arr = (char **)malloc(max_redirects * sizeof (char *));
    int loc_var_count = 0;
    int var_exists = 0;
    int redirect_count = 0;
    int status = 0; // 0 is success
    int first_redir_idx = -1;

    if (loc_vars == NULL)
    {
        fprintf(stderr, "Memory allocation failed for local variables\n");
        return 1;
    }
    if (args == NULL)
    {
        fprintf(stderr, "Memory allocation failed for arguments\n");
        return 1;
    }
    if (buffer == NULL)
    {
        fprintf(stderr, "Memory allocation failed for buffer\n");
        return 1;
    }

    if (redirection_arr == NULL)
    {
        fprintf(stderr, "Memory allocation failed for redirections\n");
        return 1;
    }
    do {
        printf("micro-mennux > ");
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
            continue; // Skip empty input

        int arg = 0;
        // Parse input into arguments
        char *token = strtok(buffer, " ");
        while (token != NULL)
        {
            if (arg >= args_num - 1)
            {
                args_num *= 2;
                char **new_args = (char **) realloc(args, args_num * sizeof(char *));
                if (new_args == NULL)
                {
                    fprintf(stderr, "Memory allocation failed for buffer\n");
                    break;
                }
                args = new_args;
            }
            char *dollar_sign = strchr(token, '$');
            int found = 0;
            if (dollar_sign != NULL)
            {
                // Handle case where '$' exists in the token
                char *var_name = dollar_sign + 1; // Skip the '$'
                char *equal_sign;
                char *value = NULL;
                int var_len = strlen(var_name);
                if (var_len > 0)
                {
                    // Search local variables
                    for (int loc_var = 0; loc_var < loc_var_count; loc_var++)
                    {
                        equal_sign = strchr(loc_vars[loc_var], '=');
                        if (equal_sign != NULL && strncmp(loc_vars[loc_var], var_name, var_len) == 0)
                        {
                            value = equal_sign + 1;
                            found = 1;
                            break;
                        }
                    }

                    // Search environment variables if not found in local variables
                    if (!found)
                    {
                        value = getenv(var_name);
                        if (value != NULL)
                            found = 1;
                    }

                    // Handle case where variable was found
                    if (found)
                    {
                        size_t prefix_len = dollar_sign - token;
                        size_t arg_len = prefix_len + strlen(value) + 1;
                        args[arg] = (char *) malloc(arg_len * sizeof(char));
                        if (args[arg] == NULL)
                        {
                            perror("Memory allocation failed for arg\n");
                            break;
                        }

                        // Copy the prefix (part before $) and the variable value
                        strncpy(args[arg], token, prefix_len);
                        args[arg][prefix_len] = '\0';  // Null-terminate the prefix
                        strcat(args[arg], value);
                    }
                    else //non-existant variable
                    {
                        
                        args[arg] = (char *) malloc((strlen(token) - var_len) * sizeof(char));
                        if (args[arg] == NULL)
                        {
                            perror("Memory allocation failed for arg\n");
                            break;       
                        }
                        strncpy(args[arg], token, strlen(token) - var_len - 1);
                    }
                }
                else
                {
                    args[arg] = (char *) malloc(strlen(token) * sizeof(char));
                    if (args[arg] == NULL)
                    {
                        perror("Memory allocation failed for arg\n");
                        break;
                    }
                    // Copy the prefix (part before $) and the variable value
                    strcpy(args[arg], token);
                }
            }
            else
            {
                // Case where no '$' in the token
                size_t token_len = strlen(token);
                args[arg] = (char *) malloc((token_len + 1) * sizeof(char));
                if (args[arg] == NULL)
                {
                    perror("Memory allocation failed for arg\n");
                    break;
                }
                strcpy(args[arg], token);
            }
            token = strtok(NULL, " ");
            arg++;
        }

        // checking for redirections and handling them
        redirect_count = 0;
        for (int i = 0; i < arg - 1; i++)
        {
            int redir_op_found = valid_redirect(args[i]);
            if (redir_op_found)
            {
                if (redirect_count > max_redirects)
                {
                    max_redirects *= 2;
                    char **temp_redirect_arr = realloc(redirection_arr, max_redirects * sizeof(char));
                    if (temp_redirect_arr == NULL)
                    {
                        fprintf(stderr, "Memory reallocation failed for redirection array");
                        break;
                    }
                    redirection_arr = temp_redirect_arr;
                }
                redirection_arr[redirect_count] = (char *) malloc(strlen(token) * sizeof(char));
                if (redirection_arr[redirect_count] == NULL)
                {
                    fprintf(stderr, "Memory allocation failed for redirection array");
                    break;
                }
                strcpy(redirection_arr[redirect_count++], token);
            }
        }
        args[arg] = NULL; // Null-terminate args
        if (strncmp(args[0], "echo", 4) == 0)
        {
            if (first_redir_idx >= 0)
            {
                
            }
            for (int arg = 1; args[arg] != NULL; arg++)
            {
                printf("%s", args[arg]);
                if (args[arg + 1] != NULL)
                    putchar(' ');
            }
            putchar('\n');
            status = 0;
            continue;
        }  
        else if (strncmp(args[0], "exit", 4) == 0)
        {
            printf("Good Bye\n");
            break;
            status = 0;
        }
        else if (strncmp(args[0], "cd", 2) == 0)
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
                    break;
                }
                loc_vars = temp;
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
        else
        {
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
                    status = WEXITSTATUS(status);
            }    
        }
    } while (1);

    //free allocated memory for arguments
    for (int j = 0; j < loc_var_count; j++)
        free(loc_vars[j]);
    for (int j = 0; args[j] != NULL ; j++)
        free(args[j]);
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

/**
 * handle_redirection
 * 
 * return
 */
void handle_reirection(char **args, char ** redirection_array, int redirect_count)
{
    char saved_fds[3];
    saved_fds[0] = dup(STDIN_FILENO);   // stdin
    saved_fds[1] = dup(STDOUT_FILENO);  // stdout
    saved_fds[2] = dup(STDERR_FILENO);  // stderr

    for (int op = 0; op < redirect_count; op++)
    {
        if (strcmp(redirection_array[op], "<"))
        {

        }
        else if (strcmp(redirection_array[op], ">"))
        {

        }
        else if (strcmp(redirection_array[op], "2>"))
        {

        }
    
        
    }
}

/**
 * valid redirect: Indicates whether an argument has a redirection
 * 
 * returns: -1 if there's no valid redirection and the index of 
 */
int valid_redirect(char *arg)
{
    int redir_idx = -1;
    for (int i = 0; i < strlen(arg); i++)
    {
        if (strncmp(arg + i, "<", 1) == 0 || strncmp(arg + i, ">", 1) || strncmp(arg + i, "2>", 2))
        {
            redir_idx = i;
            break;
        }
    }
    return redir_idx;
}