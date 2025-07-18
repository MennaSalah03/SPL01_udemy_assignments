#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <errno.h>


int assign_detect(char *string);
int handle_redirection(char **args, int arg_count);
int redirection_op_idx(char *arg);
extern char **environ;

//microshell_main
int main(int argc, char **argv)
{
    int buff_size = 1024;
    int args_num = 64;
    int max_vars = 3;
    char *buffer =(char *) malloc(buff_size);
    char **args = (char **) malloc(args_num * sizeof(char *));
    char **loc_vars = (char **) malloc(max_vars * sizeof(char *));
    int loc_var_count = 0;
    int status = 0; // 0 is success

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

    do
    {
        printf("micro-mennux: ");
        fflush(stdout);
        fflush(stderr);
        // Read all available input at once
        if (fgets(buffer, buff_size, stdin) == NULL)
            break;
            
        while (strchr(buffer, '\n') == NULL && !feof(stdin))
        {
            buff_size *= 2;
            buffer = (char *) realloc(buffer, buff_size);
            if (!fgets(buffer + strlen(buffer), buff_size - strlen(buffer), stdin))
                break;
        }

        // Now split by newlines and process each command
        char *line = strtok(buffer, "\n");
        while (line != NULL)
        {
            if (strlen(line) == 0)
            {
                line = strtok(NULL, "\n");
                continue; // Skip empty lines
            }
           
            // Process this individual command (your existing parsing code goes here)
            // You'll need to wrap your existing parsing and execution code in a function
            // or duplicate it here for each 'line' 
            
            line = strtok(NULL, "\n");
        }
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
            // make temporary copy of token. This helps us allocate memory at the end of the loop
            char *tmp_arg; 
            char *dollar_sign = strchr(token, '$');
            int token_redir_idx = redirection_op_idx(token);
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
                        tmp_arg = (char *) malloc(arg_len * sizeof(char));
                        if (tmp_arg == NULL)
                        {
                            perror("Memory allocation failed for arg\n");
                            break;
                        }

                        // Copy the prefix (part before $) and the variable value
                        strncpy(tmp_arg, token, prefix_len);
                        tmp_arg[prefix_len] = '\0';  // Null-terminate the prefix
                        strcat(tmp_arg, value);
                    }
                    else //non-existant variable
                    {
                        
                        tmp_arg = (char *) malloc((strlen(token) - var_len) * sizeof(char));
                        if (tmp_arg == NULL)
                        {
                            perror("Memory allocation failed for arg\n");
                            break;       
                        }
                        strncpy(tmp_arg, token, strlen(token) - var_len - 1);
                        tmp_arg[strlen(token) - var_len - 1] = '\0';
                    }
                }
                else
                {
                    tmp_arg = (char *) malloc((strlen(token) + 1) * sizeof(char));
                    if (tmp_arg == NULL)
                    {
                        perror("Memory allocation failed for arg\n");
                        break;
                    }
                    // Copy the prefix (part before $) and the variable value
                    strcpy(tmp_arg, token);
                }
            }
            else
            {
                // Case where no '$' in the token
                tmp_arg = (char *) malloc((strlen(token) + 1) * sizeof(char));
                if (tmp_arg == NULL)
                {
                    perror("Memory allocation failed for arg\n");
                    break;
                }
                strcpy(tmp_arg, token);
            }

            if (token_redir_idx >= 0)
            {

                int redir_parts = 1;
                int one_or_2_chars = 1;

                if (token_redir_idx > 0) // there's prefix
                    redir_parts++;

                if (token_redir_idx < (int) strlen(tmp_arg) && tmp_arg[token_redir_idx] == '2')
                     one_or_2_chars = 2;

                if (token_redir_idx + one_or_2_chars < (int) strlen(tmp_arg)) //there's a suffix
                    redir_parts++;
    
                if (arg + redir_parts >= args_num - 1)
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
                if (token_redir_idx > 0)
                {
                    args[arg] = (char *) malloc((token_redir_idx + 1) * sizeof(char));
                    if (args[arg] == NULL)
                    {
                        fprintf(stderr, "Memory allocation failed for this arg\n");
                        break;
                    }
                    strncpy(args[arg], tmp_arg, token_redir_idx);
                    args[arg][token_redir_idx] = '\0';
                    arg++;
                }
                args[arg] = (char *) malloc((one_or_2_chars + 1) * sizeof(char));
                if (args[arg] == NULL)
                {
                    fprintf(stderr, "Memory allocation failed for buffer\n");
                    break;
                }
                strncpy(args[arg], tmp_arg + token_redir_idx, one_or_2_chars);
                args[arg][one_or_2_chars] = '\0';
                
                if (token_redir_idx + one_or_2_chars < (int) strlen(tmp_arg))
                {                   
                    arg++;
                    int suff_len = strlen(tmp_arg) - token_redir_idx - one_or_2_chars;
                    args[arg] = (char *) malloc((suff_len + 1) * sizeof(char));
                    if (args[arg] == NULL)
                    {
                        fprintf(stderr, "Memory allocation failed for buffer\n");
                        break;
                    }
                    strncpy(args[arg], tmp_arg + token_redir_idx + one_or_2_chars, suff_len);
                    args[arg][suff_len] = '\0';
                }
            }
            else
            {
                // No redirection found, handle normally
                args[arg] = (char *) malloc((strlen(tmp_arg) + 1) * sizeof(char));
                if (args[arg] == NULL)
                {
                    fprintf(stderr, "Memory allocation failed for arg\n");
                    break;
                }
                strcpy(args[arg], tmp_arg);
            }
            
            token = strtok(NULL, " ");
            arg++;
            free(tmp_arg);
        }

        args[arg] = NULL; // Null-terminate args
        
        // Check if there are any redirection operators in the command
        int has_redirection = 0;
        for (int i = 0; i < arg; i++)
        {
            if (args[i] && (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], "2>") == 0))
            {
                has_redirection = 1;
                break;
            }
        }
        
        if (strcmp(args[0], "echo") == 0)
        {
            pid_t pid = fork();
            
            if (pid < 0)
            {
                perror("fork failed\n");
                exit(1);
            }
            else if (pid == 0)
            {
                // Child process - handle redirection if present
                if (has_redirection)
                {
                    if (handle_redirection(args, arg) != 0)
                    {
                        exit(1);
                    }
                }

                // Print echo output
                for (int i = 1; args[i] != NULL; i++)
                {
                    // Skip redirection operators and their targets
                    if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0 || strcmp(args[i], "2>") == 0)
                    {
                        i++; // Skip the filename too
                        continue;
                    }
                    printf("%s", args[i]);
                    if (args[i + 1] != NULL && strcmp(args[i + 1], "<") != 0 && 
                        strcmp(args[i + 1], ">") != 0 && strcmp(args[i + 1], "2>") != 0)
                        putchar(' ');
                }
                putchar('\n');
                exit(0);
            }
            else if (pid > 0)
            {
                waitpid(pid, &status, 0);
                if (WIFEXITED(status))
                    status = WEXITSTATUS(status);
            }
        }  
        else if (strcmp(args[0], "exit") == 0)
        {
            printf("Good Bye\n");
            status = 0;
            break;
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
        }
        else if (assign_detect(args[0]) == 0)
        {
            if (loc_var_count >= max_vars)
            {
                max_vars *= 2;
                char **temp = (char **) realloc(loc_vars, max_vars * sizeof(char *)); 
                if (temp == NULL)
                {
                    fprintf(stderr, "Memory allocation failed for the local variables array\n");
                    break;
                }
                loc_vars = temp;
            }
            loc_vars[loc_var_count] = (char *) malloc((strlen(args[0]) + 1) * sizeof(char));
            if (loc_vars[loc_var_count] == NULL)
            {
                fprintf(stderr, "Memory allocation failed for the current local variables\n");
                return 1;
            }
            strcpy(loc_vars[loc_var_count], args[0]);
            
            loc_var_count++;
            status = 0;
        }
        else if ((strcmp(args[0], "export") == 0) && args[1])
        {
            if (putenv(strdup(args[1])) != 0)
            {
                perror("Environment variable not added\n");
            }
            status = 0;
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
                // Child process - handle redirection if present
                if (has_redirection)
                {
                    if (handle_redirection(args, arg) != 0)
                    {
                        exit(1);
                    }
                }
                
                char **exec_args = (char **) malloc((arg + 1) * sizeof(char *));
                if (exec_args == NULL)
                {
                    perror("allocation failed\n");
                    exit(-1);
                }
                int exec_arg_count = 0;
                
                for (int i = 0; i < arg; i++)
                {
                    if (args[i] && strcmp(args[i], "<") != 0 && strcmp(args[i], ">") != 0 && strcmp(args[i], "2>") != 0)
                    {
                        // Check if this is a filename following a redirection operator
                        int is_redirect_target = 0;
                        if (i > 0 && args[i - 1] && 
                            (strcmp(args[i - 1], "<") == 0 || strcmp(args[i - 1], ">") == 0 || strcmp(args[i - 1], "2>") == 0))
                        {
                            is_redirect_target = 1;
                        }
                        
                        if (!is_redirect_target)
                        {
                            exec_args[exec_arg_count] = args[i];
                            exec_arg_count++;
                        }
                    }
                }
                exec_args[exec_arg_count] = NULL;
                
                if (execvp(exec_args[0], exec_args) == -1)
                {
                    // execvp failed
                    printf("%s: command not found\n", exec_args[0]);
                    free(exec_args);
                    exit(1);
                }
                free(exec_args);
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
    for (int j = 0; args[j] != NULL; j++)
        free(args[j]);
    free(loc_vars);
    free(buffer);
    free(args);

    return status;
}


/**
 * assign_detect: checks whether a character exists in a string
 * 
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
 * handle_redirection: handles the redirection as specified in the command
 * 
 * args: argument array
 * arg_count: number of arguments
 * 
 * return the status of the operation 0 for success, -ve number for failure
 */
int handle_redirection(char **args, int arg_count)
{
    int i = 0;
    
    while (i < arg_count && args[i] != NULL)
    {
        if (strcmp(args[i], "<") == 0)
        {
            // Input redirection
            if (i + 1 < arg_count && args[i + 1] != NULL)
            {
                int fd = open(args[i + 1], O_RDONLY);
                if (fd == -1)
                {
                    if (errno == EACCES)
                    {
                        fprintf(stderr, "%s: Permission denied\n", args[i + 1]);
                        return -1;
                    }
                    else if (errno == ENOENT)
                    {
                        fprintf(stderr, "cannot access %s: No such file or directory\n", args[i + 1]);
                        return -1;
                    }
                    else
                    {
                        fprintf(stderr, "Error opening input file: %s\n", strerror(errno));
                        return -1;
                    }
                }
                if (dup2(fd, STDIN_FILENO) == -1)
                {
                    perror("Error redirecting stdin");
                    return -1;
                }
                close(fd);
                i += 2; // Skip the redirection operator and filename
            }
            else
            {
                fprintf(stderr, "Syntax error: missing filename after '<'\n");
                return -1;
            }
        }
        else if (strcmp(args[i], ">") == 0)
        {
            // Output redirection
            if (i + 1 < arg_count && args[i + 1] != NULL)
            {
                int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1)
                {
                    if (errno == EACCES)
                    {
                        fprintf(stderr, "%s: Permission denied\n", args[i + 1]);
                        return -1;
                    }
                    else if (errno == ENOENT)
                    {
                        fprintf(stderr, "cannot access %s: No such file or directory\n", args[i + 1]);
                        return -1;
                    }
                    else
                    {
                        fprintf(stderr, "Error opening output file: %s\n", strerror(errno));
                        return -1;
                    }
                }
                if (dup2(fd, STDOUT_FILENO) == -1)
                {
                    perror("Error redirecting stdout");
                    return -2;
                }
                close(fd);
                i += 2; // Skip the redirection operator and filename
            }
            else
            {
                fprintf(stderr, "Syntax error: missing filename after '>'\n");
                return -2;
            }
        }
        else if (strcmp(args[i], "2>") == 0)
        {
            // Error redirection
            if (i + 1 < arg_count && args[i + 1] != NULL)
            {
                int fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd == -1)
                {
                    if (errno == EACCES)
                    {
                        fprintf(stderr, "%s: Permission denied\n", args[i + 1]);
                        return -1;
                    }
                    else if (errno == ENOENT)
                    {
                        fprintf(stderr, "cannot access %s: No such file or directory\n", args[i + 1]);
                        return -1;
                    }
                    else
                    {
                        fprintf(stderr, "Error error output file: %s\n", strerror(errno));
                        return -1;
                    }
                }
                if (dup2(fd, STDERR_FILENO) == -1)
                {
                    perror("Error redirecting stderr");
                    return -3;
                }
                close(fd);
                i += 2; // Skip the redirection operator and filename
            }
            else
            {
                fprintf(stderr, "Syntax error: missing filename after '2>'\n");
                return -3;
            }
        }
        else
        {
            // Not a redirection operator, move to next argument
            i++;
        }
    }
    return 0;
}

/**
 * redirection_op_idx: Indicates whether an argument has a redirection
 * 
 * returns: -1 if there's no valid redirection and the index of the redirection operator if found
 */
int redirection_op_idx(char *arg)
{
    int redir_idx = -1;
    for (int i = 0; i < (int) strlen(arg); i++)
    {
        if (strncmp(arg + i, "2>", 2) == 0 || strncmp(arg + i, "<", 1) == 0 || strncmp(arg + i, ">", 1) == 0)
        {
            redir_idx = i;
            break;
        }
    }
    return redir_idx;
}